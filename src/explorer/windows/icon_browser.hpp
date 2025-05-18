#pragma once

#include "erhe_imgui/imgui_window.hpp"

#include <imgui/imgui.h>

namespace erhe::imgui {
    class Imgui_windows;
}

namespace explorer {

class Explorer_context;

class Icon_browser : public erhe::imgui::Imgui_window
{
public:
    Icon_browser(
        erhe::imgui::Imgui_renderer& imgui_renderer,
        erhe::imgui::Imgui_windows&  imgui_windows
    );

    // Implements Imgui_window
    void imgui() override;

private:
    ImGuiTextFilter m_name_filter;
};

} // namespace explorer
