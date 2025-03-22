#pragma once

#include "tools/tool.hpp"
#include "erhe_imgui/imgui_window.hpp"

#include <string>
#include <vector>

namespace erhe::imgui {
    class Imgui_windows;
}

namespace explorer {

class Explorer_context;
class Explorer_message_bus;

class Hover_tool
    : public erhe::imgui::Imgui_window
    , public Tool
{
public:
    Hover_tool(
        erhe::imgui::Imgui_renderer& imgui_renderer,
        erhe::imgui::Imgui_windows&  imgui_windows,
        Explorer_context&            explorer_context,
        Explorer_message_bus&        explorer_message_bus,
        Tools&                       tools
    );

    // Implements Tool
    void tool_render    (const Render_context& context) override;

    // Implements Imgui_window
    void imgui() override;

    void add_line(const std::string& line);

private:
    bool                     m_show_snapped_grid_position     {false};
    bool                     m_geometry_debug_hover_facet_only{true};
    std::vector<std::string> m_text_lines;
};

} // namespace explorer
