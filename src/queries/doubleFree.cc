#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void VulnerabilityChecker::DoubleFree() {
    auto start = std::chrono::high_resolution_clock::now();
    std::set<std::string> ignore = config[IGNORE];

    Index counter = 0;
    for (auto func : Query::functions()) {
        debug("[DEBUG][Query::DoubleFree][%u/%u] Function %s\n", counter++,
              numFuncs, func->name().c_str());
        if (ignore.count(func->name())) {
            continue;
        }

        for (auto const& item : config[CONTROL_FLOW]) {
            std::string source = item[SOURCE];
            std::string dest = item[DEST];

            auto sourcePredicate =
                Predicate().instType(InstType::Call).label(source);

            auto callSourceInsts = Query::instructions({func}, sourcePredicate);

            for (Node* callSource : callSourceInsts) {
                auto pdgEdgeCond =
                    Query::pdgEdge(callSource->label(), PDGType::Function);

                Node* destNode = nullptr;
                auto destPredicate =
                    Predicate()
                        .type(NodeType::Instruction)
                        .instType(InstType::Call)
                        .label(dest)
                        .EXEC(destNode = node)
                        .reaches(callSource, destNode, pdgEdgeCond);

                auto destInsts =
                    Query::BFS({callSource}, destPredicate, Query::CFG_EDGES);
                for (Node* callDest : destInsts) {
                    Node* inst = nullptr;
                    auto dfPredicate =
                        Predicate()
                            .type(NodeType::Instruction)
                            .instType(InstType::Call)
                            .label(dest)
                            .inPDGEdge(callSource->label(), PDGType::Function)
                            .EXEC(inst = node)
                            .reaches(callSource, inst, pdgEdgeCond);
                    auto dfInst = NodeStream(callDest)
                                      .BFS(dfPredicate, Query::CFG_EDGES, 1)
                                      .findFirst();

                    if (dfInst.isPresent()) {
                        std::stringstream desc;
                        desc << "Value from call " << callSource->label()
                             << " used after call to " << dest;
                        vulns.emplace_back(VulnType::DoubleFree, func->name(),
                                           dfInst.get()->label(), desc.str());
                    }
                }
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    info["doubleFree"] = time;
}
