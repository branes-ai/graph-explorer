#pragma once

#include "renderers/viewport_config.hpp"

#include "erhe_imgui/imgui_window.hpp"

#include <glm/glm.hpp>

namespace erhe::imgui {
    class Imgui_windows;
}

namespace explorer {

class Explorer_context;

class Viewport_config_window : public erhe::imgui::Imgui_window
{
public:
    static constexpr const char* c_visualization_mode_strings[] = {
        "Off",
        "Selected",
        "All"
    };

    Viewport_config_window(
        erhe::imgui::Imgui_renderer& imgui_renderer,
        erhe::imgui::Imgui_windows&  imgui_windows,
        Explorer_context&            explorer_context
    );

    // Implements Imgui_window 
    void imgui() override;

    // Public API
    static void render_style_ui(Render_style_data& render_style);

    void set_edit_data(Viewport_config*);

private:
    Explorer_context& m_context;
    Viewport_config*  m_edit_data{nullptr};
};

} // namespace explorer
