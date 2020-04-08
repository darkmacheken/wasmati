#ifndef WASMATI_DOT_H
#define WASMATI_DOT_H
#include "src/graph.h"

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

    // Nodes
    void visitModule(Module*) override;
    void visitFunction(Function*) override;
    void visitTypeNode(TypeNode*) override;
    void visitSimpleNode(SimpleNode*) override;
    void visitInstructions(Instructions*) override;
    void visitInstruction(Instruction*) override;
    void visitReturn(Return*) override;
    void visitElse(Else*) override;
    void visitStart(Start*) override;
    void visitTrap(Trap*) override;
    void visitIndexNode(IndexNode*) override;

private:
    // Expressions
    void OnBinaryExpr(BinaryExpr*) override;
    void OnBlockExpr(BlockExpr*) override;
    void OnBrExpr(BrExpr*) override;
    void OnBrIfExpr(BrIfExpr*) override;
    void OnBrOnExnExpr(BrOnExnExpr*) override;
    void OnBrTableExpr(BrTableExpr*) override;
    void OnCallExpr(CallExpr*) override;
    void OnCallIndirectExpr(CallIndirectExpr*) override;
    void OnCompareExpr(CompareExpr*) override;
    void OnConstExpr(ConstExpr*) override;
    void OnConvertExpr(ConvertExpr*) override;
    void OnDropExpr(DropExpr*) override;
    void OnGlobalGetExpr(GlobalGetExpr*) override;
    void OnGlobalSetExpr(GlobalSetExpr*) override;
    void OnIfExpr(IfExpr*) override;
    void OnLoadExpr(LoadExpr*) override;
    void OnLocalGetExpr(LocalGetExpr*) override;
    void OnLocalSetExpr(LocalSetExpr*) override;
    void OnLocalTeeExpr(LocalTeeExpr*) override;
    void OnLoopExpr(LoopExpr*) override;
    void OnMemoryCopyExpr(MemoryCopyExpr*) override;
    void OnDataDropExpr(DataDropExpr*) override;
    void OnMemoryFillExpr(MemoryFillExpr*) override;
    void OnMemoryGrowExpr(MemoryGrowExpr*) override;
    void OnMemoryInitExpr(MemoryInitExpr*) override;
    void OnMemorySizeExpr(MemorySizeExpr*) override;
    void OnTableCopyExpr(TableCopyExpr*) override;
    void OnElemDropExpr(ElemDropExpr*) override;
    void OnTableInitExpr(TableInitExpr*) override;
    void OnTableGetExpr(TableGetExpr*) override;
    void OnTableSetExpr(TableSetExpr*) override;
    void OnTableGrowExpr(TableGrowExpr*) override;
    void OnTableSizeExpr(TableSizeExpr*) override;
    void OnTableFillExpr(TableFillExpr*) override;
    void OnRefFuncExpr(RefFuncExpr*) override;
    void OnRefNullExpr(RefNullExpr*) override;
    void OnRefIsNullExpr(RefIsNullExpr*) override;
    void OnNopExpr(NopExpr*) override;
    void OnReturnExpr(ReturnExpr*) override;
    void OnReturnCallExpr(ReturnCallExpr*) override;
    void OnReturnCallIndirectExpr(ReturnCallIndirectExpr*) override;
    void OnSelectExpr(SelectExpr*) override;
    void OnStoreExpr(StoreExpr*) override;
    void OnUnaryExpr(UnaryExpr*) override;
    void OnUnreachableExpr(UnreachableExpr*) override;
    void OnTryExpr(TryExpr*) override;
    void OnThrowExpr(ThrowExpr*) override;
    void OnRethrowExpr(RethrowExpr*) override;
    void OnAtomicWaitExpr(AtomicWaitExpr*) override;
    void OnAtomicNotifyExpr(AtomicNotifyExpr*) override;
    void OnAtomicLoadExpr(AtomicLoadExpr*) override;
    void OnAtomicStoreExpr(AtomicStoreExpr*) override;
    void OnAtomicRmwExpr(AtomicRmwExpr*) override;
    void OnAtomicRmwCmpxchgExpr(AtomicRmwCmpxchgExpr*) override;
    void OnTernaryExpr(TernaryExpr*) override;
    void OnSimdLaneOpExpr(SimdLaneOpExpr*) override;
    void OnSimdShuffleOpExpr(SimdShuffleOpExpr*) override;
    void OnLoadSplatExpr(LoadSplatExpr*) override;

    std::string writeConst(const Const& const_);
    void setSameRank();
    void setDepth(const Node* node, Index depth);
};

}  // namespace wasmati

#endif /* end of WASMATI_DOT_H */
