#ifndef WASMATI_QUERY_BUILDER_H_
#define WASMATI_QUERY_BUILDER_H_

#include "src/graph.h"

#include <set>
using namespace wabt;

namespace wasmati {
class Query {
public:
    typedef std::function<bool(Node*)> NodeCondition;
    typedef std::function<bool(Edge*)> EdgeCondition;
    typedef std::set<Node*> NodeSet;
    typedef std::set<Edge*> EdgeSet;
    /// @brief Condition to returns all edges
    static const EdgeCondition& allEdges;

    /// @brief Returns the childrens of the given nodes.
    /// @param nodes Set of nodes
    /// @param edgeCondition Edge condition to be taken.
    /// @return A set of nodes that are the children.
    static NodeSet children(const NodeSet& nodes,
                            const EdgeCondition& edgeCondition = allEdges);

    /// @brief Given a set of Edges, returns the edges that satisfy the
    /// given condition.
    /// @param edges Set of edges.
    /// @param edgeCondition Edge condition to be present in the result.
    /// @return Set of edges
    static EdgeSet filterEdges(const EdgeSet& edges,
                               const EdgeCondition& edgeCondition = allEdges);

    /// @brief Filter the given nodes and return those that satisfy the given
    /// nodeCondition
    /// @param nodes Set of nodes
    /// @param nodeCondition Node condition.
    /// @return Set of nodes filtered
    static NodeSet filter(const NodeSet& nodes,
                          const NodeCondition& nodeCondition);

    /// @brief Makes a backward BFS alongside the edges that satisfy the
    /// condition edgeCondition, and returns the nodes that satisfy the
    /// condition nodeCondition
    /// @param nodes Set of nodes
    /// @param nodeCondition Node Condition to be present in the result.
    /// @param edgeCondition Edge Condition to be taken.
    /// @param limit The maximum number os elements in the result set.
    /// @return Set of nodes
    static NodeSet backwardsBFS(const NodeSet& nodes,
                                const NodeCondition& nodeCondition,
                                const EdgeCondition& edgeCondition = allEdges,
                                Index limit = UINT32_MAX);

    /// @brief Makes a classic BFS alongside the edges that satisfy the
    /// condition edgeCondition, and returns the nodes that satisfy the
    /// condition nodeCondition
    /// @param nodes Set of nodes
    /// @param nodeCondition Node Condition to be present in the result.
    /// @param edgeCondition Edge Condition to be taken.
    /// @param limit The maximum number os elements in the result set.
    /// @return Set of nodes
    static NodeSet BFS(const NodeSet& nodes,
                       const NodeCondition& nodeCondition,
                       const EdgeCondition& edgeCondition = allEdges,
                       Index limit = UINT32_MAX);
};
}  // namespace wasmati
#endif  // WABT_AST_BUILDER_H_
