#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

std::pair<bool, Index> VulnerabilityChecker::verifyMallocConst(Node* node) {
    std::set<std::string> mallocs = config.at(MALLOC);
    if (node->instType() == ExprType::Call && mallocs.count(node->label())) {
        auto constEdge =
            EdgeStream(node->inEdges()).filterPDG(PDGType::Const).findFirst();
        if (constEdge.isPresent()) {
            return std::make_pair(true, constEdge.get()->value().u32);
        }
        return std::make_pair(false, 0);
    }
    auto e = EdgeStream(node->outEdges(EdgeType::PDG))
                 .filter([&](Edge* e) {
                     return e->pdgType() == PDGType::Function &&
                            mallocs.count(e->label());
                 })
                 .findFirst();
    if (!e.isPresent()) {
        return std::make_pair(false, 0);
    }
    auto mallocCall =
        NodeStream(node)
            .BFS(Predicate().instType(ExprType::Call).label(e.get()->label()),
                 Query::pdgEdge(e.get()->label(), PDGType::Function), 1, true)
            .findFirst();
    if (!mallocCall.isPresent()) {
        return std::make_pair(false, 0);
    }
    auto constEdge = EdgeStream(mallocCall.get()->inEdges())
                         .filterPDG(PDGType::Const)
                         .findFirst();
    if (constEdge.isPresent()) {
        return std::make_pair(true, constEdge.get()->value().u32);
    }
    return std::make_pair(false, 0);
}
