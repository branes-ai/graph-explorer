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

#include <geogram/mesh/mesh_repair.h>

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
    sw::dfa::DomainFlowGraph* dfg = m_context.graph_window->get_domain_flow_graph();
    if (test_any_rhs_bits_set(message.update_flags, Message_flag_bit::c_flag_bit_selection)) {
        bool new_nodes = false;
        for (const auto& item : message.no_longer_selected) {
            std::shared_ptr<Graph_node> graph_ui_node = std::dynamic_pointer_cast<Graph_node>(item);
            if (!graph_ui_node) {
                continue;
            }
            std::shared_ptr<erhe::scene::Node> scene_graph_node = graph_ui_node->get_convex_hull_visualization();
            if (scene_graph_node) {
                scene_graph_node->recursive_remove();
                graph_ui_node->set_convex_hull_visualization({});
            }
        }
        for (const auto& item : message.newly_selected) {
            std::shared_ptr<Graph_node> graph_ui_node = std::dynamic_pointer_cast<Graph_node>(item);
            if (!graph_ui_node) {
                continue;
            }
            std::size_t node_id = graph_ui_node->get_payload();
            const sw::dfa::DomainFlowNode& node = dfg->graph.node(node_id);
            std::shared_ptr<erhe::scene::Node> scene_graph_node = add_node_convex_hull(node);
            if (scene_graph_node) {
                graph_ui_node->set_convex_hull_visualization(scene_graph_node);
                new_nodes = true;
            }
        }
        if (new_nodes && m_last_scene_bbox.is_valid()) {
            std::shared_ptr<Scene_root> scene_root = m_context.scene_builder->get_scene_root();
            m_context.fly_camera_tool->frame(m_last_scene_bbox, Frame_mode::look_at_with_standard_y_up);
            // TODO Test Frame_mode::keep_orientation and/or Frame_mode::choose_direction_based_on_bbox
        }
    }
}

void Node_convex_hull_visualization::update_bounding_box()
{
    m_last_scene_bbox = erhe::math::Bounding_box{};
    m_root->for_each_child_const<erhe::scene::Node>(
        [this](const erhe::scene::Node& node) -> bool
        {
            std::shared_ptr<erhe::scene::Mesh> mesh = get_mesh(&node);
            if (!mesh) {
                return true;
            }
            const std::vector<erhe::primitive::Primitive>& primitives = mesh->get_primitives();
            for (const erhe::primitive::Primitive& primitive : primitives) {
                if (!primitive.render_shape) {
                    continue;
                }
                const erhe::primitive::Buffer_mesh& buffer_mesh = primitive.render_shape->get_renderable_mesh();
                const erhe::math::Bounding_box bbox_in_world = buffer_mesh.bounding_box.transformed_by(node.world_from_node());
                m_last_scene_bbox.include(bbox_in_world);
            }
            return true;
        }
    );
}

auto Node_convex_hull_visualization::add_node_convex_hull(const sw::dfa::DomainFlowNode& node) -> std::shared_ptr<erhe::scene::Node>
{
    std::shared_ptr<erhe::scene::Node> scene_graph_node{};
    std::shared_ptr<Scene_root> scene_root = m_context.scene_builder->get_scene_root();
    std::lock_guard<ERHE_PROFILE_LOCKABLE_BASE(std::mutex)> scene_lock{scene_root->item_host_mutex};

    if (!m_root) {
        const glm::vec3 root_pos{0.0, 1.0f, 0.0f};
        m_root = std::make_shared<erhe::scene::Node>("root");
        m_root->enable_flag_bits(erhe::Item_flags::content | erhe::Item_flags::visible | erhe::Item_flags::show_in_ui);
        m_root->set_world_from_node(erhe::math::create_translation<float>(root_pos));
        m_root->set_parent(scene_root->get_scene().get_root_node());
    }

    update_bounding_box();

    // Create material
    if (!m_material) {
        auto& material_library = scene_root->content_library()->materials;
        m_material = material_library->make<erhe::primitive::Material>(
            "mat_convex_hull", glm::vec3{1.0, 1.0f, 1.0f}, glm::vec2{0.3f, 0.4f}, 0.0f
        );
    }

    // Build convex hull mesh
    std::shared_ptr<erhe::geometry::Geometry> geometry = std::make_shared<erhe::geometry::Geometry>("geometry_convex_hull");
    GEO::Mesh& geo_mesh = geometry->get_mesh();

#if 0 // New API - faces are not all consistently oriented (either all in clockwise or counter-clockwise order)
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
    geo_mesh.vertices.set_double_precision();
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
    geo_mesh.vertices.set_single_precision();
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
    GEO::mesh_reorient(geo_mesh);

    const uint64_t geometry_process_flags =
        erhe::geometry::Geometry::process_flag_connect |
        erhe::geometry::Geometry::process_flag_build_edges |
        erhe::geometry::Geometry::process_flag_compute_facet_centroids |
        erhe::geometry::Geometry::process_flag_compute_smooth_vertex_normals |
        erhe::geometry::Geometry::process_flag_generate_facet_texture_coordinates;
    m_geometry->process(geometry_process_flags);
#else // Old API - this path gets faces correctly oriented but the new API that has explicit faces will be nicer
    // Extract points from node
    std::vector<glm::vec3> convex_hull_points;
    const sw::dfa::PointSet points = node.convexHullPointSet();
    if (points.pointSet.size() < 4) {
        log_graph->info("Not enough points for convex hull for {}:", node.getName());
        return {};
    }

    log_graph->info("Convex hull input points for {}:", node.getName());
        for (const sw::dfa::Point<int>& p : points.pointSet) {
        log_graph->info("  {}, {}, {}", p.coords[0], p.coords[1], p.coords[2]);
        convex_hull_points.emplace_back(p.coords[0], p.coords[1], p.coords[2]);
    }
    erhe::geometry::shapes::make_convex_hull(geo_mesh, convex_hull_points);
    const uint64_t geometry_process_flags =
        erhe::geometry::Geometry::process_flag_connect |
        erhe::geometry::Geometry::process_flag_build_edges |
        erhe::geometry::Geometry::process_flag_compute_facet_centroids |
        erhe::geometry::Geometry::process_flag_compute_smooth_vertex_normals |
        erhe::geometry::Geometry::process_flag_generate_facet_texture_coordinates |
        erhe::geometry::Geometry::process_flag_merge_coplanar_neighbors;
    geometry->process(geometry_process_flags);
    log_graph->info("Convex hull output points for {}:", node.getName());
    erhe::math::Bounding_box aabb{};
    for (GEO::index_t vertex : geo_mesh.vertices) {
        const GEO::vec3f p = get_pointf(geo_mesh.vertices, vertex);
        log_graph->info("  {}, {}, {}", p.x, p.y, p.z);
        aabb.include(glm::vec3{p.x, p.y, p.z});
    }
#endif

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
    erhe::primitive::Primitive primitive{geometry, m_material, build_info, erhe::primitive::Normal_style::polygon_normals};

    using namespace erhe;
    const uint64_t node_flags = Item_flags::visible | Item_flags::content | Item_flags::show_in_ui;
    const uint64_t mesh_flags = Item_flags::visible | Item_flags::content | Item_flags::opaque | Item_flags::id | Item_flags::show_in_ui;

    float x = m_last_scene_bbox.is_valid()
        ? m_last_scene_bbox.max.x + m_gap + aabb.center().x
        : -aabb.center().x;

    ERHE_VERIFY(primitive.render_shape->make_raytrace(geo_mesh));
    scene_graph_node = std::make_shared<erhe::scene::Node>("node_convex_hull");
    auto scene_mesh = std::make_shared<erhe::scene::Mesh>("", primitive);
    scene_mesh->layer_id = scene_root->layers().content()->id;
    scene_mesh->enable_flag_bits(mesh_flags);
    scene_graph_node->attach              (scene_mesh);
    scene_graph_node->set_parent          (m_root);
    scene_graph_node->set_parent_from_node(erhe::math::create_translation<float>(x, 0.0f, 0.0f));
    scene_graph_node->enable_flag_bits    (node_flags);

    update_bounding_box();
    return scene_graph_node;
}

void Node_convex_hull_visualization::imgui()
{
}

} // namespace explorer
