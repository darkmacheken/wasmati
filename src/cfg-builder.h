#ifndef WASMATI_CFG_BUILDER_H_
#define WASMATI_CFG_BUILDER_H_

#include <list>
#include "ast-builder.h"
#include "src/cast.h"
#include "query.h"

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

    /// @brief Constructs the CFG in the given expression list
    /// @param es Expression list
    bool construct(const ExprList& es);

private:
    void insertEdgeFromLastExpr(const wabt::ExprList& es,
                                wasmati::Node* blockInst);
    Node* getBlock(const std::string& expr);
};

}  // namespace wasmati
#endif  // WABT_CFG_BUILDER_H_
