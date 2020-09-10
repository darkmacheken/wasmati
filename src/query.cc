#include "src/query.h"
namespace wasmati {
const Query::EdgeCondition& Query::allEdges = [](Edge*) { return true; };

Query::NodeSet Query::children(const NodeSet& nodes,
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

Query::EdgeSet Query::filterEdges(const EdgeSet& edges,
                                  const EdgeCondition& edgeCondition) {
    EdgeSet result;
    for (auto e : edges) {
        if (edgeCondition(e)) {
            result.insert(e);
        }
    }
    return result;
}

Query::NodeSet Query::filter(const NodeSet& nodes,
                             const NodeCondition& nodeCondition) {
    NodeSet result;
    for (Node* node : nodes) {
        if (nodeCondition(node)) {
            result.insert(node);
        }
    }
    return result;
}

Query::NodeSet Query::backwardsBFS(const NodeSet& nodes,
                                   const NodeCondition& nodeCondition,
                                   const EdgeCondition& edgeCondition,
                                   Index limit) {
    NodeSet result;
    NodeSet nextQuery;
    if (nodes.size() == 0 || limit == 0) {
        return result;
    }

    for (Node* node : nodes) {
        auto inEdges = EdgeSet(node->inEdges().begin(), node->inEdges().end());
        for (Edge* edge : filterEdges(inEdges, edgeCondition)) {
            if (nodeCondition(edge->src())) {
                if (limit == 0) {
                    return result;
                }
                result.insert(edge->src());
                limit--;
            }
            nextQuery.insert(edge->src());
        }
    }
    if (nextQuery.size() == 0) {
        return result;
    }
    auto queryResult = backwardsBFS(nextQuery, nodeCondition, edgeCondition);
    if (queryResult.size() == 0) {
        return result;
    }
    result.insert(queryResult.begin(), queryResult.end());
    return result;
}
Query::NodeSet Query::BFS(const NodeSet& nodes,
                          const NodeCondition& nodeCondition,
                          const EdgeCondition& edgeCondition,
                          Index limit) {
    NodeSet result;
    NodeSet nextQuery;
    if (nodes.size() == 0 || limit == 0) {
        return result;
    }

    for (Node* node : nodes) {
        auto outEdges = EdgeSet(node->outEdges().begin(), node->outEdges().end());
        for (Edge* edge : filterEdges(outEdges, edgeCondition)) {
            if (nodeCondition(edge->dest())) {
                if (limit == 0) {
                    return result;
                }
                result.insert(edge->dest());
                limit--;
            }
            nextQuery.insert(edge->dest());
        }
    }
    if (nextQuery.size() == 0) {
        return result;
    }
    auto queryResult = BFS(nextQuery, nodeCondition, edgeCondition);
    if (queryResult.size() == 0) {
        return result;
    }
    result.insert(queryResult.begin(), queryResult.end());
    return result;
}
}  // namespace wasmati
