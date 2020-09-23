#include "vulns.h"

void wasmati::checkVulnerabilities(Graph* graph) {
    Query::setGraph(graph);
    // checkUnreachableCode();
    // checkBufferOverflow();
    // checkIntegerOverflow();
    // checkUseAfterFree();
    for (auto func : Query::functions(Query::ALL_NODES)) {
        checkBufferSizes(func);
    }
}

void wasmati::checkUnreachableCode() {}

void wasmati::checkBufferOverflow() {}

void wasmati::checkIntegerOverflow() {}

void wasmati::checkUseAfterFree() {}

std::map<int, int> wasmati::checkBufferSizes(Node* func) {
    std::map<int, int> buffers;
    EdgeSet queryEdges;
    Index totalSizeAllocated;
    int sizeAllocated;
    auto allocQuery = Query::instructions({func}, [&](Node* node) {
        if (node->instType() == ExprType::Binary &&
            node->opcode() == Opcode::I32Sub) {
            auto inEdges = node->inEdges(EdgeType::PDG);
            EdgeSet edgeSet = EdgeSet(inEdges.begin(), inEdges.end());
            queryEdges = Query::filterEdges(edgeSet, [](Edge* e) {
                return e->pdgType() == PDGType::Const ||
                       (e->pdgType() == PDGType::Global &&
                        e->label().compare("$g0") == 0);
            });
            return queryEdges.size() == 2;
        }
        return false;
    });

    if (allocQuery.size() != 1 || queryEdges.size() != 2) {
        return buffers;
    }

    for (Edge* e : queryEdges) {
        if (e->pdgType() == PDGType::Const) {
            totalSizeAllocated = e->value().u32;
            sizeAllocated = totalSizeAllocated - 32;
        }
    }
    if (sizeAllocated <= 0) {
        return buffers;
    }

    std::set<int> buffs;
    auto buffQuery = Query::instructions({func}, [&](Node* node) {
        if (node->instType() == ExprType::Binary &&
            node->opcode() == Opcode::I32Add) {
            auto inEdges = node->inEdges(EdgeType::PDG);
            EdgeSet edgeSet = EdgeSet(inEdges.begin(), inEdges.end());
            auto queryEdges = Query::filterEdges(edgeSet, [&](Edge* e) {
                return (e->pdgType() == PDGType::Const &&
                        e->value().u32 >= 32 &&
                        e->value().u32 < totalSizeAllocated) ||
                       (e->pdgType() == PDGType::Global &&
                        e->label().compare("$g0") == 0);
            });
            auto querySub = Query::BFS(
                {node}, [&](Node* n) { return n == *allocQuery.begin(); },
                Query::PDG_EDGES, 1, true);
            return queryEdges.size() == 2 && querySub.size() == 1;
        }
        return false;
    });

    for (Node* node : buffQuery) {
        for (Edge* e : node->inEdges(EdgeType::PDG)) {
            if (e->pdgType() == PDGType::Const) {
                buffs.insert(e->value().u32);
            }
        }
    }
    for (auto it = buffs.begin(); it != buffs.end(); ++it) {
        int size = 0;
        if (std::next(it) != buffs.end()) {
            size = (*std::next(it)) - *it;
        } else {
            size = totalSizeAllocated - *it;
        }
        buffers[*it] = size;
    }
    return buffers;
}
