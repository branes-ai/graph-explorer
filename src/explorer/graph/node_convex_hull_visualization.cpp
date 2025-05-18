#include "graph/node_convex_hull_visualization.hpp"
#include "graph/graph_node.hpp"
#include "graph/graph_window.hpp"
#include "explorer_log.hpp"

#include "explorer_context.hpp"
#include "explorer_message_bus.hpp"
#include "explorer_rendering.hpp"

#include "renderers/mesh_memory.hpp"
#include "renderers/render_context.hpp"

#include "scene/content_library.hpp"
#include "scene/scene_builder.hpp"
#include "scene/scene_root.hpp"

#include "tools/fly_camera_tool.hpp"
#include "tools/selection_tool.hpp"
#include "tools/transform/transform_tool.hpp"

#include "erhe_bit/bit_helpers.hpp"
#include "erhe_geometry/shapes/convex_hull.hpp"
#include "erhe_imgui/imgui_windows.hpp"
#include "erhe_imgui/imgui_renderer.hpp"
#include "erhe_primitive/primitive.hpp"
#include "erhe_primitive/primitive_builder.hpp"
#include "erhe_primitive/buffer_mesh.hpp"
#include "erhe_profile/profile.hpp"
#include "erhe_renderer/line_renderer.hpp"
#include "erhe_renderer/scoped_line_renderer.hpp"
#include "erhe_renderer/text_renderer.hpp"
#include "erhe_scene/node.hpp"
#include "erhe_scene/mesh.hpp"

#include <dfa/dfa.hpp>

#include <geogram/mesh/mesh_repair.h>

namespace explorer {

Node_convex_hull_visualization::Node_convex_hull_visualization(
    Explorer_context&     explorer_context,
    Explorer_message_bus& explorer_message_bus,
    Explorer_rendering&   explorer_rendering
)
    : m_context{explorer_context}
{
    explorer_rendering.add(this);

    explorer_message_bus.add_receiver(
        [&](Explorer_message& message) {
            on_message(message);
        }
    );
}

void Node_convex_hull_visualization::recreate_visualization_scene_graph()
{
    sw::dfa::DomainFlowGraph* dfg = m_context.graph_window->get_domain_flow_graph();
    if (dfg == nullptr) {
        return;
    }

    std::vector<Graph_node*> ui_nodes;
    Graph& ui_graph = m_context.graph_window->get_ui_graph();
    const std::vector<erhe::graph::Node*>& nodes = ui_graph.get_nodes();
    for (erhe::graph::Node* node : nodes) {
        Graph_node* graph_ui_node = dynamic_cast<Graph_node*>(node);
        if (graph_ui_node == nullptr) {
            continue;
        }
        ui_nodes.push_back(graph_ui_node);
    }

    std::sort(
        ui_nodes.begin(),
        ui_nodes.end(),
        [dfg](const Graph_node* lhs, const Graph_node* rhs)
        {
            const std::size_t lhs_node_id = lhs->get_payload();
            const std::size_t rhs_node_id = rhs->get_payload();
            const sw::dfa::DomainFlowNode& lhs_node = dfg->graph.node(lhs_node_id);
            const sw::dfa::DomainFlowNode& rhs_node = dfg->graph.node(rhs_node_id);
            return lhs_node.getDepth() < rhs_node.getDepth();
        }
    );

    m_last_scene_bbox = {};

    if (!m_root) {
        std::shared_ptr<Scene_root> scene_root = m_context.scene_builder->get_scene_root();
        const glm::vec3 root_pos{0.0, 1.0f, 0.0f};
        m_root = std::make_shared<erhe::scene::Node>("root");
        m_root->enable_flag_bits(erhe::Item_flags::content | erhe::Item_flags::visible | erhe::Item_flags::show_in_ui);
        m_root->set_world_from_node(erhe::math::create_translation<float>(root_pos));
        m_root->set_parent(scene_root->get_scene().get_root_node());
    } else {
        m_root->remove_all_children_recursively();
    }
    for (Graph_node* ui_node : ui_nodes) {
        const std::size_t node_id = ui_node->get_payload();
        const sw::dfa::DomainFlowNode& node = dfg->graph.node(node_id);
        glm::vec3 index_space_offset{0.0f, 0.0f, 0.0f};
        std::shared_ptr<erhe::scene::Node> scene_graph_node = add_node_convex_hull(node, index_space_offset);
        if (scene_graph_node) {
            ui_node->set_convex_hull_visualization(scene_graph_node, index_space_offset);
        }
    }

    update_bounding_box();
    const glm::vec3 root_pos{0.0f, 0.5f * m_last_scene_bbox.diagonal().y, 0.0f};
    m_root->set_world_from_node(erhe::math::create_translation<float>(root_pos));

    if (m_last_scene_bbox.is_valid()) {
        std::shared_ptr<Scene_root> scene_root = m_context.scene_builder->get_scene_root();
        m_context.fly_camera_tool->frame(m_last_scene_bbox, Frame_mode::look_at_with_standard_y_up);
        // TODO Test Frame_mode::keep_orientation and/or Frame_mode::choose_direction_based_on_bbox
    }
}

void Node_convex_hull_visualization::on_message(Explorer_message& message)
{
    using namespace erhe::bit;
    if (test_any_rhs_bits_set(message.update_flags, Message_flag_bit::c_flag_bit_graph_loaded)) {
        recreate_visualization_scene_graph();
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

auto Node_convex_hull_visualization::add_node_convex_hull(
    const sw::dfa::DomainFlowNode& node,
    glm::vec3&                     index_space_offset
) -> std::shared_ptr<erhe::scene::Node>
{
    std::shared_ptr<Scene_root> scene_root = m_context.scene_builder->get_scene_root();
    std::lock_guard<ERHE_PROFILE_LOCKABLE_BASE(std::mutex)> scene_lock{scene_root->item_host_mutex};

    update_bounding_box();

    // Create material
    if (!m_material) {
        auto& material_library = scene_root->content_library()->materials;
        m_material = material_library->make<erhe::primitive::Material>(
            "mat_convex_hull", glm::vec3{1.0, 1.0f, 1.0f}, glm::vec2{0.3f, 0.4f}, 0.0f
        );
        m_material->opacity = 0.25f;
    }

    // Build convex hull mesh
    std::shared_ptr<erhe::geometry::Geometry> geometry = std::make_shared<erhe::geometry::Geometry>("geometry_convex_hull");

    GEO::Mesh& geo_mesh = geometry->get_mesh();

    const sw::dfa::ConvexHull<int>          convex_hull  = node.getConvexHull();
    const std::vector<sw::dfa::Point<int>>& vertices     = convex_hull.vertices();
    const std::vector<sw::dfa::Face>&       faces        = convex_hull.faces();
    const std::size_t                       vertex_count = vertices.size();
    const std::size_t                       face_count   = faces.size();
    if ((vertex_count < 3) || (face_count < 1)) {
        log_graph->warn("Not enough vertices / faces for node convex hull mesh");
        index_space_offset = glm::vec3{0.0f, 0.0f, 0.0f};
        return {};
    }
    geo_mesh.vertices.set_double_precision();
    geo_mesh.vertices.create_vertices(static_cast<GEO::index_t>(vertex_count));

    // First pass to comput aabb
    erhe::math::Bounding_box input_aabb{};
    for (std::size_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index) {
        const sw::dfa::Point<int>& p = vertices[vertex_index];
        const int x = (p.dimension() >= 1) ? p.coords[0] : 0;
        const int y = (p.dimension() >= 2) ? p.coords[1] : 0;
        const int z = (p.dimension() >= 3) ? p.coords[2] : 0;
        input_aabb.include(glm::vec3{x, y, z});
    }

    // Second pass - translate vertices so that (0, 0, 0) is center
    erhe::math::Bounding_box aabb{};
    index_space_offset = - input_aabb.center();
    for (std::size_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index) {
        const sw::dfa::Point<int>& p = vertices[vertex_index];
        const double x = static_cast<double>((p.dimension() >= 1) ? p.coords[0] : 0) + index_space_offset.x;
        const double y = static_cast<double>((p.dimension() >= 2) ? p.coords[1] : 0) + index_space_offset.y;
        const double z = static_cast<double>((p.dimension() >= 3) ? p.coords[2] : 0) + index_space_offset.z;
        geo_mesh.vertices.point(static_cast<GEO::index_t>(vertex_index)) = GEO::vec3{static_cast<double>(x), static_cast<double>(y), static_cast<double>(z)};
        aabb.include(glm::vec3{x, y, z});
    }
    geo_mesh.vertices.set_single_precision();
    for (std::size_t face_index = 0; face_index < face_count; ++face_index) {
        const sw::dfa::Face& face = faces[face_index];
        const std::size_t corner_count = face.num_vertices();
        GEO::index_t facet = geo_mesh.facets.create_polygon(static_cast<GEO::index_t>(corner_count));
        const std::vector<size_t>& corner_vertices = face.vertices();
        for (std::size_t local_corner_index = 0; local_corner_index < corner_count; ++local_corner_index) {
            const std::size_t vertex = corner_vertices[local_corner_index];
            geo_mesh.facets.set_vertex(facet, static_cast<GEO::index_t>(local_corner_index), static_cast<GEO::index_t>(vertex));
        }
    }

    const uint64_t geometry_process_flags =
        erhe::geometry::Geometry::process_flag_connect |
        erhe::geometry::Geometry::process_flag_build_edges |
        erhe::geometry::Geometry::process_flag_compute_facet_centroids |
        erhe::geometry::Geometry::process_flag_compute_smooth_vertex_normals |
        erhe::geometry::Geometry::process_flag_generate_facet_texture_coordinates;

    for (GEO::index_t facet : geo_mesh.facets) {
        const GEO::vec3f facet_normal = GEO::normalize(mesh_facet_normalf(geo_mesh, facet));
        const GEO::vec3f centroid = GEO::normalize(mesh_facet_centerf(geo_mesh, facet));
        const float dot_product = GEO::dot(facet_normal, centroid);
        if (dot_product < 0.0f) {
            geo_mesh.facets.flip(facet);
        }
    }
    GEO::mesh_reorient(geo_mesh, nullptr);
    geometry->process(geometry_process_flags);

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

    const glm::vec3 half_size = 0.5f * aabb.diagonal();
    float x = m_last_scene_bbox.is_valid()
        ? m_last_scene_bbox.max.x + m_gap + half_size.x
        : -aabb.center().x;

    ERHE_VERIFY(primitive.render_shape->make_raytrace(geo_mesh));
    std::shared_ptr<erhe::scene::Node> scene_graph_node = std::make_shared<erhe::scene::Node>("node_convex_hull");
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

auto Node_convex_hull_visualization::get_material() -> erhe::primitive::Material*
{
    return m_material.get();
}

void Node_convex_hull_visualization::render(const Render_context& context)
{
    ERHE_PROFILE_FUNCTION();

    if (context.viewport_scene_view == nullptr) {
        return;
    }

    if (m_context.transform_tool->is_transform_tool_active()) {
        return;
    }

    std::shared_ptr<erhe::scene::Camera> selected_camera;
    const auto& selection = m_context.selection->get_selection();
    for (const auto& item : selection) {
        if (erhe::scene::is_camera(item)) {
            selected_camera = std::dynamic_pointer_cast<erhe::scene::Camera>(item);
        }
    }

    const auto scene_root = context.scene_view.get_scene_root();
    if (!scene_root) {
        return;
    }

    erhe::renderer::Scoped_line_renderer line_renderer = context.get_line_renderer(2, true, true);

    //for (const auto& node : scene_root->get_hosted_scene()->get_flat_nodes()) {
    //    if (node && should_visualize(m_node_axis_visualization, node)) {
    const glm::vec4 red   {1.0f, 0.0f, 0.0f, 1.0f};
    const glm::vec4 green {0.0f, 1.0f, 0.0f, 1.0f};
    const glm::vec4 blue  {0.0f, 0.0f, 1.0f, 1.0f};
    const glm::vec3 O     { 0.0f };
    const glm::vec3 axis_x{1000.0f, 0.0f, 0.0f};
    const glm::vec3 axis_y{0.0f, 1000.0f, 0.0f};
    const glm::vec3 axis_z{0.0f, 0.0f, 1000.0f};

    //const mat4 m{node->world_from_node()};
    line_renderer.set_thickness(30.0f);
    line_renderer.add_line(red,   10.0f, O, red,   10.0f, axis_x );
    line_renderer.add_line(green, 10.0f, O, green, 10.0f, axis_y );
    line_renderer.add_line(blue,  10.0f, O, blue,  10.0f, axis_z );

    const Hover_entry& content = context.scene_view.get_hover(Hover_entry::content_slot);
    if (
        !content.valid                  ||
        !content.position.has_value()   ||
        !content.normal.has_value()     ||
        (content.scene_mesh == nullptr) ||
        !content.geometry
    ) {
        return;
    }

    ERHE_VERIFY(content.scene_mesh != nullptr);
    ERHE_VERIFY(content.scene_mesh_primitive_index != std::numeric_limits<std::size_t>::max());

    erhe::geometry::Geometry& geometry = *content.geometry.get();
    GEO::Mesh& geo_mesh = geometry.get_mesh();

    const GEO::index_t facet = content.facet;
    const GEO::index_t corner_count = geo_mesh.facets.nb_corners(content.facet);
    if (corner_count < 3) {
        return;
    }

    const glm::vec3 hover_position_in_world = content.position.value();

    const erhe::scene::Node* node = content.scene_mesh->get_node();
    if (node == nullptr) {
        return;
    }

    glm::vec3 n = glm::normalize(content.normal.value());

    struct Vertex_position {
        GEO::index_t vertex;
        glm::vec3    position;
    };
    std::vector<Vertex_position> vertex_positions;
    for (GEO::index_t corner : geo_mesh.facets.corners(facet)) {
        const GEO::index_t vertex     = geo_mesh.facet_corners.vertex(corner);
        const GEO::vec3f   p_in_mesh_ = get_pointf(geo_mesh.vertices, vertex);
        const glm::vec3    p_in_mesh  = to_glm_vec3(p_in_mesh_);
        vertex_positions.emplace_back(vertex, p_in_mesh);
    }

    for (const Vertex_position& vertex_position : vertex_positions) {
        const glm::vec3 p = node->transform_point_from_local_to_world(vertex_position.position);
        line_renderer.add_lines(
            glm::vec4{1.0f, 0.0f, 1.0f, 1.0f},
            {{ p, p + n }}
        );
    }
}

} // namespace explorer
