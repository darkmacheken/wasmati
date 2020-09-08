#ifndef WASMATI_CFG_BUILDER_H_
#define WASMATI_CFG_BUILDER_H_

#include "src/ast-builder.h"
#include "src/query.h"
#include <list>

using namespace wabt;

namespace wasmati {
struct CFG {
    ModuleContext& mc;
    Graph& graph;
    AST& ast;
    std::list<std::pair<std::string, Node*>> _blocks;

    CFG(ModuleContext& mc, Graph& graph, AST& ast)
        : mc(mc), graph(graph), ast(ast) {}

    ~CFG() {}

    void generateCFG(GenerateCPGOptions& options);

    Node* construct(const Expr& e, Node* lastInst, bool ifCondition);
    Node* construct(const ExprList& es, Node* lastInst, bool ifCondition);
};

}  // namespace wasmati
#endif  // WABT_CFG_BUILDER_H_
