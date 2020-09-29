#include "query.h"
#include <iostream>
namespace wasmati {
const Graph* Query::_graph = nullptr;
const EdgeCondition& Query::ALL_EDGES = [](Edge*) { return true; };
const EdgeCondition& Query::AST_EDGES = [](Edge* e) {
    return e->type() == EdgeType::AST;
};
const EdgeCondition& Query::CFG_EDGES = [](Edge* e) {
    return e->type() == EdgeType::CFG;
};
const EdgeCondition& Query::PDG_EDGES = [](Edge* e) {
    return e->type() == EdgeType::PDG;
};
const NodeCondition& Query::ALL_NODES = [](Node* node) { return true; };

NodeSet Query::children(const NodeSet& nodes,
                        const EdgeCondition& edgeCondition) {
    NodeSet result;
    for (Node* node : nodes) {
        EdgeSet edges = filterEdges(
            EdgeSet(node->outEdges().begin(), node->outEdges().end()),
            edgeCondition);
        for (Edge* e : edges) {
            result.insert(e->dest());
        }
    }
    return result;
}

NodeSet Query::parents(const NodeSet& nodes,
                       const EdgeCondition& edgeCondition) {
    NodeSet result;
    for (Node* node : nodes) {
        EdgeSet edges =
            filterEdges(EdgeSet(node->inEdges().begin(), node->inEdges().end()),
                        edgeCondition);
        for (Edge* e : edges) {
            result.insert(e->src());
        }
    }
    return result;
}

EdgeSet Query::filterEdges(const EdgeSet& edges,
                           const EdgeCondition& edgeCondition) {
    EdgeSet result;
    for (auto e : edges) {
        if (edgeCondition(e)) {
            result.insert(e);
        }
    }
    return result;
}

NodeSet Query::filter(const NodeSet& nodes,
                      const NodeCondition& nodeCondition) {
    NodeSet result;
    for (Node* node : nodes) {
        if (nodeCondition(node)) {
            result.insert(node);
        }
    }
    return result;
}

bool Query::contains(const NodeSet& nodes, const NodeCondition& nodeCondition) {
    for (Node* node : nodes) {
        if (nodeCondition(node)) {
            return true;
        }
    }
    return false;
}

bool Query::containsEdge(const EdgeSet& edges,
                         const EdgeCondition& edgeCondition) {
    for (Edge* e : edges) {
        if (edgeCondition(e)) {
            return true;
        }
    }
    return false;
}

NodeSet Query::BFS(const NodeSet& nodes,
                   const NodeCondition& nodeCondition,
                   const EdgeCondition& edgeCondition,
                   Index limit,
                   bool reverse) {
    NodeSet result;
    NodeSet nextQuery;
    if (nodes.size() == 0 || limit == 0) {
        return result;
    }

    for (Node* node : nodes) {
        auto edges =
            reverse ? EdgeSet(node->inEdges().begin(), node->inEdges().end())
                    : EdgeSet(node->outEdges().begin(), node->outEdges().end());
        for (Edge* edge : filterEdges(edges, edgeCondition)) {
            auto node = reverse ? edge->src() : edge->dest();
            if (nodeCondition(node)) {
                if (limit == 0) {
                    return result;
                }
                result.insert(node);
                limit--;
            }
            nextQuery.insert(node);
        }
    }
    if (nextQuery.size() == 0) {
        return result;
    }
    auto queryResult =
        BFS(nextQuery, nodeCondition, edgeCondition, limit, reverse);
    if (queryResult.size() == 0) {
        return result;
    }
    result.insert(queryResult.begin(), queryResult.end());
    return result;
}

NodeSet Query::BFSincludes(const NodeSet& nodes,
                           const NodeCondition& nodeCondition,
                           const EdgeCondition& edgeCondition,
                           Index limit,
                           bool reverse) {
    NodeSet result;
    NodeSet nextQuery;
    if (nodes.size() == 0 || limit == 0) {
        return result;
    }

    for (Node* node : nodes) {
        if (nodeCondition(node)) {
            if (limit == 0) {
                return result;
            }
            result.insert(node);
            limit--;
        }
        auto edges =
            reverse ? EdgeSet(node->inEdges().begin(), node->inEdges().end())
                    : EdgeSet(node->outEdges().begin(), node->outEdges().end());
        for (Edge* edge : filterEdges(edges, edgeCondition)) {
            auto node = reverse ? edge->src() : edge->dest();
            nextQuery.insert(node);
        }
    }
    if (nextQuery.size() == 0) {
        return result;
    }
    auto queryResult =
        BFS(nextQuery, nodeCondition, edgeCondition, limit, reverse);
    if (queryResult.size() == 0) {
        return result;
    }
    result.insert(queryResult.begin(), queryResult.end());
    return result;
}

NodeSet Query::module() {
    assert(_graph != nullptr);
    return {_graph->getModule()};
}

NodeSet Query::functions(const NodeCondition& nodeCondition) {
    return filter(children(module(), AST_EDGES), nodeCondition);
}
NodeSet Query::instructions(const NodeSet& nodes,
                            const NodeCondition& nodeCondition) {
    NodeSet funcInstsNode;
    for (Node* node : nodes) {
        assert(node->type() == NodeType::Function);
        if (node->isImport()) {
            continue;
        }
        funcInstsNode.insert(node->getChild(1, EdgeType::AST));
    }

    return BFS(
        funcInstsNode,
        [&](Node* node) {
            return node->type() == NodeType::Instruction && nodeCondition(node);
        },
        AST_EDGES);
}
}  // namespace wasmati
