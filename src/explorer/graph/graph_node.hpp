#pragma once

#include "erhe_graph/node.hpp"

#include <imgui/imgui.h>

#include <vector>

namespace ax::NodeEditor { class EditorContext; }

namespace explorer {

class Explorer_context;
class Sheet;
class Graph;

class Node_edge
{
public:
    static constexpr int left   = 0;
    static constexpr int right  = 1;
    static constexpr int top    = 2;
    static constexpr int bottom = 3;
    static constexpr int count  = 4;
};

auto get_node_edge_name(int direction) -> const char*;

class Graph_node : public erhe::graph::Node
{
public:
    Graph_node(const std::string_view label, std::size_t payload);

    void make_input_pin (std::size_t key, std::string_view name);
    void make_output_pin(std::size_t key, std::string_view name);

    void node_editor(Explorer_context& context, ax::NodeEditor::EditorContext& node_editor);

    [[nodiscard]] auto get_payload() -> size_t;
    virtual void imgui();

protected:
    friend class Node_properties_window;

    struct Node_context
    {
        Explorer_context&              context;
        ax::NodeEditor::EditorContext& node_editor;
        ImDrawList*                    draw_list      {nullptr};
        float                          pin_width;
        float                          pin_label_width;
        float                          side_width     {0.0f};
        float                          center_width;
        ImVec2                         pin_table_size {};
        ImVec2                         node_table_size{};
        ImFont*                        icon_font      {nullptr};
        int                            pin_edge       {0};
        float                          edge_x         {0.0f};
    };
    void show_pins(Node_context& context, std::vector<erhe::graph::Pin>& pins);

    void text_unformatted_edge(int edge, const char* text);

    static constexpr std::size_t pin_key_todo = 1;

    std::size_t m_payload;
    int         m_input_pin_edge {Node_edge::left};
    int         m_output_pin_edge{Node_edge::right};
};

} // namespace explorer
