#pragma once

#include "erhe_imgui/imgui_window.hpp"

namespace erhe::imgui {
    class Imgui_windows;
}

namespace explorer {

class Explorer_context;

class Clipboard_window : public erhe::imgui::Imgui_window
{
public:
    Clipboard_window(
        erhe::imgui::Imgui_renderer& imgui_renderer,
        erhe::imgui::Imgui_windows&  imgui_windows,
        Explorer_context&            explorer
    );

    // Implements Imgui_window
    void imgui() override;

private:
    Explorer_context& m_context;
};

}
