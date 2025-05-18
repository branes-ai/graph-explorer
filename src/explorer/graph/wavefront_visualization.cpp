#include "graph/wavefront_visualization.hpp"
#include "graph/timeline_window.hpp"
#include "graph/node_convex_hull_visualization.hpp"
#include "explorer_log.hpp"
#include "explorer_context.hpp"
#include "explorer_message_bus.hpp"
#include "explorer_rendering.hpp"
#include "graph/graph_node.hpp"
#include "graph/graph_window.hpp"
#include "renderers/render_context.hpp"
#include "renderers/programs.hpp"
#include "windows/property_editor.hpp"
#include "tools/selection_tool.hpp"

#include "erhe_bit/bit_helpers.hpp"
#include "erhe_imgui/imgui_windows.hpp"
#include "erhe_imgui/imgui_renderer.hpp"
#include "erhe_math/math_util.hpp"
#include "erhe_primitive/material.hpp"
#include "erhe_scene/node.hpp"
#include "erhe_scene_renderer/cube_instance_buffer.hpp"

#include <dfa/dfa.hpp>

namespace explorer {

Wavefront_visualization::Wavefront_visualization(
    erhe::graphics::Instance&                graphics_instance,
    erhe::scene_renderer::Program_interface& program_interface,
    erhe::imgui::Imgui_renderer&             imgui_renderer,
    erhe::imgui::Imgui_windows&              imgui_windows,
    Explorer_context&                        explorer_context,
    Explorer_message_bus&                    explorer_message_bus,
    Explorer_rendering&                      explorer_rendering,
    Programs&                                programs
)
    : Imgui_window   {imgui_renderer, imgui_windows, "Wavefront Visualization", "waverfront_visualization"}
    , m_context      {explorer_context}
    , m_cube_renderer{graphics_instance, program_interface}
    , m_start_color  {0.0f, 0.0f, 0.0f, 1.0f}
    , m_end_color    {1.0f, 1.0f, 1.0f, 1.0f}

{
    explorer_message_bus.add_receiver(
        [&](Explorer_message& message) {
            on_message(message);
        }
    );
    explorer_rendering.add(this);

    m_pipeline = std::make_unique<erhe::graphics::Pipeline>(
        erhe::graphics::Pipeline_data{
            .name           = "cubes",
            .shader_stages  = &programs.cubes.shader_stages,
            .vertex_input   = &m_empty_vertex_input,
            .input_assembly = erhe::graphics::Input_assembly_state::triangles,
            .rasterization  = erhe::graphics::Rasterization_state::cull_mode_back_ccw,
            .depth_stencil  = erhe::graphics::Depth_stencil_state::depth_test_enabled_stencil_test_disabled(true),
            .color_blend    = erhe::graphics::Color_blend_state::color_blend_disabled
        }
    );
}

void Wavefront_visualization::fetch_wavefront(Graph_node& graph_ui_node)
{
    std::size_t node_id = graph_ui_node.get_payload();
    sw::dfa::DomainFlowGraph* dfg = m_context.graph_window->get_domain_flow_graph();
    if (dfg == nullptr) {
        m_pending_update = true;
        return;
    }
    const sw::dfa::DomainFlowNode& node = dfg->graph.node(node_id);
    if (!node.isOperator()) {
        return;
    }

    static const float size = 0.4f;
    const sw::dfa::Schedule<sw::dfa::DomainFlowNode::ConstraintCoefficientType>& schedule = node.getSchedule();

    std::vector<Wavefront_frame>& frames = graph_ui_node.wavefront_frames();
    frames.clear();
    erhe::math::Bounding_box aabb{};
    for (
        std::map<std::size_t, sw::dfa::Wavefront>::const_iterator i = schedule.begin(), end = schedule.end();
        i != end;
        ++i
    ) {
        const int time = static_cast<int>(i->first);
        const sw::dfa::Wavefront& wavefront = i->second;

        std::vector<uint32_t> cubes(wavefront.size());
        std::size_t cube_index = 0;
        for (const sw::dfa::IndexPoint& index_point : wavefront) {
            const std::vector<int>& p = index_point.coordinates;
            const int x = (p.size() >= 1) ? p[0] : 0;
            const int y = (p.size() >= 2) ? p[1] : 0;
            const int z = (p.size() >= 3) ? p[2] : 0;
            aabb.include(glm::vec3{x, y, z});
            cubes[cube_index++] = erhe::scene_renderer::pack_x11y11z10(x, y, z);
        }
        frames.push_back(
            Wavefront_frame{{}, {}, m_cube_renderer.make_buffer(cubes), time}
        );
    }
    glm::vec3 color_bias = -aabb.min;
    glm::vec3 color_scale = glm::vec3{1.0f} / aabb.diagonal();
    for (Wavefront_frame& frame : frames) {
        frame.color_bias  = glm::vec4{color_bias, 0.0f};
        frame.color_scale = glm::vec4{color_scale, 1.0f};
    }
    std::sort(
        frames.begin(),
        frames.end(),
        [](const Wavefront_frame& lhs, const Wavefront_frame& rhs)
        {
            return lhs.time < rhs.time;
        }
    );
}

void Wavefront_visualization::update_wavefront_visualization()
{
    sw::dfa::DomainFlowGraph* dfg = m_context.graph_window->get_domain_flow_graph();
    if (dfg == nullptr) {
        m_pending_update = true;
        return;
    }

    Graph& ui_graph = m_context.graph_window->get_ui_graph();
    const std::vector<erhe::graph::Node*>& nodes = ui_graph.get_nodes();
    for (erhe::graph::Node* node : nodes) {
        Graph_node* graph_ui_node = dynamic_cast<Graph_node*>(node);
        if (graph_ui_node == nullptr) {
            continue;
        }
        fetch_wavefront(*graph_ui_node);
    }

    m_pending_update = false;
}

void Wavefront_visualization::on_message(Explorer_message& message)
{
    using namespace erhe::bit;
    if (test_any_rhs_bits_set(message.update_flags, Message_flag_bit::c_flag_bit_graph_loaded)) {
        update_wavefront_visualization();
    }
}

void Wavefront_visualization::imgui()
{
    Property_editor property_editor;

    property_editor.add_entry(
        "Hull Opacity",
        [this]() {
            erhe::primitive::Material* material = m_context.node_convex_hull_visualization->get_material();
            if (material != nullptr) {
                ImGui::SliderFloat("##", &material->opacity, 0.0f, 1.0f);
            }
        }
    );

    property_editor.add_entry(
        "Point Size", 
        [this]() {
            ImGui::SliderFloat("##", &m_cube_size, 0.0, 1.0);
        }
    );

    property_editor.add_entry(
        "Start Color",
        [this]() {
            ImGui::ColorEdit3("##", m_start_color, ImGuiColorEditFlags_NoAlpha);
        }
    );

    property_editor.add_entry(
        "End Color", 
        [this]() {
            ImGui::ColorEdit3("##", m_end_color, ImGuiColorEditFlags_NoAlpha);
        }
    );

    property_editor.show_entries();

}

void Wavefront_visualization::render(const Render_context& context)
{
    if (m_pending_update) {
        update_wavefront_visualization();
    }

    Selection& selection = m_context.graph_window->get_selection();
    std::vector<std::shared_ptr<Graph_node>> selected_graph_nodes = selection.get_all<Graph_node>();

    std::size_t first = std::numeric_limits<std::size_t>::max();
    std::size_t last  = std::numeric_limits<std::size_t>::lowest();
    Graph& ui_graph = m_context.graph_window->get_ui_graph();
    const std::vector<erhe::graph::Node*>& nodes = ui_graph.get_nodes();
    for (erhe::graph::Node* node_ : nodes) {
        Graph_node* graph_ui_node = dynamic_cast<Graph_node*>(node_);
        if (graph_ui_node == nullptr) {
            continue;
        }
        if (!graph_ui_node->show_wavefront()) {
            continue;
        }
        const std::vector<Wavefront_frame>& frames = graph_ui_node->wavefront_frames();
        if (frames.empty()) {
            continue;
        }
        const std::size_t time_offset = graph_ui_node->get_wavefront_time_offset();
        first = std::min(first, frames.front().time + time_offset);
        last  = std::max(first, frames.back ().time + time_offset);
    }

    std::size_t length = last - first + 1;
    m_context.timeline_window->set_timeline_length(static_cast<float>(length));
    m_frame_index = static_cast<int>(first) + static_cast<int>(m_context.timeline_window->get_play_position());

    for (erhe::graph::Node* node_ : nodes) {
        Graph_node* graph_ui_node = dynamic_cast<Graph_node*>(node_);
        if (graph_ui_node == nullptr) {
            continue;
        }

        if (!graph_ui_node->show_wavefront()) {
            continue;
        }

        const std::shared_ptr<erhe::scene::Node>& node   = graph_ui_node->get_index_space_node();
        const std::vector<Wavefront_frame>&       frames = graph_ui_node->wavefront_frames();
        if (!node || frames.empty()) {
            continue;
        }

        const int time_offset = graph_ui_node->get_wavefront_time_offset();
        const int node_time   = m_frame_index - time_offset;
        if ((node_time >= 0) && (node_time < frames.size())) {
            const Wavefront_frame& frame = frames.at(node_time);
            erhe::scene_renderer::Cube_renderer::Render_parameters parameters{
                .cube_instance_buffer = *frame.cube_instance_buffer.get(),
                .pipeline             = *m_pipeline.get(),
                .camera               = context.camera,
                .node                 = node,
                .primitive_settings   = {},
                .viewport             = context.viewport,
                .cube_size            = glm::vec4{m_cube_size, m_cube_size, m_cube_size, 0.0f},
                .color_bias           = frame.color_bias,
                .color_scale          = frame.color_scale,
                .color_start          = glm::vec4{m_start_color[0], m_start_color[1], m_start_color[2], 1.0f},
                .color_end            = glm::vec4{m_end_color[0], m_end_color[1], m_end_color[2], 1.0f}
            };
            m_cube_renderer.render(parameters);
        }
    }
}

} // namespace explorer

