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

    erhe::scene_renderer::Cube_instance_buffer& cube_instance_buffer = m_cube_renderer.get_buffer();

    // TODO - Initialize from graph
    int edge = 128;
    float scale = static_cast<float>(edge - 1);
    float size = 0.75f;
    for (int x = 0; x < edge; ++x) {
        const float r_x = static_cast<float>(x) / scale;
        std::vector<erhe::scene_renderer::Cube_instance> cubes;
        cubes.reserve(edge * edge);
        for (int y = 0; y < edge; ++y) {
            const float r_y = static_cast<float>(y) / scale;
            for (int z = 0; z < edge; ++z) {
                const float r_z = static_cast<float>(z) / scale;
                cubes.emplace_back(
                    glm::vec4{ static_cast<float>(x * 2 - 4), static_cast<float>(y * 2 - 4), static_cast<float>(z * 2 - 4), size },
                    glm::vec4{ r_x, r_y, r_z, 1.0f }
                );
            }
        }
        size_t frame = cube_instance_buffer.append_frame(cubes);
        m_frames.push_back(frame);
   }
}

auto get_operator_color(sw::dfa::DomainFlowOperator op) -> glm::vec4
{
    static_cast<void>(op);
    return glm::vec4{0.2f, 1.0f, 0.2f, 1.0f};
#if 0
    using namespace sw::dfa;
    switch (op) {
        case DomainFlowOperator::FUNCTION:                 return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::FUNCTION_ARGUMENT:        return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::FUNCTION_RETURN:          return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::FUNCTION_RESULT:          return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::ABS:                      return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::ADD:                      return glm::vec4{1.0f, 1.0f, 0.0f, 1.0f};
        case DomainFlowOperator::CAST:                     return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::CLAMP:                    return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::CONCAT:                   return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::CONSTANT:                 return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::CONV2D:                   return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::CONV3D:                   return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::DEPTHWISE_CONV2D:         return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::EXP:                      return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::FC:                       return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::GATHER:                   return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::SUB:                      return glm::vec4{0.0f, 0.0f, 1.0f, 1.0f};
        case DomainFlowOperator::MUL:                      return glm::vec4{0.0f, 1.0f, 0.0f, 1.0f};
        case DomainFlowOperator::DIV:                      return glm::vec4{1.0f, 0.0f, 0.0f, 1.0f};
        case DomainFlowOperator::LINEAR:                   return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::MATMUL:                   return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::NEGATE:                   return glm::vec4{0.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::PAD:                      return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::MAXPOOL2D:                return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::AVGPOOL2D:                return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::RECIPROCAL:               return glm::vec4{1.0f, 0.0f, 1.0f, 1.0f};
        case DomainFlowOperator::REDUCE_ALL:               return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::REDUCE_MAX:               return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::REDUCE_MIN:               return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::REDUCE_SUM:               return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::REDUCE_PROD:              return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::RELU:                     return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::ATAN:                     return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::SIGMOID:                  return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::RESHAPE:                  return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::TRANSPOSE:                return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::TRANSPOSE_CONV2D:         return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::UNKNOWN:                  return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        case DomainFlowOperator::DomainFlowOperator_COUNT: return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        default:                                           return glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    }
#endif
}

void Wavefront_visualization::fetch_wavefront(const sw::dfa::DomainFlowNode& node, std::shared_ptr<erhe::scene::Node> scene_graph_node)
{
    static const float size = 0.4f;
    const glm::vec4 color = get_operator_color(node.getOperatorType());
    const glm::mat4 world_from_node = scene_graph_node->world_from_node();
    const sw::dfa::Schedule<sw::dfa::DomainFlowNode::ConstraintCoefficientType>& schedule = node.getSchedule();
    erhe::scene_renderer::Cube_instance_buffer& cube_instance_buffer = m_cube_renderer.get_buffer();
    for (
        std::map<std::size_t, sw::dfa::Wavefront>::const_iterator i = schedule.begin(), end = schedule.end();
        i != end;
        ++i
    ) {
        //// const std::size_t time = i->first;
        const sw::dfa::Wavefront& wavefront = i->second;

        std::vector<erhe::scene_renderer::Cube_instance> cubes;
        cubes.reserve(wavefront.size());

        for (const sw::dfa::IndexPoint& index_point : wavefront) {
            const std::vector<int>& p = index_point.coordinates;
            const int x = (p.size() >= 1) ? p[0] : 0;
            const int y = (p.size() >= 2) ? p[1] : 0;
            const int z = (p.size() >= 3) ? p[2] : 0;
            const glm::vec3 position = world_from_node * glm::vec4{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 1.0f};
            cubes.emplace_back(
                glm::vec4{position.x, position.y, position.z, size},
                color
            );
        }
        size_t frame = cube_instance_buffer.append_frame(cubes);
        m_frames.push_back(frame);
    }
}

auto Wavefront_visualization::check_for_selection_changes(Explorer_message& message) -> bool
{
    using namespace erhe::bit;

    sw::dfa::DomainFlowGraph* dfg = m_context.graph_window->get_domain_flow_graph();
    if (dfg == nullptr) {
        log_graph->error("dfg is nullptr");
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

    erhe::scene_renderer::Cube_instance_buffer& cube_instance_buffer = m_cube_renderer.get_buffer();
    cube_instance_buffer.clear();
    m_frames.clear();
    m_frame_index = 0;

    Selection& selection = m_context.graph_window->get_selection();
    std::vector<std::shared_ptr<Graph_node>> selected_graph_nodes = selection.get_all<Graph_node>();
    if (selected_graph_nodes.empty()) {
        return;
    }

    for (const auto& graph_ui_node : selected_graph_nodes) {
        std::size_t node_id = graph_ui_node->get_payload();
        std::shared_ptr<erhe::scene::Node> visualization_node = graph_ui_node->get_convex_hull_visualization();
        const sw::dfa::DomainFlowNode& node = dfg->graph.node(node_id);

        if (!node.isOperator() || !visualization_node) {
            continue;
        }
        fetch_wavefront(node, visualization_node);
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
    if (!m_frames.empty()) {
        property_editor.add_entry(
            "Frame", 
            [this]() {
                ImGui::SliderInt(
                    "##frame",
                    &m_frame_index,
                    0,
                    static_cast<int>(m_frames.size()) - 1,
                    "%d"
                );
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

    if (m_frames.empty()) {
        return;
    }

    erhe::scene_renderer::Cube_renderer::Render_parameters parameters{
        .frame    = m_frames[m_frame_index],
        .pipeline = *m_pipeline.get(),
        .camera   = context.camera,
        .viewport = context.viewport
    };
    m_cube_renderer.render(parameters);
}

} // namespace explorer

