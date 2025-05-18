#include "graph/graph_node.hpp"
#include "graph/graph_window.hpp"
#include "explorer_context.hpp"
#include "explorer_log.hpp"
#include "tools/selection_tool.hpp"

#include "erhe_defer/defer.hpp"
#include "erhe_graph/link.hpp"
#include "erhe_graph/pin.hpp"
#include "erhe_math/math_util.hpp"
#include "erhe_scene/node.hpp"

#include "erhe_imgui/imgui_node_editor.h"
#include "erhe_imgui/imgui_renderer.hpp"

namespace explorer {

auto get_node_edge_name(int direction) -> const char*
{
    switch (direction) {
        case Node_edge::left:   return "Left";
        case Node_edge::right:  return "Right";
        case Node_edge::top:    return "Top";
        case Node_edge::bottom: return "Bottom";
        default: return "?";
    }
}

Graph_node::Graph_node(const std::string_view label, std::size_t payload)
    : erhe::graph::Node{label}
    , m_payload        {payload}
{
}

auto Graph_node::get_payload() const -> size_t
{
    return m_payload;
}

auto Graph_node::get_convex_hull_visualization() -> std::shared_ptr<erhe::scene::Node>
{
    return m_convex_hull_visualization;
}

auto Graph_node::get_index_space_node() -> std::shared_ptr<erhe::scene::Node>
{
    return m_index_space_node;
}


void Graph_node::set_convex_hull_visualization(const std::shared_ptr<erhe::scene::Node>& node, const glm::vec3& index_space_offset)
{
    m_convex_hull_visualization = node;

    if (m_index_space_node) {
        m_index_space_node->recursive_remove();
        m_index_space_node.reset();
    }
    m_index_space_node = std::make_shared<erhe::scene::Node>("offset");
    m_index_space_node->set_parent          (node);
    m_index_space_node->set_parent_from_node(erhe::math::create_translation<float>(index_space_offset));
}

auto Graph_node::get_wavefront_time_offset() const -> int
{
    return m_wavefront_time_offset;
}

auto Graph_node::show_wavefront() const -> bool
{
    return m_show_wavefront;
}

void Graph_node::get_time_range(int& first, int& last) const
{
    if (m_wavefront_frames.empty()) {
        first = 0;
        last = 0;
        return;
    }
    first = m_wavefront_frames.front().time;
    last = m_wavefront_frames.back().time;
}

void Graph_node::set_wavefront_time_offset(int offset)
{
    m_wavefront_time_offset = offset;
}

auto Graph_node::wavefront_frames() -> std::vector<Wavefront_frame>&
{
    return m_wavefront_frames;
}

void Graph_node::make_input_pin(std::size_t key, std::string_view name)
{
    base_make_input_pin(key, name);
}

void Graph_node::make_output_pin(std::size_t key, std::string_view name)
{
    base_make_output_pin(key, name);
}

void Graph_node::imgui()
{
}

void Graph_node::node_editor(Explorer_context& explorer_context, ax::NodeEditor::EditorContext& node_editor, Graph_window& graph_window)
{
    ImGui::PushID(static_cast<int>(get_id()));
    ERHE_DEFER( ImGui::PopID(); );

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{0.0f, 0.0f});
    ERHE_DEFER( ImGui::PopStyleVar(1); );

    ax::NodeEditor::NodeId node_id{get_id()};
    node_editor.BeginNode(node_id);
    Node_context context {
        .context         = explorer_context,
        .node_editor     = node_editor,
        .pin_width       =   0.0f,
        .pin_label_width = 160.0f,
        .center_width    = 140.0f,
        .icon_font       = explorer_context.imgui_renderer->icon_font()
    };
    context.side_width      = context.pin_width + context.pin_label_width;
    context.pin_table_size  = ImVec2{context.side_width, 0.0f};
    context.node_table_size = ImVec2{context.center_width + 2.0f * context.side_width, 0.0f};
    context.draw_list       = ImGui::GetWindowDrawList();

    const ImVec2 node_position = node_editor.GetNodePosition(node_id);
    const ImVec2 node_size     = node_editor.GetNodeSize(node_id);
    const ImVec2 top_left      = node_position;
    const ImVec2 bottom_right  = top_left + node_size;
    const float  left_edge     = top_left.x;
    const float  right_edge    = bottom_right.x;

    ImGui::BeginTable("##NodeTable", 3, ImGuiTableFlags_None, context.node_table_size);
    ImGui::TableSetupColumn("lhs",    ImGuiTableColumnFlags_WidthFixed, context.side_width);
    ImGui::TableSetupColumn("center", ImGuiTableColumnFlags_WidthFixed, context.center_width);
    ImGui::TableSetupColumn("rhs",    ImGuiTableColumnFlags_WidthFixed, context.side_width);

    // "Header" row
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(m_name.c_str());
    }

    ImGui::TableNextRow();

    {
        // Left edge
        ImGui::TableSetColumnIndex(0);
        context.pin_edge      = Node_edge::left;
        context.edge_x        = left_edge;
        if (m_input_pin_edge == Node_edge::left) {
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_SourceDirection, ImVec2{1.0f, 0.0});
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_TargetDirection, ImVec2{-1.0f, 0.0});
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_PivotAlignment,  ImVec2{-0.75f, 0.5f});
            show_pins(context, get_input_pins());
            node_editor.PopStyleVar(3);
        }
        if (m_output_pin_edge == Node_edge::left) {
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_SourceDirection, ImVec2{-1.0f, 0.0});
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_TargetDirection, ImVec2{ 1.0f, 0.0});
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_PinArrowSize,   0.0f);
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_PinArrowWidth,  0.0f);
            show_pins(context, get_output_pins());
            node_editor.PopStyleVar(4);
        }

        // Content
        ImGui::TableSetColumnIndex(1);
        imgui();

        // Right edge
        ImGui::TableSetColumnIndex(2);
        context.pin_edge      = Node_edge::right;
        context.edge_x        = right_edge;
        if (m_input_pin_edge == Node_edge::right) {
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_SourceDirection, ImVec2{1.0f, 0.0});
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_TargetDirection, ImVec2{1.0f, 0.0});
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_PivotAlignment,  ImVec2{1.75f, 0.5f});
            show_pins(context, get_input_pins());
            node_editor.PopStyleVar(3);
        }
        if (m_output_pin_edge == Node_edge::right) {
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_SourceDirection, ImVec2{1.0f, 0.0});
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_TargetDirection, ImVec2{1.0f, 0.0});
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_PinArrowSize,  0.0f);
            node_editor.PushStyleVar(ax::NodeEditor::StyleVar_PinArrowWidth, 0.0f);
            show_pins(context, get_output_pins());
            node_editor.PopStyleVar(4);
        }
    }

    ImGui::EndTable();

    node_editor.EndNode();
    const bool item_selection   = is_selected();
    const bool editor_selection = node_editor.IsNodeSelected(get_id());
    if (item_selection != editor_selection) {
        if (editor_selection) {
            graph_window.get_selection().add_to_selection(shared_from_this());
        } else {
            graph_window.get_selection().remove_from_selection(shared_from_this());
        }
    }
}

void Graph_node::text_unformatted_edge(int edge, const char* text)
{
    switch (edge) {
        case Node_edge::left: {
            ImGui::TextUnformatted(text);
            break;
        }
        case Node_edge::right: {
            float column_width = ImGui::GetColumnWidth();
            float text_width   = ImGui::CalcTextSize(text).x;
            float padding      = column_width - text_width;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding);
            ImGui::TextUnformatted(text);
            break;
        }
        default: {
            ImGui::TextUnformatted("!TODO!"); // TODO
            break;
        }
    }
}

void Graph_node::show_pins(Node_context& context, std::vector<erhe::graph::Pin>& pins)
{
    float half_extent = 10.0f;
    for (const erhe::graph::Pin& pin : pins) {
        text_unformatted_edge(context.pin_edge, pin.get_name().data());

        const ImVec2 cell_min      = ImGui::GetItemRectMin();
        const ImVec2 cell_max      = ImGui::GetItemRectMax();
        const float  cell_center_y = 0.5f * (cell_min.y + cell_max.y);
        const ImVec2 pin_center{context.edge_x, cell_center_y};
        const ImVec2 min{pin_center.x - half_extent, pin_center.y - half_extent};
        const ImVec2 max{pin_center.x + half_extent, pin_center.y + half_extent};

        context.node_editor.BeginPin(ax::NodeEditor::PinId{&pin}, pin.is_source() ? ax::NodeEditor::PinKind::Output : ax::NodeEditor::PinKind::Input);
        context.node_editor.PinRect(min, max);
        context.node_editor.EndPin();

        context.draw_list->AddRectFilled(min, max, 0xff444444, 4.0f, ImDrawFlags_RoundCornersAll);
        context.draw_list->AddRect      (min, max, 0xffcccccc, 4.0f, ImDrawFlags_RoundCornersAll, 2.0f);
    }
}

} // namespace explorer
