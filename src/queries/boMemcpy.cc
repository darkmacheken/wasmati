#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void VulnerabilityChecker::BOMemcpy() {
    std::set<std::string> memcpyFuncs = config.at(BO_MEMCPY);
    std::set<std::string> ignore = config[IGNORE];

    for (auto func : Query::functions()) {
        if (ignore.count(func->name()) == 1) {
            continue;
        }

        if (memcpyFuncs.count(func->name()) == 1) {
            // if it is already a sink, no use check
            continue;
        }

        NodeStream(func)
            .instructions(Predicate()
                              .instType(ExprType::Call)
                              .TEST(memcpyFuncs.count(node->label()) == 1))
            .forEach([&](Node* call) {
                auto dest = NodeStream(call)
                                .child(0)
                                .filter(Predicate()
                                            .inPDGEdge("$g0", PDGType::Global)
                                            .Or()
                                            .outPDGEdge(PDGType::Const))
                                .findFirst();

                if (!dest.isPresent()) {
                    return;
                }

                auto src = call->getChild(1);
                auto parameters = Query::parameters({func});
                auto localVarDeps =
                    EdgeStream(src->outEdges(EdgeType::PDG))
                        .setUnion(src->inEdges(EdgeType::PDG))
                        .filterPDG(PDGType::Local)
                        .filter([&](Edge* e) {
                            return NodeStream(func)
                                .parameters(Predicate().name(e->label()))
                                .findFirst()
                                .isPresent();
                        })
                        .map<Node*>([&](Edge* e) {
                            return NodeStream(func)
                                .parameters(Predicate().name(e->label()))
                                .findFirst()
                                .get();
                        });

                for (auto const localVar : localVarDeps) {
                    std::set<std::string> visited;
                    auto tainted = isTainted(localVar, visited);
                    if (tainted.first == "") {
                        continue;
                    }
                    std::stringstream desc;
                    desc << localVar->name() << " tainted from param "
                         << tainted.first << " in " << tainted.second;
                    vulns.emplace_back(VulnType::BufferOverflow, func->name(),
                                       call->label(), desc.str());
                    break;
                }
            });
    }
}
