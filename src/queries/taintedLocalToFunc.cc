#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void VulnerabilityChecker::TaintedLocalToFunc() {
    auto start = std::chrono::high_resolution_clock::now();
    std::set<std::string> sinks = config.at(SINKS);

    if (config.at(IMPORT_AS_SINKS)) {
        auto funcs = Query::map<std::string>(
            Query::functions(Predicate().isImport(true)),
            [](Node* node) { return node->name(); });
        sinks.insert(funcs.begin(), funcs.end());
    }

    std::set<std::string> whitelist = config.at(WHITELIST);
    for (std::string funcName : whitelist) {
        sinks.erase(funcName);
    }

    std::set<std::string> ignore = config[IGNORE];

    Index counter = 0;
    for (auto func : Query::functions()) {
        debug("[DEBUG][Query::TaintedLocalToFunc][%u/%u] Function %s\n",
              counter++, numFuncs, func->name().c_str());
        if (ignore.count(func->name()) == 1) {
            continue;
        }
        if (sinks.count(func->name()) == 1) {
            // if it is already a sink, no use check
            continue;
        }
        std::map<std::string, std::pair<std::string, std::string>>
            taintedParams;
        NodeStream(func).parameters().forEach([&](Node* param) {
            std::set<std::string> visited;
            taintedParams[param->name()] = isTainted(param, visited);
        });

        NodeStream(func)
            .instructions(Predicate()
                              .instType(InstType::Call)
                              .TEST(sinks.count(node->label()) == 1))
            .forEach([&](Node* call) {
                auto localsDependsList =
                    EdgeStream(call->inEdges(EdgeType::PDG))
                        .filterPDG(PDGType::Local)
                        .map<std::string>([](Edge* e) { return e->label(); });

                std::set<std::string> localsDepends(localsDependsList.begin(),
                                                    localsDependsList.end());

                NodeStream(call)
                    .children(Query::AST_EDGES)
                    .forEach([&](Node* arg) {
                        auto argsLocalsDepends =
                            EdgeStream(arg->inEdges(EdgeType::PDG))
                                .filterPDG(PDGType::Local)
                                .map<std::string>(
                                    [](Edge* e) { return e->label(); });

                        localsDepends.insert(argsLocalsDepends.begin(),
                                             argsLocalsDepends.end());
                    });

                for (std::string local : localsDepends) {
                    if (taintedParams.count(local) == 0) {
                        auto param = NodeStream(func)
                                         .parameters(Predicate().name(local))
                                         .findFirst();
                        if (!param.isPresent()) {
                            continue;
                        }
                        std::set<std::string> visited;
                        auto tainted = isTainted(param.get(), visited);
                    }
                    if (taintedParams[local].first == "") {
                        continue;
                    } else {
                        std::stringstream desc;
                        desc << local << " tainted from param "
                             << taintedParams[local].first << " in "
                             << taintedParams[local].second;
                        vulns.emplace_back(VulnType::Tainted, func->name(),
                                           call->label(), desc.str());
                    }
                }
            });
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    info["taintedLocalToFunc"] = time;
}
