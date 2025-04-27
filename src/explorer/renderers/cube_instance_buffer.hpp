#pragma once

#include "erhe_graphics/shader_resource.hpp"
#include "erhe_renderer/gpu_ring_buffer.hpp"

namespace erhe::graphics {
    class Instance;
}

namespace explorer {

class Cube_instance_struct
{
public:
    std::size_t position; // vec4 4 * 4 bytes
};

class Cube_interface
{
public:
    explicit Cube_interface(erhe::graphics::Instance& graphics_instance);

    erhe::graphics::Shader_resource cube_instance_block;
    erhe::graphics::Shader_resource cube_instance_struct;
    Cube_instance_struct            offsets;
    std::size_t                     max_cube_instance_count;
};

class Cube_instance_buffer : public erhe::renderer::GPU_ring_buffer
{
public:
    Cube_instance_buffer(erhe::graphics::Instance& graphics_instance, Cube_interface& cube_interface);

    auto update(std::size_t& out_cube_instance_count) -> erhe::renderer::Buffer_range;

private:
    Cube_interface& m_cube_interface;
};

} // namespace explorer
