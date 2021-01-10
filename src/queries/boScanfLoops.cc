#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void VulnerabilityChecker::BoScanfLoops() {
    auto start = std::chrono::high_resolution_clock::now();
    std::set<std::string> ignore = config[IGNORE];

    Index counter = 0;
    for (auto func : Query::functions()) {
        debug("[DEBUG][Query::BoScanfLoops][%u/%u] Function %s\n", counter++,
              numFuncs, func->name().c_str());
        if (ignore.count(func->name())) {
            continue;
        }
        // find all loops
        auto loops =
            Query::instructions({func}, Predicate().instType(ExprType::Loop));

        for (Node* loop : loops) {
            auto insts = Query::BFS({loop}, Query::ALL_INSTS, Query::AST_EDGES);
            auto callScanf = Query::filter(
                insts,
                Predicate()
                    .instType(ExprType::Call)
                    .label("$scanf")
                    .TEST(node->getChild(1)->instType() == ExprType::LocalGet));

            if (callScanf.size() == 0) {
                continue;
            }

            // Get the variables dependences on arg 1
            std::set<std::string> varDepend;
            Edge* e = nullptr;
            NodeStream(callScanf)
                .map([](Node* call) { return call->getChild(1); })
                .filter(Predicate()
                            .insert(Predicate()
                                        .outPDGEdge(e, PDGType::Global)
                                        .Or()
                                        .outPDGEdge(e, PDGType::Local))
                            .EXEC(varDepend.insert(e->label())));

            // find br_if
            auto brifs = NodeStream(insts)
                             .filter(Predicate()
                                         .instType(ExprType::BrIf)
                                         .label(loop->label()))
                             .child(0, EdgeType::AST)
                             .filter(Predicate()
                                         .instType(ExprType::Compare)
                                         .opcode(Opcode::I32Eq, false)
                                         .opcode(Opcode::I32Eqz, false))
                             .parents(Query::AST_EDGES)
                             .toNodeSet();

            if (brifs.size() == 0) {
                continue;
            }

            // check if there is a load
            for (Node* brif : brifs) {
                auto loadInsts =
                    NodeStream(brif)
                        .BFS(Query::ALL_NODES, Query::AST_EDGES)
                        .filter(Predicate()
                                    .instType(ExprType::Load)
                                    .inEdge(EdgeType::PDG, varDepend));

                if (loadInsts.size() > 0) {
                    std::stringstream desc;
                    loadInsts.child(0, EdgeType::AST)
                        .filter(Predicate().instType(ExprType::LocalGet));

                    if (loadInsts.empty()) {
                        return;
                    }
                    Index value = 0;
                    auto param1 =
                        NodeStream(brif)
                            .child(0, EdgeType::AST)
                            .filter(Predicate().pdgConstEdgeU32(value));

                    if (param1.empty()) {
                        continue;
                    }
                    desc << "In loop " << loop->label() << ":";
                    desc << " buffer pointed by "
                         << loadInsts.findFirst().get()->label();
                    desc << " reaches $scanf until *"
                         << loadInsts.findFirst().get()->label() << " = "
                         << std::to_string(value);
                    vulns.emplace_back(VulnType::BufferOverflow, func->name(),
                                       "", desc.str());
                }
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
        .count();
    info["boScanfLoops"] = time;
}
