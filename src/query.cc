#include "src/query.h"
namespace wasmati {
const Query::EdgeCondition& Query::allEdges = [](Edge*) { return true; };

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

Query::NodeSet Query::searchBackwards(const NodeSet& nodes,
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
    auto queryResult = searchBackwards(nextQuery, nodeCondition, edgeCondition);
    if (queryResult.size() == 0) {
        return result;
    }
    result.insert(queryResult.begin(), queryResult.end());
    return result;
}
}  // namespace wasmati
