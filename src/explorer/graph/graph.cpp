#include "graph/graph.hpp"
#include "graph/graph_node.hpp"

namespace explorer {

void Graph::evaluate()
{
    sort();
    for (erhe::graph::Node* node : m_nodes) {
        Graph_node* graph_node = dynamic_cast<Graph_node*>(node);
        graph_node->evaluate(*this);
    }
}

} // namespace explorer
