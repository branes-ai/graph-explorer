#include "scene/debug_draw.hpp"

#include "explorer_context.hpp"
#include "explorer_log.hpp"

#include "erhe_renderer/line_renderer.hpp"
#include "erhe_renderer/text_renderer.hpp"

#include <glm/glm.hpp>

namespace explorer {

Debug_draw::Debug_draw(Explorer_context& explorer_context)
    : m_context   {explorer_context}
    , m_debug_mode{0}
{
    using IDebug_draw = erhe::physics::IDebug_draw;

    //// TODO restore connection require<Scene_root>();

    m_debug_mode =
        IDebug_draw::c_Draw_wireframe           |
        IDebug_draw::c_Draw_aabb                |
        IDebug_draw::c_Draw_features_text       |
        IDebug_draw::c_Draw_contact_points      |
        //IDebug_draw::c_No_deactivation        |
        //IDebug_draw::c_No_nelp_text           |
        IDebug_draw::c_Draw_text                |
        //IDebug_draw::c_Profile_timings        |
        //IDebug_draw::c_Enable_sat-comparison  |
        //IDebug_draw::c_Enable_ccd             |
        //IDebug_draw::c_Draw_constraints       |
        //IDebug_draw::c_Draw_constraint_limits |
        IDebug_draw::c_Fast_wireframe           |
        IDebug_draw::c_Draw_normals             |
        IDebug_draw::c_Draw_frames;
}

Debug_draw::~Debug_draw()
{
}

auto Debug_draw::get_colors() const -> Colors
{
    return m_colors;
}

void Debug_draw::set_colors(const Colors& colors)
{
    m_colors = colors;
}

void Debug_draw::draw_line(const glm::vec3 from, const glm::vec3 to, const glm::vec3 color)
{
    static_cast<void>(from);
    static_cast<void>(to);
    static_cast<void>(color);
    /// TODO auto& line_renderer = m_context.line_renderer->get(
    /// TODO line_renderer.set_thickness(line_width);
    /// TODO line_renderer.add_lines(glm::vec4{color, 1.0f}, { {from, to} });
}

void Debug_draw::draw_3d_text(const glm::vec3 location, const char* text)
{
    uint32_t text_color = 0xffffffffu; // abgr
    m_context.text_renderer->print(location, text_color, text);
}

void Debug_draw::set_debug_mode(int debug_mode)
{
    m_debug_mode = debug_mode;
}

auto Debug_draw::get_debug_mode() const -> int
{
    return m_debug_mode;
}

void Debug_draw::draw_contact_point(
    const glm::vec3 point,
    const glm::vec3 normal,
    float           distance,
    int             lifeTime,
    const glm::vec3 color
)
{
    static_cast<void>(lifeTime);

    draw_line(point, point + normal * distance, color);
    glm::vec3 ncolor{0};
    draw_line(point, point + (normal * 0.01f), ncolor);
}

void Debug_draw::report_error_warning(const char* warning)
{
    if (warning == nullptr) {
        return;
    }
    log_physics->warn(warning);
}

}
