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

class Node_attach_operation : public Operation
{
public:
    Node_attach_operation();
    Node_attach_operation(const std::shared_ptr<erhe::scene::Node_attachment>& attachment, const std::shared_ptr<erhe::scene::Node>& host_node);

    // Implements Operation
    auto describe() const -> std::string   override;
    void execute (Explorer_context& context) override;
    void undo    (Explorer_context& context) override;

private:
    std::shared_ptr<erhe::scene::Node_attachment> m_attachment;
    std::shared_ptr<erhe::scene::Node>            m_host_node_before;
    std::shared_ptr<erhe::scene::Node>            m_host_node_after;
};

}
