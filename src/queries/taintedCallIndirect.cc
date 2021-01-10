#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void VulnerabilityChecker::TaintedCallIndirect() {
    auto start = std::chrono::high_resolution_clock::now();
    std::set<std::string> ignore = config[IGNORE];
    std::set<std::string> sources = config.at(SOURCES);

    if (config.at(IMPORT_AS_SOURCES)) {
        auto funcs = Query::map<std::string>(
            Query::functions([](Node* node) { return node->isImport(); }),
            [](Node* node) { return node->name(); });
        sources.insert(funcs.begin(), funcs.end());
    }

    Index counter = 0;
    for (auto func : Query::functions()) {
        debug("[DEBUG][Query::FormatString][%u/%u] Function %s\n", counter++,
              numFuncs, func->name().c_str());
        if (ignore.count(func->name())) {
            continue;
        }

        auto callIndirects =
            NodeStream(func)
                .instructions(Predicate().instType(ExprType::CallIndirect))
                .toNodeSet();

        for (Node* callIndirect : callIndirects) {
            auto lastArg =
                NodeStream(callIndirect).children(Query::AST_EDGES).findLast();
            if (!lastArg.isPresent()) {
                continue;
            }
            auto pdgEdge = EdgeStream(lastArg.get()->inEdges(EdgeType::PDG))
                               .setUnion(lastArg.get()->outEdges(EdgeType::PDG))
                               .findFirst();
            if (!pdgEdge.isPresent()) {
                continue;
            }

            if (pdgEdge.get()->pdgType() == PDGType::Function &&
                sources.count(pdgEdge.get()->label()) == 1) {
                std::stringstream desc;
                desc << "Source " << pdgEdge.get()->label()
                     << " reaches last argument from call_indirect.";
                vulns.emplace_back(VulnType::Tainted, func->name(),
                                   "call_indirect", desc.str());
            } else if (pdgEdge.get()->pdgType() == PDGType::Local) {
                auto param =
                    NodeStream(func)
                        .parameters(Predicate().name(pdgEdge.get()->label()))
                        .findFirst();
                if (!param.isPresent()) {
                    continue;
                }
                std::set<std::string> visited;
                auto tainted = isTainted(param.get(), visited);
                if (tainted.first != "") {
                    std::stringstream desc;
                    desc << pdgEdge.get()->label() << " tainted from param "
                         << tainted.first << " in " << tainted.second;
                    vulns.emplace_back(VulnType::Tainted, func->name(),
                                       "call_indirect", desc.str());
                }
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    info["taintedCallIndirect"] = time;
}
