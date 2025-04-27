// #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "renderers/cube_instance_buffer.hpp"
#include "explorer_log.hpp"
#include "erhe_profile/profile.hpp"
#include "erhe_verify/verify.hpp"

namespace explorer {

Cube_interface::Cube_interface(erhe::graphics::Instance& graphics_instance)
    : cube_instance_block {graphics_instance, "instance", 3, erhe::graphics::Shader_resource::Type::shader_storage_block}
    , cube_instance_struct{graphics_instance, "Instance"}
    , offsets{
        .position = cube_instance_struct.add_vec4("position")->offset_in_parent()
    }
{
    cube_instance_block.add_struct("instances", &cube_instance_struct, erhe::graphics::Shader_resource::unsized_array);
    cube_instance_block.set_readonly(true);
}

Cube_instance_buffer::Cube_instance_buffer(erhe::graphics::Instance& graphics_instance, Cube_interface& cube_interface)
    : GPU_ring_buffer{
        graphics_instance,
        erhe::renderer::GPU_ring_buffer_create_info{
            .target        = gl::Buffer_target::shader_storage_buffer,
            .binding_point = cube_interface.cube_instance_block.binding_point(),
            .size          = cube_interface.cube_instance_struct.size_bytes() * cube_interface.max_cube_instance_count,
            .debug_label   = "primitive"
        }
    }
    , m_cube_interface{cube_interface}
{
}

auto Cube_instance_buffer::update(std::size_t& out_cube_instance_count) -> erhe::renderer::Buffer_range
{
    ERHE_PROFILE_FUNCTION();

    // SPDLOG_LOGGER_TRACE(
    //     log_primitive_buffer,
    //     "meshes.size() = {}, write_offset = {}",
    //     meshes.size(),
    //     m_writer.write_offset
    // );

    out_cube_instance_count = 0;

    const int xx = 128;
    const int yy = 128;
    const int zz = 128;
    const std::size_t cube_instance_count = xx * yy * zz;
    const auto        entry_size          = m_cube_interface.cube_instance_struct.size_bytes();
    //const auto&       offsets             = m_cube_interface.offsets;
    const std::size_t max_byte_count      = cube_instance_count * entry_size;

    erhe::renderer::Buffer_range buffer_range = open(erhe::renderer::Ring_buffer_usage::CPU_write, max_byte_count);
    std::span<std::byte>         gpu_data     = buffer_range.get_span();
    std::byte* const             start        = gpu_data.data();
    const std::size_t            byte_count   = gpu_data.size_bytes();
    const std::size_t            float_count  = byte_count / sizeof(float);
    const std::span<float>       gpu_float_data {reinterpret_cast<float*>(start), float_count};
    size_t                       write_offset = 0;

    for (int x = 0; x < xx; ++x) {
        for (int y = 0; y < yy; ++y) {
            for (int z = 0; z < xx; ++z) {
                glm::vec3 position{x, y, z};
                gpu_float_data[write_offset++] = static_cast<float>(x);
                gpu_float_data[write_offset++] = static_cast<float>(y);
                gpu_float_data[write_offset++] = static_cast<float>(z);
                gpu_float_data[write_offset++] = 1.0f;
            }
        }
    }

    buffer_range.close(write_offset * sizeof(float));

    // SPDLOG_LOGGER_TRACE(log_primitive_buffer, "wrote {} entries to primitive buffer", primitive_index);
    return buffer_range;
}

} // namespace explorer
