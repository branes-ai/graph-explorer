#include "graph/node_properties.hpp"
#include "graph/graph_node.hpp"
#include "graph/graph_window.hpp"
#include "explorer_context.hpp"
#include "tools/selection_tool.hpp"

#include "erhe_defer/defer.hpp"
#include "erhe_imgui/imgui_node_editor.h"
#include "erhe_imgui/imgui_windows.hpp"
#include "erhe_imgui/imgui_renderer.hpp"
#include "erhe_bit/bit_helpers.hpp"
#include "erhe_profile/profile.hpp"
#include "erhe_verify/verify.hpp"

#include <dfa/dfa.hpp>

#include <fmt/format.h>

#if defined(ERHE_GUI_LIBRARY_IMGUI)
#   include <imgui/imgui.h>
#   include <imgui/misc/cpp/imgui_stdlib.h>
#endif

#include <string>
#include <sstream>


namespace explorer {

Node_properties_window::Node_properties_window(
    erhe::imgui::Imgui_renderer& imgui_renderer,
    erhe::imgui::Imgui_windows&  imgui_windows,
    Explorer_context&            explorer_context
)
    : Imgui_window{imgui_renderer, imgui_windows, "Node Properties", "node_properties"}
    , m_context   {explorer_context}
{
}

void Node_properties_window::on_begin()
{
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
}

void Node_properties_window::on_end()
{
    ImGui::PopStyleVar();
}

void Node_properties_window::item_flags(const std::shared_ptr<erhe::Item_base>& item)
{
    ERHE_PROFILE_FUNCTION();

    m_property_editor.push_group("Flags", ImGuiTreeNodeFlags_None, 0.0f);

    using namespace erhe::bit;
    using Item_flags = erhe::Item_flags;

    const uint64_t flags = item->get_flag_bits();
    for (uint64_t bit_position = 0; bit_position < Item_flags::count; ++ bit_position) {
        m_property_editor.add_entry(Item_flags::c_bit_labels[bit_position], [item, bit_position, flags, this]() {
            const uint64_t bit_mask = uint64_t{1} << bit_position;
            bool           value    = test_all_rhs_bits_set(flags, bit_mask);
            if (ImGui::Checkbox("##", &value)) {
                if (bit_mask == Item_flags::selected) {
                    if (value) {
                        m_context.selection->add_to_selection(item);
                    } else {
                        m_context.selection->remove_from_selection(item);
                    }
                } else {
                    item->set_flag_bits(bit_mask, value);
                }
            }
        });
    }

    m_property_editor.pop_group();
}

void Node_properties_window::node_properties(Graph_node& ui_node)
{
    ax::NodeEditor::EditorContext* node_editor = m_context.graph_window->get_node_editor();

    sw::dfa::DomainFlowGraph* dfg = m_context.graph_window->get_domain_flow_graph();
    if (dfg != nullptr) {
        m_property_editor.push_group("Domain Flow Node", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed, 0.0f);
        using namespace sw::dfa;
        std::size_t node_id = ui_node.get_payload();
        const DomainFlowNode& node = dfg->graph.node(node_id);
        m_property_editor.add_entry("Node ID", [node_id]() { ImGui::Text("%zu", node_id); });
        m_property_editor.add_entry("Name",    [&node  ]() { ImGui::TextUnformatted(node.getName().c_str()); });
        m_property_editor.add_entry("Op",      [&node  ]() { std::stringstream ss; ss << node.getOperator(); ImGui::TextUnformatted(ss.str().c_str()); });
        m_property_editor.add_entry("Depth",   [&node  ]() { ImGui::Text("%d", node.getDepth());});

        const std::size_t attribute_count = node.getNrAttributes();
        if (attribute_count > 0) {
            m_property_editor.push_group("Attributes", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed, 0.0f);
            for (const auto& [key, value] : node.getAttributes()) {
                m_property_editor.add_entry(key, [&value]() { ImGui::TextUnformatted(value.c_str());});
            }
            m_property_editor.pop_group();
        }

        std::size_t input_count = node.getNrInputs();
        if (input_count > 0) {
            m_property_editor.push_group("Inputs", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed, 0.0f);
            for (std::size_t i = 0; i < input_count; ++i) {
                std::string label = fmt::format("[{}]", i);
                m_property_editor.add_entry(label, [&node, i]() { ImGui::TextUnformatted(node.operandType.at(i).c_str());});
            }
            m_property_editor.pop_group();
        }

        std::size_t output_count = node.getNrOutputs();
        if (output_count > 0) {
            m_property_editor.push_group("Outputs", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed, 0.0f);
            for (std::size_t i = 0; i < output_count; ++i) {
                std::string label = fmt::format("[{}] : {}", i, node.getResultType(i));
                m_property_editor.add_entry(label, [&node, i]() { ImGui::TextUnformatted(node.getResultValue(i).c_str());});
            }
            m_property_editor.pop_group();
        }

        const auto complexity = node.getArithmeticComplexity();
        if (!complexity.empty()) {
            m_property_editor.push_group("Arithmetic Complexity", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed, 0.0f);
            int i = 0;
            for (const auto& entry : complexity) {
                std::string label = fmt::format("[{}]", i++);
                m_property_editor.add_entry(
                    label,
                    [entry]() {
                        bool first = true;
                        std::stringstream ss;
                        const std::string   s1    = std::get<0>(entry);
                        const std::string   s2    = std::get<1>(entry);
                        const std::uint64_t value = std::get<2>(entry);
                        if (!s1.empty()) {
                            ss << s1;
                            first = false;
                        }
                        if (!s2.empty()) {
                            if (!first) {
                                ss << " | ";
                            }
                            ss << s2;
                            first = false;
                        }
                        if (!first) {
                            ss << " | ";
                        }
                        ss << static_cast<std::size_t>(value);

                        ImGui::TextUnformatted(ss.str().c_str());
                    }
                );
            }
            m_property_editor.pop_group();
        }

        m_property_editor.pop_group();
    }

    m_property_editor.push_group("Wavefront", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed, 0.0f);
    m_property_editor.add_entry(
        "Show",
        [&ui_node]() {
            ImGui::Checkbox("##", &ui_node.m_show_wavefront);
        }
    );
    m_property_editor.add_entry(
        "Time offset",
        [&ui_node]() {
            std::size_t time_offset_ = ui_node.get_wavefront_time_offset();
            int time_offset = static_cast<int>(time_offset_);
            ImGui::InputInt("##", &time_offset, 1, 10);
            if (ImGui::IsItemEdited()) {
                ui_node.set_wavefront_time_offset(static_cast<std::size_t>(time_offset));
            }
        }
    );
    const std::vector<Wavefront_frame>& frames = ui_node.wavefront_frames();
    if (!frames.empty()) {
        m_property_editor.add_entry(
            "First",
            [&ui_node, &frames]() {
                ImGui::Text("%zu", frames.front().time);
            }
        );
        m_property_editor.add_entry(
            "Last",
            [&ui_node, &frames]() {
                ImGui::Text("%zu", frames.back().time);
            }
        );
    }
    m_property_editor.pop_group();

    m_property_editor.push_group("Node Visual", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed, 0.0f);
    m_property_editor.add_entry(
        "Position",
        [&ui_node, node_editor]() {
            ImVec2 position = node_editor->GetNodePosition(ui_node.get_id());
            ImGui::DragFloat2("##", &position.x, 0.1f);
        }
    );

    m_property_editor.add_entry(
        "Inputs",
        [&ui_node]() {
            if (ImGui::BeginCombo("##inputs", get_node_edge_name(ui_node.m_input_pin_edge))) {
                for (int i = 0; i < Node_edge::count; ++i) {
                    bool selected = (ui_node.m_input_pin_edge == i);
                    ImGui::Selectable(get_node_edge_name(i), &selected, ImGuiSelectableFlags_None);
                    if (selected) {
                        ui_node.m_input_pin_edge = i;
                    }
                }
                ImGui::EndCombo();
            }
        }
    );
    m_property_editor.add_entry(
        "Outputs",
        [&ui_node]() {
            if (ImGui::BeginCombo("##outputs", get_node_edge_name(ui_node.m_output_pin_edge))) {
                for (int i = 0; i < Node_edge::count; ++i) {
                    bool selected = (ui_node.m_output_pin_edge == i);
                    ImGui::Selectable(get_node_edge_name(i), &selected, ImGuiSelectableFlags_None);
                    if (selected) {
                        ui_node.m_output_pin_edge = i;
                    }
                }
                ImGui::EndCombo();
            }
        }
    );
    m_property_editor.pop_group();
}

void Node_properties_window::imgui()
{
    ERHE_PROFILE_FUNCTION();

    m_property_editor.reset();

    const auto selected_graph_node = m_context.graph_window->get_selection().get<erhe::graph::Node>();
    if (selected_graph_node) {
        Graph_node* graph_node = dynamic_cast<Graph_node*>(selected_graph_node.get());
        if (graph_node != nullptr) {
            node_properties(*graph_node);
        }
    }

    m_property_editor.show_entries();
}

} // namespace explorer
