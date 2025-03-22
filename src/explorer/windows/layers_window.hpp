#pragma once

#include "erhe_imgui/imgui_window.hpp"

namespace erhe::imgui {
    class Imgui_windows;
}

namespace explorer {

class Explorer_context;

class Layers_window : public erhe::imgui::Imgui_window
{
public:
    Layers_window(erhe::imgui::Imgui_renderer& imgui_renderer, erhe::imgui::Imgui_windows& imgui_windows, Explorer_context& explorer_context);

    // Implements Imgui_window
    void imgui() override;

private:
    Explorer_context& m_context;
};

} // namespace explorer
