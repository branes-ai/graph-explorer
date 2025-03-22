#pragma once

#include "erhe_imgui/imgui_window.hpp"

namespace erhe::imgui {
    class Imgui_windows;
}
namespace ImNodes::Ez {
    struct Context;
}

namespace explorer {

class Explorer_context;

class Rendergraph_window : public erhe::imgui::Imgui_window
{
public:
    Rendergraph_window(erhe::imgui::Imgui_renderer& imgui_renderer, erhe::imgui::Imgui_windows& imgui_windows, Explorer_context& explorer_context);
    ~Rendergraph_window() noexcept override;

    // Implements Imgui_window
    void imgui() override;
    auto flags() -> ImGuiWindowFlags override;

private:
    Explorer_context&     m_context;
    float                 m_image_size{100.0f};
    float                 m_curve_strength{10.0f};
    ImNodes::Ez::Context* m_imnodes_context{nullptr};
};

} // namespace explorer
