#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void VulnerabilityChecker::TaintedFuncToFunc() {
    std::set<std::string> sources = config.at(SOURCES);
    std::set<std::string> sinks = config.at(SINKS);

    if (config.at(IMPORT_AS_SOURCES)) {
        auto funcs = Query::map<std::string>(
            Query::functions([](Node* node) { return node->isImport(); }),
            [](Node* node) { return node->name(); });
        sources.insert(funcs.begin(), funcs.end());
    }

    if (config.at(IMPORT_AS_SINKS)) {
        auto funcs = Query::map<std::string>(
            Query::functions([](Node* node) { return node->isImport(); }),
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
        debug("[DEBUG][Query::TaintedFuncToFunc][%u/%u] Function %s\n",
            counter++, numFuncs, func->name().c_str());
        if (ignore.count(func->name())) {
            continue;
        }
        if (sinks.count(func->name()) == 1) {
            // if it is already a sink, no use check
            continue;
        }
        auto query = Query::instructions({ func }, [&](Node* node) {
            if (node->instType() == ExprType::Call &&
                sinks.count(node->label()) == 1) {
                auto pdgEdges = node->inEdges(EdgeType::PDG);
                return Query::containsEdge(pdgEdges, [&](Edge* e) {
                    if (e->pdgType() == PDGType::Function &&
                        sources.count(e->label()) == 1) {
                        std::stringstream desc;
                        desc << "Source " << e->label() << " reaches sink "
                            << node->label();
                        vulns.emplace_back(VulnType::Tainted, func->name(),
                            node->label(), desc.str());
                        return true;
                    }
                    return false;
                    });
            }
            return false;
            });
    }
}