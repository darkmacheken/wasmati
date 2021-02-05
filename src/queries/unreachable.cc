#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void VulnerabilityChecker::UnreachableCode() {
    Index counter = 0;
    NodeStream(Query::functions()).forEach([&](Node* func) {
        debug("[DEBUG][Query::UnreachableCode][%u/%u] Function %s\n", counter++,
              numFuncs, func->name().c_str());

        auto pred = Predicate()
                        .instType(InstType::Return, false)
                        .instType(InstType::Block, false)
                        .instType(InstType::Loop, false)
                        .instType(InstType::Unreachable, false)
                        .inEdge(EdgeType::CFG, false);
        auto queryInsts = Query::instructions({func}, pred);
        if (!queryInsts.empty()) {
            vulns.emplace_back(VulnType::Unreachable, func->name(), "", "");
        }
    });
}
