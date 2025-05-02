#pragma once

#include "renderable.hpp"

#include "erhe_graphics/state/vertex_input_state.hpp"
#include "erhe_imgui/imgui_window.hpp"
#include "erhe_scene_renderer/cube_renderer.hpp"

namespace erhe::graphics       { class Instance; }
namespace erhe::scene_renderer { class Program_interface; }
namespace erhe::imgui          { class Imgui_renderer; }

namespace explorer {

class Explorer_context;
class Explorer_message;
class Explorer_message_bus;
class Explorer_rendering;
class Programs;

class Wavefront_visualization
    : public erhe::imgui::Imgui_window
    , public Renderable
{
public:
    Wavefront_visualization(
        erhe::graphics::Instance&                graphics_instance,
        erhe::scene_renderer::Program_interface& program_interface,
        erhe::imgui::Imgui_renderer&             imgui_renderer,
        erhe::imgui::Imgui_windows&              imgui_windows,
        Explorer_context&                        explorer_context,
        Explorer_message_bus&                    explorer_message_bus,
        Explorer_rendering&                      explorer_rendering,
        Programs&                                programs
    );

    // Implements Renderable
    void render(const Render_context& context) override;

    // Implements Imgui_window
    void imgui() override;

private:
    void on_message(Explorer_message& message);

    bool                                      m_show{ false }; 
    int                                       m_frame_index{0};
    Explorer_context&                         m_context;
    std::unique_ptr<erhe::graphics::Pipeline> m_pipeline;
    erhe::scene_renderer::Cube_renderer       m_cube_renderer;
    erhe::graphics::Vertex_input_state        m_empty_vertex_input;
    std::vector<std::size_t>                  m_frames;
};

} // namespace explorer
