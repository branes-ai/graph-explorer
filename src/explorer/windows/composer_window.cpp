#include "windows/composer_window.hpp"

#include "explorer_context.hpp"
#include "explorer_rendering.hpp"
#include "erhe_imgui/imgui_host.hpp"
#include "erhe_imgui/imgui_windows.hpp"

namespace explorer {

Composer_window::Composer_window(erhe::imgui::Imgui_renderer& imgui_renderer, erhe::imgui::Imgui_windows& imgui_windows, Explorer_context& explorer_context)
    : erhe::imgui::Imgui_window{imgui_renderer, imgui_windows, "Composer", "composer"}
    , m_context                {explorer_context}
{
}

void Composer_window::imgui()
{
    m_context.explorer_rendering->imgui();
}

} // namespace explorer