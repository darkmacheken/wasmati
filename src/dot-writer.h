#ifndef WASMATI_DOT_H
#define WASMATI_DOT_H
#include "graph.h"

namespace wasmati {

class DotWriter : public GraphWriter {
    std::vector<std::vector<int>> _depth;

public:
    DotWriter(wabt::Stream* stream, Graph* graph, GenerateCPGOptions options)
        : GraphWriter(stream, graph, options) {}
    void writeGraph() override;

    // Edges
    void visitASTEdge(ASTEdge*) override;
    void visitCFGEdge(CFGEdge*) override;
    void visitPDGEdge(PDGEdge*) override;


private:
    void visitSimpleNode(int nodeId, const std::string& nodeName);
    void setSameRank();
    void setDepth(const Node* node, Index depth);
    std::string typeToString(Type type);

    // Inherited via GraphWriter
    virtual void visitModule(Module* node) override;
    virtual void visitFunction(Function* node) override;
    virtual void visitFunctionSignature(FunctionSignature* node) override;
    virtual void visitParameters(Parameters* node) override;
    virtual void visitInstructions(Instructions* node) override;
    virtual void visitLocals(Locals* node) override;
    virtual void visitResults(Results* node) override;
    virtual void visitElse(Else* node) override;
    virtual void visitStart(Start* node) override;
    virtual void visitTrap(Trap* node) override;
    virtual void visitVarNode(VarNode* node) override;
    virtual void visitNopInst(NopInst* node) override;
    virtual void visitUnreachableInst(UnreachableInst* node) override;
    virtual void visitReturnInst(ReturnInst* node) override;
    virtual void visitBrTableInst(BrTableInst* node) override;
    virtual void visitCallIndirectInst(CallIndirectInst* node) override;
    virtual void visitDropInst(DropInst* node) override;
    virtual void visitSelectInst(SelectInst* node) override;
    virtual void visitMemorySizeInst(MemorySizeInst* node) override;
    virtual void visitMemoryGrowInst(MemoryGrowInst* node) override;
    virtual void visitConstInst(ConstInst* node) override;
    virtual void visitBinaryInst(BinaryInst* node) override;
    virtual void visitCompareInst(CompareInst* node) override;
    virtual void visitConvertInst(ConvertInst* node) override;
    virtual void visitUnaryInst(UnaryInst* node) override;
    virtual void visitLoadInst(LoadInst* node) override;
    virtual void visitStoreInst(StoreInst* node) override;
    virtual void visitBrInst(BrInst* node) override;
    virtual void visitBrIfInst(BrIfInst* node) override;
    virtual void visitCallInst(CallInst* node) override;
    virtual void visitGlobalGetInst(GlobalGetInst* node) override;
    virtual void visitGlobalSetInst(GlobalSetInst* node) override;
    virtual void visitLocalGetInst(LocalGetInst* node) override;
    virtual void visitLocalSetInst(LocalSetInst* node) override;
    virtual void visitLocalTeeInst(LocalTeeInst* node) override;
    virtual void visitBeginBlockInst(BeginBlockInst* node) override;
    virtual void visitBlockInst(BlockInst* node) override;
    virtual void visitLoopInst(LoopInst* node) override;
    virtual void visitIfInst(IfInst* node) override;
};

}  // namespace wasmati

#endif /* end of WASMATI_DOT_H */
