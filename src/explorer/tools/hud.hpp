
#pragma once

#include "tools/tool.hpp"

#include "erhe_commands/command.hpp"

#include <glm/glm.hpp>

namespace erhe::graphics {
    class Instance;
}
namespace erhe::imgui {
    class Imgui_renderer;
}
namespace erhe::rendergraph {
    class Rendergraph;
}
namespace erhe::scene {
    class Node;
};

namespace explorer {

class Explorer_context;
class Explorer_message;
class Explorer_message_bus;
class Explorer_windows;
class Headset_view;
class Icon_set;
class Mesh_memory;
class Rendertarget_imgui_host;
class Rendertarget_mesh;
class Scene_builder;
class Tools;
class Scene_views;

class Hud;

class Toggle_hud_visibility_command : public erhe::commands::Command
{
public:
    Toggle_hud_visibility_command(erhe::commands::Commands& commands, Explorer_context& context);
    auto try_call() -> bool override;

private:
    Explorer_context& m_context;
};

class Hud_drag_command : public erhe::commands::Command
{
public:
    Hud_drag_command(erhe::commands::Commands& commands, Explorer_context& context);
    void try_ready  () override;
    auto try_call   () -> bool override;
    void on_inactive() override;

private:
    Explorer_context& m_context;
};

class Hud
    : public Tool
{
public:
    Hud(
        erhe::commands::Commands&       commands,
        erhe::graphics::Instance&       graphics_instance,
        erhe::imgui::Imgui_renderer&    imgui_renderer,
        erhe::rendergraph::Rendergraph& rendergraph,
        Explorer_context&               explorer_context,
        Explorer_message_bus&           explorer_message_bus,
        Explorer_windows&               explorer_windows,
        Headset_view&                   headset_view,
        Mesh_memory&                    mesh_memory,
        Scene_builder&                  scene_builder,
        Tools&                          tools
    );

    // Implements Tool
    void tool_render(const Render_context& context) override;

    void imgui();

    // Public APi
    [[nodiscard]] auto get_rendertarget_imgui_viewport() const -> std::shared_ptr<Rendertarget_imgui_host>;
    auto toggle_mesh_visibility() -> bool;
    void set_mesh_visibility   (bool value);

    auto try_begin_drag() -> bool;
    void on_drag       ();
    void end_drag      ();

    [[nodiscard]] auto world_from_node() const -> glm::mat4;
    [[nodiscard]] auto node_from_world() const -> glm::mat4;
    [[nodiscard]] auto intersect_ray  (const glm::vec3& ray_origin_in_world, const glm::vec3& ray_direction_in_world) -> std::optional<glm::vec3>;

private:
    void on_message           (Explorer_message& message);
    void update_node_transform(const glm::mat4& world_from_hud);

    Toggle_hud_visibility_command m_toggle_visibility_command;

#if defined(ERHE_XR_LIBRARY_OPENXR)
    Hud_drag_command                          m_drag_command;
    erhe::commands::Redirect_command          m_drag_float_redirect_update_command;
    erhe::commands::Drag_enable_float_command m_drag_float_enable_command;
    erhe::commands::Redirect_command          m_drag_bool_redirect_update_command;
    erhe::commands::Drag_enable_command       m_drag_bool_enable_command;
#endif

    std::weak_ptr<erhe::scene::Node>          m_drag_node;
    bool                                      m_mesh_visible{true};

    std::shared_ptr<erhe::scene::Node>        m_rendertarget_node;
    std::shared_ptr<Rendertarget_mesh>        m_rendertarget_mesh;
    std::shared_ptr<Rendertarget_imgui_host>  m_rendertarget_imgui_viewport;
    glm::mat4                                 m_world_from_hud{1.0f};
    float m_x       {-0.09f};
    float m_y       { 0.0f};
    float m_z       {-0.38f};
    bool  m_enabled {true};

    std::optional<glm::mat4> m_node_from_control;
};

} // namespace explorer
