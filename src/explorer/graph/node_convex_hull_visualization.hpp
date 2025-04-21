#pragma once

#include "erhe_imgui/imgui_window.hpp"

namespace sw::dfa { 
    struct DomainFlowGraph;
    struct DomainFlowNode;
}

namespace erhe::geometry {
    class Geometry;
}
namespace erhe::primitive {
    class Material;
}
namespace erhe::scene {
    class Node;
}

namespace explorer {

class Explorer_context;
class Explorer_message;
class Explorer_message_bus;

class Node_convex_hull_visualization 
    : public erhe::imgui::Imgui_window // Later we can remove window as base class
{
public:
    Node_convex_hull_visualization(
        erhe::imgui::Imgui_renderer& imgui_renderer,
        erhe::imgui::Imgui_windows&  imgui_windows,
        Explorer_context&            explorer_context,
        Explorer_message_bus&        explorer_message_bus
    );

    // Implements Imgui_window
    void imgui() override;

private:
    void on_message(Explorer_message& message);

    void reset_scene_for_node_convex_hull(const sw::dfa::DomainFlowNode& node);

    Explorer_context&                          m_context;
    const sw::dfa::DomainFlowNode*             m_visualized_node{nullptr};
    std::shared_ptr<erhe::primitive::Material> m_material;
    std::shared_ptr<erhe::geometry::Geometry>  m_geometry;
    std::shared_ptr<erhe::scene::Node>         m_root;
};

} // namespace explorer
