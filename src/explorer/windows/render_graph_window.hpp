#pragma once

#include "erhe_imgui/imgui_window.hpp"

#include <memory>

namespace erhe::application
{
    class Rendergraph;
}

namespace explorer
{

class Explorer_scenes;

class Rendergraph_window
    : public erhe::imgui::Imgui_window
{
public:
    static constexpr std::string_view c_title{"Render Graph"};

    Rendergraph_window();

    // Implements Imgui_window
    void imgui() override;

private:
    Explorer_scenes&                m_explorer_scenes;
    erhe::rendergraph::Rendergraph& m_render_graph;
};

} // namespace explorer
