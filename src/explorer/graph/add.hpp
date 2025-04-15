#pragma once

#include "graph/graph_node.hpp"

namespace explorer {

class Add : public Graph_node
{
public:
    Add();
    void evaluate(Graph&) override;
    void imgui   ()       override;
};

} // namespace explorer
