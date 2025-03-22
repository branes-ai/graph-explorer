#pragma once

#include "erhe_imgui/imgui_window.hpp"

namespace erhe::imgui {
    class Imgui_windows;
}

namespace explorer {

class Explorer_context;

// ImGui window for showing downsample steps for a Post_processing node
class Post_processing_window : public erhe::imgui::Imgui_window
{
public:
    Post_processing_window(erhe::imgui::Imgui_renderer& imgui_renderer, erhe::imgui::Imgui_windows& imgui_windows, Explorer_context& explorer_context);

    // Implements Imgui_window
    void imgui() override;

private:
    Explorer_context& m_context;
    bool  m_scale_size   {true};
    float m_size         {0.05f};
    bool  m_linear_filter{true};
};

} // namespace explorer
