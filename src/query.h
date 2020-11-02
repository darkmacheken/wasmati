#ifndef WASMATI_QUERY_BUILDER_H_
#define WASMATI_QUERY_BUILDER_H_

#include <set>
#include "graph.h"
#include "include/nlohmann/json.hpp"

using nlohmann::json;

namespace wasmati {
typedef std::function<bool(Node*)> NodeCondition;
typedef std::function<bool(Edge*)> EdgeCondition;

class Query {
    static const Graph* _graph;
    static NodeSet emptyNodeSet;

    static NodeSet& getEmptyNodeSet() {
        emptyNodeSet.clear();
        return emptyNodeSet;
    }

public:
    static void setGraph(const Graph* graph) { _graph = graph; }

public:
    /// @brief Condition to return all edges
    static const EdgeCondition& ALL_EDGES;
    /// @brief Condition to return only AST edges
    static const EdgeCondition& AST_EDGES;
    /// @brief Condition to return only CFG edges
    static const EdgeCondition& CFG_EDGES;
    /// @brief Condition to return only PDG edges
    static const EdgeCondition& PDG_EDGES;
    /// @brief Condition to return only CG edges
    static const EdgeCondition& CG_EDGES;
    /// @brief Condition to return all inst nodes
    static const NodeCondition& ALL_INSTS;
    /// @brief Condition to return all nodes
    static const NodeCondition& ALL_NODES;

    /// @brief Returns the childrens of the given nodes.
    /// @param nodes Set of nodes
    /// @param edgeCondition Edge condition to be taken.
    /// @return A set of nodes that are the children.
    static NodeSet children(const NodeSet& nodes,
                            const EdgeCondition& edgeCondition = ALL_EDGES);

    /// @brief Returns the parents of the given nodes.
    /// @param nodes Set of nodes
    /// @param edgeCondition Edge condition to be taken.
    /// @return A set of nodes that are the parents.
    static NodeSet parents(const NodeSet& nodes,
                           const EdgeCondition& edgeCondition = ALL_EDGES);

    /// @brief Given a set of Edges, returns the edges that satisfies the
    /// given condition.
    /// @param edges Set of edges.
    /// @param edgeCondition Edge condition to be present in the result.
    /// @return Set of edges
    static EdgeSet filterEdges(const EdgeSet& edges,
                               const EdgeCondition& edgeCondition = ALL_EDGES);

    /// @brief Filter the given nodes and return those that satisfies the given
    /// nodeCondition
    /// @param nodes Set of nodes
    /// @param nodeCondition Node condition.
    /// @return Set of nodes filtered
    static NodeSet filter(const NodeSet& nodes,
                          const NodeCondition& nodeCondition);

    /// @brief Returns true if there is a node that satisfies the condition
    /// nodeCondition and false otehrwise
    /// @param nodes Set of nodes
    /// @param nodeCondition Node condition
    /// @return true if the set contains a node that satisfies the given
    /// condition
    static bool contains(const NodeSet& nodes,
                         const NodeCondition& nodeCondition);

    /// @brief Returns true if there is an edge that satisfies the condition
    /// edgeCondition and false otherwise
    /// @param edges Set of edges
    /// @param edgeCondition Edgte condition
    /// @return true if the set contains an edge that satisfies the given
    /// condition
    static bool containsEdge(const EdgeSet& edges,
                             const EdgeCondition& edgeCondition);

    /// @brief Creates a new NodeSet with the results of calling a function for
    /// every node element.
    /// @param nodes Set of nodes
    /// @param func Function to call to each element.
    /// @return A NodeSet with the results of calling a function for every node
    /// element.
    static NodeSet map(const NodeSet& nodes,
                       const std::function<Node*(Node*)> func);

    /// @brief  Creates a new NodeSet with the results of calling a function for
    /// every node element
    /// @param nodes Set of nodes
    /// @param func Function to call to each element.
    /// @return A NodeSet with the results of calling a function for every node
    /// element.
    static NodeSet map(const NodeSet& nodes,
                       const std::function<NodeSet(Node*)> func);

    /// @brief Creates a new NodeSet with the results of calling a function for
    /// every node element.
    /// @tparam T Type of return
    /// @param nodes Set of nodes
    /// @param func Function to call to each element.
    /// @return A set of type T  with the results of calling a function for
    /// every node element.
    template <class T>
    static std::set<T> map(const NodeSet& nodes,
                           const std::function<T(Node*)> func) {
        std::set<T> result;
        for (Node* node : nodes) {
            result.insert(func(node));
        }
        return result;
    }

    /// @brief Makes a classic BFS alongside the edges that satisfies the
    /// condition edgeCondition, and returns the nodes that satisfies the
    /// condition nodeCondition
    /// @param nodes Set of nodes
    /// @param nodeCondition Node Condition to be present in the result.
    /// @param edgeCondition Edge Condition to be taken.
    /// @param limit The maximum number os elements in the result set.
    /// @param reverse If true, performs a backward BFS
    /// @return Set of nodes
    static NodeSet BFS(const NodeSet& nodes,
                       const NodeCondition& nodeCondition = ALL_NODES,
                       const EdgeCondition& edgeCondition = ALL_EDGES,
                       Index limit = UINT32_MAX,
                       bool reverse = false,
                       NodeSet& visited = getEmptyNodeSet());

    /// @brief Makes a classic BFS alongside the edges that satisfies the
    /// condition edgeCondition, and returns the nodes that satisfies the
    /// condition nodeCondition including the begining nodes.
    /// @param nodes Set of nodes
    /// @param nodeCondition Node Condition to be present in the result.
    /// @param edgeCondition Edge Condition to be taken.
    /// @param limit The maximum number os elements in the result set.
    /// @param reverse If true, performs a backward BFS
    /// @return Set of nodes
    static NodeSet BFSincludes(const NodeSet& nodes,
                               const NodeCondition& nodeCondition = ALL_NODES,
                               const EdgeCondition& edgeCondition = ALL_EDGES,
                               Index limit = UINT32_MAX,
                               bool reverse = false);

    /// @brief Returns the module of the graph
    /// @return A set cointaining the node module
    static NodeSet module();

    /// @brief Returns all the function nodes that satisfies teh condition
    /// nodeCondition
    /// @param nodeCondition Node condition to be present in the result.
    /// @return Set of nodes containing function nodes.
    static NodeSet functions(const NodeCondition& nodeCondition = ALL_NODES);

    /// @brief Returns all the instructions of the given functions that
    /// satisfies the nodeCondition
    /// @param nodes A set of function nodes.
    /// @param nodeCondition Condition to filter the nodes
    /// @return A set of instructions from the given functions that satisfies
    /// the condition
    static NodeSet instructions(const NodeSet& nodes,
                                const NodeCondition& nodeCondition = ALL_NODES);

    /// @brief Returns all the parameters nodes of the given functions that
    /// satisfies the nodeCondition
    /// @param nodes A set of function nodes.
    /// @param nodeCondition Condition to filter the nodes.
    /// @return A set of parameters nodes from the given functions that
    /// satisfies the condition.
    static NodeSet parameters(const NodeSet& nodes,
                              const NodeCondition& nodeCondition = ALL_NODES);
};

// This class is to store more complex queries.
struct Queries {
    static NodeSet loopsInsts(std::string& loopName);
};
}  // namespace wasmati
#endif  // WABT_AST_BUILDER_H_
