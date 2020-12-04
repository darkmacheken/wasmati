#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void VulnerabilityChecker::UnreachableCode() {
    Index counter = 0;
    NodeStream(Query::functions()).forEach([&](Node* func) {
        debug("[DEBUG][Query::UnreachableCode][%u/%u] Function %s\n", counter++,
              numFuncs, func->name().c_str());

        auto pred = Predicate()
                        .instType(ExprType::Return, false)
                        .instType(ExprType::Block, false)
                        .instType(ExprType::Loop, false)
                        .instType(ExprType::Unreachable, false)
                        .inEdge(EdgeType::CFG, false);
        auto queryInsts = Query::instructions({func}, pred);
        if (!queryInsts.empty()) {
            vulns.emplace_back(VulnType::Unreachable, func->name(), "", "");
        }
    });
}
