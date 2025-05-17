#include "graph/wavefront_visualization.hpp"
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

auto get_operator_color(sw::dfa::DomainFlowOperator op) -> glm::vec4
{
    static_cast<void>(op);
    return glm::vec4{0.2f, 1.0f, 0.2f, 1.0f};
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

    std::shared_ptr<erhe::scene::Node> visualization_node = graph_ui_node.get_convex_hull_visualization();

    static const float size = 0.4f;
    //const glm::vec4 color = get_operator_color(node.getOperatorType());
    //const glm::mat4 world_from_node = scene_graph_node->world_from_node();
    const sw::dfa::Schedule<sw::dfa::DomainFlowNode::ConstraintCoefficientType>& schedule = node.getSchedule();
    //erhe::scene_renderer::Cube_instance_buffer& cube_instance_buffer = m_cube_renderer.get_buffer();
    std::vector<Wavefront_frame>& frames = graph_ui_node.wavefront_frames();
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
            cubes[cube_index++] = erhe::scene_renderer::pack_x11y11z10(x, y, z);
        }
        frames.push_back(
            Wavefront_frame{time, m_cube_renderer.make_buffer(cubes)}
        );
    }
}

auto Wavefront_visualization::check_for_selection_changes(Explorer_message& message) -> bool
{
    using namespace erhe::bit;

    sw::dfa::DomainFlowGraph* dfg = m_context.graph_window->get_domain_flow_graph();
    if (dfg == nullptr) {
        m_pending_update = true;
        return false;
    }

    if (test_any_rhs_bits_set(message.update_flags, Message_flag_bit::c_flag_bit_selection)) {
        for (const auto& item : message.no_longer_selected) {
            std::shared_ptr<Graph_node> graph_ui_node = std::dynamic_pointer_cast<Graph_node>(item);
            if (!graph_ui_node) {
                continue;
            }

            std::size_t node_id = graph_ui_node->get_payload();
            const sw::dfa::DomainFlowNode& node = dfg->graph.node(node_id);
            if (!node.isOperator()) {
                continue;
            }
            return true;
        }

        for (const auto& item : message.newly_selected) {
            std::shared_ptr<Graph_node> graph_ui_node = std::dynamic_pointer_cast<Graph_node>(item);
            if (!graph_ui_node) {
                continue;
            }
            std::size_t node_id = graph_ui_node->get_payload();
            const sw::dfa::DomainFlowNode& node = dfg->graph.node(node_id);
            if (!node.isOperator()) {
                continue;
            }
            return true;
        }
    }
    return false;
}

void Wavefront_visualization::update_wavefront_visualization()
{
    sw::dfa::DomainFlowGraph* dfg = m_context.graph_window->get_domain_flow_graph();
    if (dfg == nullptr) {
        m_pending_update = true;
        return;
    }

    Selection& selection = m_context.graph_window->get_selection();
    std::vector<std::shared_ptr<Graph_node>> selected_graph_nodes = selection.get_all<Graph_node>();
    if (selected_graph_nodes.empty()) {
        return;
    }

    for (const std::shared_ptr<Graph_node>& graph_ui_node : selected_graph_nodes) {
        fetch_wavefront(*graph_ui_node.get());
    }
    m_pending_update = false;
}

void Wavefront_visualization::on_message(Explorer_message& message)
{
    if (!check_for_selection_changes(message)) {
        return;
    }

    update_wavefront_visualization();
}

void Wavefront_visualization::imgui()
{
    Property_editor property_editor;

    property_editor.add_entry(
        "Show", 
        [this]() {
            ImGui::Checkbox("##show", &m_show);
        }
    );

    Selection& selection = m_context.graph_window->get_selection();
    std::vector<std::shared_ptr<Graph_node>> selected_graph_nodes = selection.get_all<Graph_node>();
    if (selected_graph_nodes.empty()) {
        return;
    }

    std::size_t first = std::numeric_limits<std::size_t>::max();
    std::size_t last = std::numeric_limits<std::size_t>::lowest();
    for (const std::shared_ptr<Graph_node>& graph_ui_node : selected_graph_nodes) {
        const std::vector<Wavefront_frame>& frames = graph_ui_node->wavefront_frames();
        if (frames.empty()) {
            continue;
        }
        const std::size_t time_offset = graph_ui_node->get_wavefront_time_offset();
        first = std::min(first, frames.front().time + time_offset);
        last  = std::max(first, frames.back ().time + time_offset);
    }

    if (first < last) {
        const int lo = static_cast<int>(first);
        const int hi = static_cast<int>(last);
        m_frame_index = std::clamp(m_frame_index, lo, hi);
        property_editor.add_entry(
            "Frame", 
            [this, lo, hi]() {
                ImGui::SliderInt("##frame", &m_frame_index, lo, hi, "%d");
            }
        );
    }

    property_editor.show_entries();

}

void Wavefront_visualization::render(const Render_context& context)
{
    if (!m_show) {
        return;
    }

    if (m_pending_update) {
        update_wavefront_visualization();
    }

    Selection& selection = m_context.graph_window->get_selection();
    std::vector<std::shared_ptr<Graph_node>> selected_graph_nodes = selection.get_all<Graph_node>();
    for (const std::shared_ptr<Graph_node>& graph_ui_node : selected_graph_nodes) {
        const std::shared_ptr<erhe::scene::Node>& node   = graph_ui_node->get_convex_hull_visualization();
        const std::vector<Wavefront_frame>&       frames = graph_ui_node->wavefront_frames();
        if (frames.empty()) {
            continue;
        }

        const int time_offset = graph_ui_node->get_wavefront_time_offset();
        const int node_time   = m_frame_index - time_offset;
        if ((node_time >= 0) && (node_time < frames.size())) {
            erhe::scene_renderer::Cube_renderer::Render_parameters parameters{
                .cube_instance_buffer = *frames.at(node_time).cube_instance_buffer.get(),
                .pipeline             = *m_pipeline.get(),
                .camera               = context.camera,
                .node                 = node,
                .primitive_settings   = {},
                .viewport             = context.viewport
            };
            m_cube_renderer.render(parameters);
        }
    }
}

} // namespace explorer

