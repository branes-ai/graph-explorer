#pragma once

#include "graph/graph_node.hpp"

namespace explorer {

class Constant : public Graph_node
{
public:
    Constant();

    void evaluate(Graph&) override;
    void imgui   () override;

    Payload m_payload;
};

} // namespace explorer
