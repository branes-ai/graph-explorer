#pragma once

#include "erhe_imgui/imgui_window.hpp"

#include <chrono>
#include <optional>

namespace erhe::imgui { class Imgui_windows; }

namespace explorer {

class Explorer_context;

class Timeline_window : public erhe::imgui::Imgui_window
{
public:
    Timeline_window(
        erhe::imgui::Imgui_renderer& imgui_renderer,
        erhe::imgui::Imgui_windows&  imgui_windows,
        Explorer_context&            explorer_context
    );

    // Implements Imgui_window
    void imgui() override;

    void update();

    void set_timeline_length(float length);
    void set_play_position  (float position);
    void set_play_speed     (float speed);
    [[nodiscard]] auto get_timeline_length() const -> float;
    [[nodiscard]] auto get_play_position  () const -> float;
    [[nodiscard]] auto get_play_speed     () const -> float;

private:
    Explorer_context& m_context;

    bool  m_playing       {false};
    bool  m_looping       {true};
    bool  m_backwards     {false};
    float m_length        {1.0f};
    float m_play_speed_abs{5.0f};
    float m_play_speed    {5.0f};
    float m_play_position {0.0f};
    std::optional<std::chrono::steady_clock::time_point> m_last_time;
};

} // namespace explorer
