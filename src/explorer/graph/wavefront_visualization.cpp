#include "graph/wavefront_visualization.hpp"
#include "explorer_log.hpp"
#include "explorer_context.hpp"
#include "explorer_message_bus.hpp"
#include "explorer_rendering.hpp"
#include "renderers/render_context.hpp"
#include "renderers/programs.hpp"
#include "windows/property_editor.hpp"

#include "erhe_imgui/imgui_windows.hpp"
#include "erhe_imgui/imgui_renderer.hpp"
#include "erhe_scene_renderer/cube_instance_buffer.hpp"

namespace explorer {

Wavefront_visualization::Wavefront_visualization(
    erhe::graphics::Instance&                graphics_instance,
    erhe::scene_renderer::Program_interface& program_interface,
    erhe::imgui::Imgui_renderer&             imgui_renderer,
    erhe::imgui::Imgui_windows&              imgui_windows,
    Explorer_context&                        explorer_context,
    Explorer_message_bus&                    explorer_message_bus,
    Explorer_rendering&                      explorer_rendering,
    Programs&                                programs
)
    : Imgui_window   {imgui_renderer, imgui_windows, "Wavefront Visualization", "waverfront_visualization"}
    , m_context      {explorer_context}
    , m_cube_renderer{graphics_instance, program_interface}
{
    explorer_message_bus.add_receiver(
        [&](Explorer_message& message) {
            on_message(message);
        }
    );
    explorer_rendering.add(this);

    m_pipeline = std::make_unique<erhe::graphics::Pipeline>(
        erhe::graphics::Pipeline_data{
            .name           = "cubes",
            .shader_stages  = &programs.cubes.shader_stages,
            .vertex_input   = &m_empty_vertex_input,
            .input_assembly = erhe::graphics::Input_assembly_state::triangles,
            .rasterization  = erhe::graphics::Rasterization_state::cull_mode_back_ccw,
            .depth_stencil  = erhe::graphics::Depth_stencil_state::depth_test_enabled_stencil_test_disabled(true),
            .color_blend    = erhe::graphics::Color_blend_state::color_blend_disabled
        }
    );

    erhe::scene_renderer::Cube_instance_buffer& cube_instance_buffer = m_cube_renderer.get_buffer();

    // TODO - Initialize from graph
    int edge = 128;
    float scale = static_cast<float>(edge - 1);
    float size = 0.75f;
    for (int x = 0; x < edge; ++x) {
        const float r_x = static_cast<float>(x) / scale;
        std::vector<erhe::scene_renderer::Cube_instance> cubes;
        cubes.reserve(edge * edge);
        for (int y = 0; y < edge; ++y) {
            const float r_y = static_cast<float>(y) / scale;
            for (int z = 0; z < edge; ++z) {
                const float r_z = static_cast<float>(z) / scale;
                cubes.emplace_back(
                    glm::vec4{ static_cast<float>(x * 2 - 4), static_cast<float>(y * 2 - 4), static_cast<float>(z * 2 - 4), size },
                    glm::vec4{ r_x, r_y, r_z, 1.0f }
                );
            }
        }
        size_t frame = cube_instance_buffer.append_frame(cubes);
        m_frames.push_back(frame);
   }
}

void Wavefront_visualization::on_message(Explorer_message&)
{
}

void Wavefront_visualization::imgui()
{
    if (m_frames.empty()) {
        return;
    }
    Property_editor property_editor;
    property_editor.add_entry(
        "Show", 
        [this]() {
            ImGui::Checkbox("##show", &m_show);
        }
    );
    property_editor.add_entry(
        "Frame", 
        [this]() {
            ImGui::SliderInt("##frame",
                &m_frame_index,
                0,
                static_cast<int>(m_frames.size()) - 1,
                "%d"
            );
        }
    );
    property_editor.show_entries();

}

void Wavefront_visualization::render(const Render_context& context)
{
    if (!m_show) {
        return;
    }

    erhe::scene_renderer::Cube_renderer::Render_parameters parameters{
        .frame    = m_frames[m_frame_index],
        .pipeline = *m_pipeline.get(),
        .camera   = context.camera,
        .viewport = context.viewport
    };
    m_cube_renderer.render(parameters);
}

} // namespace explorer
