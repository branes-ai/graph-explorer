#pragma once

#include "graph/graph.hpp"

#include "erhe_imgui/imgui_window.hpp"

#include <memory>

namespace erhe::commands {
    class Commands;
}
namespace erhe::graph {
    class Link;
    class Pin;
}
namespace erhe::imgui {
    class Imgui_windows;
}

namespace sw::dfa {
    struct DomainFlowGraph;
}

namespace ax::NodeEditor {
    class EditorContext;
}

namespace explorer {

class Explorer_context;
class Explorer_message;
class Explorer_message_bus;

class Graph_node;
class Node_style_editor_window;
class Selection;

class Graph_window : public erhe::imgui::Imgui_window
{
public:
    Graph_window(
        erhe::commands::Commands&    commands,
        erhe::imgui::Imgui_renderer& imgui_renderer,
        erhe::imgui::Imgui_windows&  imgui_windows,
        Explorer_context&            explorer_context,
        Explorer_message_bus&        explorer_message_bus
    );
    ~Graph_window() noexcept override;

    // Implements Imgui_window
    void imgui() override;
    auto flags() -> ImGuiWindowFlags override;

    [[nodiscard]] auto get_selection        () -> Selection&;
    [[nodiscard]] auto get_domain_flow_graph() const -> sw::dfa::DomainFlowGraph*;
    void set_domain_flow_graph(const std::shared_ptr<sw::dfa::DomainFlowGraph>& dfg);
    auto get_ui_graph         () -> Graph&;
    auto get_node_editor      () -> ax::NodeEditor::EditorContext*;

    void clear       ();
    void fit         ();
    void graph_loaded();

private:
    void clear_constructor_subset();
    void on_message(Explorer_message& message);

    Explorer_context&                              m_context;
    Graph                                          m_graph;
    std::unique_ptr<Selection>                     m_selection;
    std::unique_ptr<ax::NodeEditor::EditorContext> m_node_editor;
    std::unique_ptr<Node_style_editor_window>      m_style_editor_window;
    bool                                           m_pending_navigate_to_content{false};
    std::shared_ptr<sw::dfa::DomainFlowGraph>      m_dfg;
};

} // namespace explorer
