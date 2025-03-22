#include "windows/clipboard_window.hpp"

#include "explorer_context.hpp"
#include "graphics/icon_set.hpp"
#include "tools/clipboard.hpp"

#include "erhe_item/item.hpp"
#include "erhe_imgui/imgui_host.hpp"
#include "erhe_imgui/imgui_windows.hpp"

namespace explorer
{

Clipboard_window::Clipboard_window(
    erhe::imgui::Imgui_renderer& imgui_renderer,
    erhe::imgui::Imgui_windows&  imgui_windows,
    Explorer_context&            explorer_context
)
    : erhe::imgui::Imgui_window{imgui_renderer, imgui_windows, "Clipboard", "clipboard"}
    , m_context                {explorer_context}
{
}

void Clipboard_window::imgui()
{
    const float scale = get_scale_value();

    const auto& items = m_context.clipboard->get_contents();
    for (const auto& item : items) {
        m_context.icon_set->item_icon(item, scale);
        ImGui::TextUnformatted(item->get_name().c_str());
    }
}

}