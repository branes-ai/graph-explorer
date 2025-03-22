#pragma once

#include "operations/ioperation.hpp"

#include "erhe_scene/node.hpp"

#include <memory>
#include <vector>

namespace erhe {
    class Hierarchy;
}
namespace erhe::scene {
    class Node;
}

namespace explorer {

class Node_transform_operation : public Operation
{
public:
    class Parameters
    {
    public:
        std::shared_ptr<erhe::scene::Node> node;
        erhe::scene::Transform             parent_from_node_before;
        erhe::scene::Transform             parent_from_node_after;
    };

    explicit Node_transform_operation(const Parameters& parameters);

    // Implements Operation
    auto describe() const -> std::string   override;
    void execute (Explorer_context& context) override;
    void undo    (Explorer_context& context) override;

private:
    Parameters m_parameters;
};

}
