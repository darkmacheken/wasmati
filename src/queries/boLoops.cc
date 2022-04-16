#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void VulnerabilityChecker::BoLoops() {
    std::set<std::string> ignore = config[IGNORE];

    Index counter = 0;
    for (auto func : Query::functions()) {
        debug("[DEBUG][Query::BoLoops][%u/%u] Function %s\n", counter++,
              numFuncs, func->name().c_str());
        if (ignore.count(func->name())) {
            continue;
        }
        // find all loops
        auto loops =
            Query::instructions({func}, Predicate().instType(InstType::Loop));

        for (Node* loop : loops) {
            auto insts = Query::BFS({loop}, Query::ALL_INSTS, Query::AST_EDGES);

            std::set<std::string> vars;
            auto stores =
                NodeStream(insts)
                    .filter(Predicate().instType(InstType::Store))
                    .child(0)
                    .filter(Predicate()
                                .instType(InstType::Binary)
                                .opcode(Opcode::I32Add))
                    .children(Query::AST_EDGES)
                    .filter(Predicate()
                                .instType(InstType::LocalGet)
                                .Or()
                                .instType(InstType::LocalTee))
                    .forEach([&](Node* node) { vars.insert(node->label()); })
                    .toNodeSet();

            for (auto var : vars) {
                auto addChildren = NodeStream(insts)
                                       .filter(Predicate()
                                                   .instType(InstType::Binary)
                                                   .opcode(Opcode::I32Add))
                                       .children();
                bool containsGet =
                    addChildren.contains(Predicate()
                                             .instType(InstType::LocalGet)
                                             .label(var)
                                             .Or()
                                             .instType(InstType::LocalTee)
                                             .label(var));
                bool containsConst =
                    addChildren.contains(Predicate().instType(InstType::Const));

                if (!(containsGet && containsConst)) {
                    continue;
                }

                auto brIfsNotEq =
                    NodeStream(insts)
                        .filter(Predicate().instType(InstType::BrIf))
                        .child(0)
                        .filter(Predicate().instType(InstType::Compare))
                        .children()
                        .filter(Predicate()
                                    .instType(InstType::LocalGet)
                                    .label(var)
                                    .Or()
                                    .instType(InstType::LocalTee)
                                    .label(var))
                        .toNodeSet();

                if (brIfsNotEq.empty()) {
                    std::stringstream desc;
                    desc << "In loop " << loop->label() << ":";
                    desc << " a buffer is assigned without bound check.";
                    vulns.emplace_back(VulnType::BufferOverflow, func->name(),
                                       "", desc.str());
                    break;
                }
            }
        }
    }
}
