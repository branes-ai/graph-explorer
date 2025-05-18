#pragma once

#include "renderable.hpp"

#include "erhe_imgui/imgui_window.hpp"
#include "erhe_math/math_util.hpp"

#include <vector>

namespace sw::dfa { 
    struct DomainFlowGraph;
    struct DomainFlowNode;
}

namespace erhe::geometry  { class Geometry; }
namespace erhe::graph     { class Node; }
namespace erhe::primitive { class Material; }
namespace erhe::scene     { class Node; }

namespace explorer {

class Explorer_context;
class Explorer_message;
class Explorer_message_bus;
class Explorer_rendering;

class Node_convex_hull_visualization 
    : public Renderable
{
public:
    Node_convex_hull_visualization(
        Explorer_context&     explorer_context,
        Explorer_message_bus& explorer_message_bus,
        Explorer_rendering&   explorer_rendering
    );

    // Implements Renderable
    void render(const Render_context& context) override;

    [[nodiscard]] auto get_material() -> erhe::primitive::Material&;

private:
    void on_message                        (Explorer_message& message);
    void recreate_visualization_scene_graph();
    void update_bounding_box               ();

    [[nodiscard]] auto add_node_convex_hull(
        const sw::dfa::DomainFlowNode& node,
        glm::vec3&                     index_space_offset
    ) -> std::shared_ptr<erhe::scene::Node>;

    Explorer_context&                               m_context;
    std::vector<std::shared_ptr<erhe::graph::Node>> m_visualized_nodes;
    std::shared_ptr<erhe::primitive::Material>      m_material;
    std::shared_ptr<erhe::scene::Node>              m_root;
    float                                           m_gap{4.0f};
    erhe::math::Bounding_box                        m_last_scene_bbox{};
};

} // namespace explorer
