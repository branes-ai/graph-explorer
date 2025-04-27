#if 0
#pragma once

#include "renderers/cube_instance_buffer.hpp"

#include "erhe_renderer/pipeline_renderpass.hpp"
#include "erhe_scene_renderer/camera_buffer.hpp"

namespace erhe::graphics {
    class Instance;
}
namespace erhe::scene {
    class Camera;
}
namespace erhe::scene_renderer {
    class Program_interface;
}

namespace explorer {

class Program_interface;

class Cube_renderer
{
public:
    Cube_renderer(erhe::graphics::Instance& graphics_instance, erhe::scene_renderer::Program_interface& program_interface);

    // Public API
    class Render_parameters
    {
    public:
        const erhe::scene::Camera*           camera           {nullptr};
        const erhe::math::Viewport&          viewport;
        const erhe::graphics::Shader_stages* override_shader_stages{nullptr};
    };

    void render(const Render_parameters& parameters);

private:
    erhe::graphics::Instance& m_graphics_instance;
    Program_interface&        m_program_interface;
    Camera_buffer             m_camera_buffer;
    Cube_instance_buffer      m_cube_instance_buffer;
};

} // namespace explorer
#endif