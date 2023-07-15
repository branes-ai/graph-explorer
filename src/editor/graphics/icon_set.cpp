#include "icon_set.hpp"
#include "editor_log.hpp"

#include "renderers/programs.hpp"

#include "erhe/configuration/configuration.hpp"
#include "erhe/imgui/imgui_renderer.hpp"
#include "erhe/scene/light.hpp"

#if defined(ERHE_SVG_LIBRARY_LUNASVG)
#   include <lunasvg.h>
#endif

namespace editor {

Icon_set::Config::Config()
{
    auto ini = erhe::configuration::get_ini("erhe.ini", "icons");
    ini->get("small_icon_size",  small_icon_size);
    ini->get("large_icon_size",  large_icon_size);
    ini->get("hotbar_icon_size", hotbar_icon_size);
}

Icon_set::Icon_set(
    erhe::graphics::Instance&    graphics_instance,
    erhe::imgui::Imgui_renderer& imgui_renderer,
    Programs&                    programs
)
    : config        {}
    , m_row_count   {16}
    , m_column_count{16}
    , m_row         {0}
    , m_column      {0}
    , m_small       {graphics_instance, imgui_renderer, programs, config.small_icon_size,  m_column_count, m_row_count}
    , m_large       {graphics_instance, imgui_renderer, programs, config.large_icon_size,  m_column_count, m_row_count}
    , m_hotbar      {graphics_instance, imgui_renderer, programs, config.hotbar_icon_size, m_column_count, m_row_count}
{

    const auto icon_directory = std::filesystem::path("res") / "icons";

    icons.bone              = load(icon_directory / "bone_data.svg");
    icons.brush_big         = load(icon_directory / "brush_big.svg");
    icons.brush_small       = load(icon_directory / "brush_small.svg");
    icons.camera            = load(icon_directory / "camera.svg");
    icons.directional_light = load(icon_directory / "directional_light.svg");
    icons.drag              = load(icon_directory / "drag.svg");
    icons.file              = load(icon_directory / "file.svg");
    icons.folder            = load(icon_directory / "filebrowser.svg");
    icons.grid              = load(icon_directory / "grid.svg");
    icons.hud               = load(icon_directory / "hud.svg");
    icons.material          = load(icon_directory / "material.svg");
    icons.mesh              = load(icon_directory / "mesh.svg");
    icons.mouse_lmb         = load(icon_directory / "mouse_lmb.svg");
    icons.mouse_lmb_drag    = load(icon_directory / "mouse_lmb_drag.svg");
    icons.mouse_mmb         = load(icon_directory / "mouse_mmb.svg");
    icons.mouse_mmb_drag    = load(icon_directory / "mouse_mmb_drag.svg");
    icons.mouse_move        = load(icon_directory / "mouse_move.svg");
    icons.mouse_rmb         = load(icon_directory / "mouse_rmb.svg");
    icons.mouse_rmb_drag    = load(icon_directory / "mouse_rmb_drag.svg");
    icons.move              = load(icon_directory / "move.svg");
    icons.node              = load(icon_directory / "node.svg");
    icons.physics           = load(icon_directory / "physics.svg");
    icons.point_light       = load(icon_directory / "point_light.svg");
    icons.pull              = load(icon_directory / "pull.svg");
    icons.push              = load(icon_directory / "push.svg");
    icons.raytrace          = load(icon_directory / "curve_path.svg");
    icons.rotate            = load(icon_directory / "rotate.svg");
    icons.scale             = load(icon_directory / "scale.svg");
    icons.scene             = load(icon_directory / "scene.svg");
    icons.select            = load(icon_directory / "select.svg");
    icons.skin              = load(icon_directory / "armature_data.svg");
    icons.space_mouse       = load(icon_directory / "space_mouse.svg");
    icons.space_mouse_lmb   = load(icon_directory / "space_mouse_lmb.svg");
    icons.space_mouse_rmb   = load(icon_directory / "space_mouse_rmb.svg");
    icons.spot_light        = load(icon_directory / "spot_light.svg");
    icons.three_dots        = load(icon_directory / "three_dots.svg");
    icons.vive              = load(icon_directory / "vive.svg");
    icons.vive_menu         = load(icon_directory / "vive_menu.svg");
    icons.vive_trackpad     = load(icon_directory / "vive_trackpad.svg");
    icons.vive_trigger      = load(icon_directory / "vive_trigger.svg");
}

auto Icon_set::load(const std::filesystem::path& path) -> glm::vec2
{
#if defined(ERHE_SVG_LIBRARY_LUNASVG)
    Expects(m_row < m_row_count);

    //const auto  current_path = std::filesystem::current_path();
    const auto document = lunasvg::Document::loadFromFile(path.string());
    if (!document) {
        log_svg->error("Unable to load {}", path.string());
        return glm::vec2{0.0f, 0.0f};
    }

    const float u = static_cast<float>(m_column) / static_cast<float>(m_column_count);
    const float v = static_cast<float>(m_row   ) / static_cast<float>(m_row_count);

    m_small .rasterize(*document.get(), m_column, m_row);
    m_large .rasterize(*document.get(), m_column, m_row);
    m_hotbar.rasterize(*document.get(), m_column, m_row);

    ++m_column;
    if (m_column >= m_column_count) {
        m_column = 0;
        ++m_row;
    }

    return glm::vec2{u, v};
#else
    static_cast<void>(path);
    return glm::vec2{};
#endif
}

auto Icon_set::get_icon(const erhe::scene::Light_type type) const -> const glm::vec2
{
    switch (type) {
        //using enum erhe::scene::Light_type;
        case erhe::scene::Light_type::spot:        return icons.spot_light;
        case erhe::scene::Light_type::directional: return icons.directional_light;
        case erhe::scene::Light_type::point:       return icons.point_light;
        default: return {};
    }
}

[[nodiscard]] auto Icon_set::get_small_rasterization() const -> const Icon_rasterization&
{
    return m_small;
}

[[nodiscard]] auto Icon_set::get_large_rasterization() const -> const Icon_rasterization&
{
    return m_large;
}

[[nodiscard]] auto Icon_set::get_hotbar_rasterization() const -> const Icon_rasterization&
{
    return m_hotbar;
}

} // namespace editor
