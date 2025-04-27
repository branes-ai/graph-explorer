#include "graph/node_convex_hull_visualization.hpp"
#include "graph/graph_node.hpp"
#include "graph/graph_window.hpp"
#include "explorer_log.hpp"

#include "explorer_context.hpp"
#include "explorer_message_bus.hpp"

#include "renderers/mesh_memory.hpp"

#include "scene/content_library.hpp"
#include "scene/scene_builder.hpp"
#include "scene/scene_root.hpp"

#include "tools/fly_camera_tool.hpp"
#include "tools/selection_tool.hpp"

#include "erhe_bit/bit_helpers.hpp"
#include "erhe_geometry/shapes/convex_hull.hpp"
#include "erhe_imgui/imgui_windows.hpp"
#include "erhe_imgui/imgui_renderer.hpp"
#include "erhe_primitive/primitive.hpp"
#include "erhe_primitive/primitive_builder.hpp"
#include "erhe_primitive/buffer_mesh.hpp"
#include "erhe_profile/profile.hpp"
#include "erhe_scene/node.hpp"
#include "erhe_scene/mesh.hpp"

#include <dfa/dfa.hpp>

namespace explorer {

Node_convex_hull_visualization::Node_convex_hull_visualization(
    erhe::imgui::Imgui_renderer& imgui_renderer,
    erhe::imgui::Imgui_windows&  imgui_windows,
    Explorer_context&            explorer_context,
    Explorer_message_bus&        explorer_message_bus
)
    : Imgui_window{imgui_renderer, imgui_windows, "Node Convex Hull Visualization", "node_convex_hull_visualization"}
    , m_context   {explorer_context}
{
    explorer_message_bus.add_receiver(
        [&](Explorer_message& message) {
            on_message(message);
        }
    );
}

void Node_convex_hull_visualization::on_message(Explorer_message& message)
{
    using namespace erhe::bit;
    if (test_any_rhs_bits_set(message.update_flags, Message_flag_bit::c_flag_bit_selection)) {

        const auto selected_graph_node = m_context.selection->get<erhe::graph::Node>();
        sw::dfa::DomainFlowGraph* dfg = m_context.graph_window->get_domain_flow_graph();

        if (selected_graph_node && (dfg != nullptr)) {
            Graph_node* graph_node = dynamic_cast<Graph_node*>(selected_graph_node.get());
            if (graph_node != nullptr) {
                std::size_t node_id = graph_node->get_payload();
                const sw::dfa::DomainFlowNode& node = dfg->graph.node(node_id);
                reset_scene_for_node_convex_hull(node);
            }
        } else {
            m_visualized_node = nullptr;
            if (m_root) {
                m_root->recursive_remove();
            }
            m_geometry.reset();
        }
    }
}

void Node_convex_hull_visualization::reset_scene_for_node_convex_hull(const sw::dfa::DomainFlowNode& node)
{
    if (m_visualized_node == &node) {
        return;
    }
    m_visualized_node = &node;

    std::shared_ptr<Scene_root> scene_root = m_context.scene_builder->get_scene_root();
    std::lock_guard<ERHE_PROFILE_LOCKABLE_BASE(std::mutex)> scene_lock{scene_root->item_host_mutex};

    if (m_root) {
        m_root->recursive_remove();
    }

    // Create material
    if (!m_material) {
        auto& material_library = scene_root->content_library()->materials;
        m_material = material_library->make<erhe::primitive::Material>(
            "mat_convex_hull", glm::vec3{1.0, 1.0f, 1.0f}, glm::vec2{0.3f, 0.4f}, 0.0f
        );
    }

    // Build convex hull mesh
    m_geometry.reset();
    m_geometry = std::make_shared<erhe::geometry::Geometry>("geometry_convex_hull");
    GEO::Mesh& geo_mesh = m_geometry->get_mesh();

    const sw::dfa::ConvexHull<int>          convex_hull  = node.convexHull();
    const std::vector<sw::dfa::Point<int>>& vertices     = convex_hull.vertices();
    const std::vector<sw::dfa::Face>&       faces        = convex_hull.faces();
    const std::size_t                       vertex_count = vertices.size();
    const std::size_t                       face_count   = faces.size();
    log_graph->info("Convex hull for {}: vertex count = {}, face count = {}", node.getName(), vertex_count, face_count);
    if ((vertex_count < 3) || (face_count < 1)) {
        log_graph->warn("Not enough vertices / faces for node convex hull mesh");
        return;
    }
    erhe::math::Bounding_box aabb{};
    geo_mesh.vertices.create_vertices(static_cast<GEO::index_t>(vertex_count));
    for (std::size_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index) {
        const sw::dfa::Point<int>& p = vertices[vertex_index];
        const int x = p.coords[0];
        const int y = p.coords[1];
        const int z = p.coords[2];
        geo_mesh.vertices.point(static_cast<GEO::index_t>(vertex_index)) = GEO::vec3{static_cast<double>(x), static_cast<double>(y), static_cast<double>(z)};
        log_graph->info("  {}, {}, {}", x, y, z);
        aabb.include(glm::vec3{x, y, z});
    }
    for (std::size_t face_index = 0; face_index < face_count; ++face_index) {
        const sw::dfa::Face& face = faces[face_index];
        const std::size_t corner_count = face.num_vertices();
        std::stringstream ss;
        ss << fmt::format("  Face {} with {} corners:", face_index, corner_count);
        GEO::index_t facet = geo_mesh.facets.create_polygon(static_cast<GEO::index_t>(corner_count));
        const std::vector<size_t>& corner_vertices = face.vertices();
        for (std::size_t local_corner_index = 0; local_corner_index < corner_count; ++local_corner_index) {
            const std::size_t vertex = corner_vertices[local_corner_index];
            ss << fmt::format(" {}", vertex);
            geo_mesh.facets.set_vertex(facet, static_cast<GEO::index_t>(local_corner_index), static_cast<GEO::index_t>(vertex));
        }
        log_graph->info("    {}", ss.str());
    }

    const uint64_t geometry_process_flags =
        erhe::geometry::Geometry::process_flag_connect |
        erhe::geometry::Geometry::process_flag_build_edges |
        erhe::geometry::Geometry::process_flag_compute_facet_centroids |
        erhe::geometry::Geometry::process_flag_compute_smooth_vertex_normals |
        erhe::geometry::Geometry::process_flag_generate_facet_texture_coordinates;
    m_geometry->process(geometry_process_flags);

    // Build buffer mesh
    Mesh_memory& mesh_memory = *m_context.mesh_memory;
    const erhe::primitive::Build_info build_info{
        .primitive_types = {
            .fill_triangles  = true,
            .edge_lines      = true,
            .corner_points   = true,
            .centroid_points = true
        },
        .buffer_info = mesh_memory.buffer_info
    };
    erhe::primitive::Primitive primitive{m_geometry, m_material, build_info, erhe::primitive::Normal_style::polygon_normals};

    ERHE_VERIFY(primitive.render_shape->make_raytrace(geo_mesh));
    const glm::vec3 root_pos{0.0, 1.0f, 0.0f};

    m_root = std::make_shared<erhe::scene::Node>("root");

    m_root->enable_flag_bits(erhe::Item_flags::content | erhe::Item_flags::visible | erhe::Item_flags::show_in_ui);
    m_root->set_world_from_node(erhe::math::create_translation<float>(root_pos));
    {
        auto scene_graph_node = std::make_shared<erhe::scene::Node>("node_convex_hull");
        auto scene_mesh = std::make_shared<erhe::scene::Mesh>("", primitive);
        scene_mesh->layer_id = scene_root->layers().content()->id;
        scene_mesh->enable_flag_bits(erhe::Item_flags::content | erhe::Item_flags::shadow_cast | erhe::Item_flags::opaque);
        scene_graph_node->attach              (scene_mesh);
        scene_graph_node->set_parent          (m_root);
        scene_graph_node->set_parent_from_node(erhe::math::create_translation<float>(0.0f, 0.0f, 0.0f));
        scene_graph_node->enable_flag_bits    (erhe::Item_flags::content | erhe::Item_flags::visible | erhe::Item_flags::show_in_ui);
    }
    m_root->set_parent(scene_root->get_scene().get_root_node());

    m_context.fly_camera_tool->frame(aabb, Frame_mode::look_at_with_standard_y_up);

    // TODO Test
    //m_context.fly_camera_tool->frame(aabb, Frame_mode::keep_orientation);
    //m_context.fly_camera_tool->frame(aabb, Frame_mode::choose_direction_based_on_bbox);
}

void Node_convex_hull_visualization::imgui()
{
}

} // namespace explorer
