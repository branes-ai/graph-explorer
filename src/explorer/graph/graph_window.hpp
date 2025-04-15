#pragma once

#include "graph/graph.hpp"

#include "erhe_commands/command.hpp"
//#include "erhe_dataformat/dataformat.hpp"
#include "erhe_graph/graph.hpp"
#include "erhe_graph/node.hpp"
#include "erhe_imgui/imgui_window.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

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
namespace ax::NodeEditor {
    class EditorContext;
}

namespace explorer {

class Explorer_context;
class Explorer_message;
class Explorer_message_bus;

class Graph_node;
class Node_style_editor_window;

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

private:
    auto make_constant   () -> Graph_node*;
    auto make_add        () -> Graph_node*;

    void on_message(Explorer_message& message);

    Explorer_context&                               m_context;
    Graph                                           m_graph;
    std::unique_ptr<ax::NodeEditor::EditorContext>  m_node_editor;

    std::unique_ptr<Node_style_editor_window>       m_style_editor_window;
    std::vector<std::shared_ptr<Graph_node>>        m_nodes;
};

} // namespace explorer
