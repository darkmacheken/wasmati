#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

/// @brief
/// @param param
/// @param visited
/// @return  Returns a pair (param, func_name)
std::pair<std::string, std::string> VulnerabilityChecker::isTainted(
    Node* param,
    std::set<std::string>& visited) {
    Node* func = Query::function(param);
    if (visited.count(func->name()) == 1) {
        return std::make_pair("", "");
    }
    visited.insert(func->name());

    if (config[TAINTED].contains(func->name())) {
        for (Index index : config[TAINTED][func->name()][PARAMS]) {
            if (index == param->index()) {
                return std::make_pair(param->name(), func->name());
            }
        }
    } else if (config[EXPORTED_AS_SINKS] && func->isExport()) {
        std::set<std::string> whitelist = config[WHITELIST];
        if (whitelist.count(func->name()) == 0) {
            return std::make_pair(param->name(), func->name());
        }
    }

    auto args = NodeStream(param)
                    .function()
                    .parents([](Edge* e) {
                        return e->type() == EdgeType::CG &&
                               e->src()->instType() == InstType::Call;
                    })
                    .child(param->index(), EdgeType::AST)
                    .toNodeSet();

    for (auto arg : args) {
        auto localVarsList =
            EdgeStream(arg->outEdges(EdgeType::PDG))
                .setUnion(arg->inEdges(EdgeType::PDG))
                .filterPDG(PDGType::Local)
                .distincLabel()
                .map<std::string>([](Edge* e) { return e->label(); });
        std::set<std::string> localVars(localVarsList.begin(),
                                        localVarsList.end());

        auto newParams = NodeStream(Query::function(arg))
                             .parameters(Predicate().TEST(
                                 localVars.count(node->name()) == 1))
                             .toNodeSet();
        for (Node* param : newParams) {
            auto tainted = isTainted(param, visited);
            if (tainted.first != "") {
                return tainted;
            }
        }
    }
    return std::make_pair("", "");
}
