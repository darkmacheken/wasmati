#ifndef WASMATI_VISITOR_H
#define WASMATI_VISITOR_H

#include "src/common.h"
#include "src/ir.h"
#include "src/ir-util.h"
#include "src/cast.h"
#include "src/stream.h"


using namespace wabt;
namespace wasmati {
class ASTEdge;
class CFGEdge;
class PDGEdge;
class Module;
class Function;
class NamedNode;
class TypeNode;
class SimpleNode;
class Instructions;
class Instruction;
class Return;
class IndexNode;
class Graph;

class GraphVisitor {
public:
	// Edges
	virtual void visitASTEdge(ASTEdge*) = 0;
	virtual void visitCFGEdge(CFGEdge*) = 0;
	virtual void visitPDGEdge(PDGEdge*) = 0;

	// Nodes
	virtual void visitModule(Module*) = 0;
	virtual void visitFunction(Function*) = 0;
	virtual void visitTypeNode(TypeNode*) = 0;
	virtual void visitSimpleNode(SimpleNode*) = 0;
	virtual void visitInstructions(Instructions*) = 0;
	virtual void visitReturn(Return*) = 0;
	virtual void visitInstruction(Instruction*) = 0;
	virtual void visitIndexNode(IndexNode*) = 0;

protected:
	// Expressions
	virtual void visitExpr(Expr*);
	virtual void OnBinaryExpr(BinaryExpr*) = 0;
	virtual void OnBlockExpr(BlockExpr*) = 0;
	virtual void OnBrExpr(BrExpr*) = 0;
	virtual void OnBrIfExpr(BrIfExpr*) = 0;
	virtual void OnBrOnExnExpr(BrOnExnExpr*) = 0;
	virtual void OnBrTableExpr(BrTableExpr*) = 0;
	virtual void OnCallExpr(CallExpr*) = 0;
	virtual void OnCallIndirectExpr(CallIndirectExpr*) = 0;
	virtual void OnCompareExpr(CompareExpr*) = 0;
	virtual void OnConstExpr(ConstExpr*) = 0;
	virtual void OnConvertExpr(ConvertExpr*) = 0;
	virtual void OnDropExpr(DropExpr*) = 0;
	virtual void OnGlobalGetExpr(GlobalGetExpr*) = 0;
	virtual void OnGlobalSetExpr(GlobalSetExpr*) = 0;
	virtual void OnIfExpr(IfExpr*) = 0;
	virtual void OnLoadExpr(LoadExpr*) = 0;
	virtual void OnLocalGetExpr(LocalGetExpr*) = 0;
	virtual void OnLocalSetExpr(LocalSetExpr*) = 0;
	virtual void OnLocalTeeExpr(LocalTeeExpr*) = 0;
	virtual void OnLoopExpr(LoopExpr*) = 0;
	virtual void OnMemoryCopyExpr(MemoryCopyExpr*) = 0;
	virtual void OnDataDropExpr(DataDropExpr*) = 0;
	virtual void OnMemoryFillExpr(MemoryFillExpr*) = 0;
	virtual void OnMemoryGrowExpr(MemoryGrowExpr*) = 0;
	virtual void OnMemoryInitExpr(MemoryInitExpr*) = 0;
	virtual void OnMemorySizeExpr(MemorySizeExpr*) = 0;
	virtual void OnTableCopyExpr(TableCopyExpr*) = 0;
	virtual void OnElemDropExpr(ElemDropExpr*) = 0;
	virtual void OnTableInitExpr(TableInitExpr*) = 0;
	virtual void OnTableGetExpr(TableGetExpr*) = 0;
	virtual void OnTableSetExpr(TableSetExpr*) = 0;
	virtual void OnTableGrowExpr(TableGrowExpr*) = 0;
	virtual void OnTableSizeExpr(TableSizeExpr*) = 0;
	virtual void OnTableFillExpr(TableFillExpr*) = 0;
	virtual void OnRefFuncExpr(RefFuncExpr*) = 0;
	virtual void OnRefNullExpr(RefNullExpr*) = 0;
	virtual void OnRefIsNullExpr(RefIsNullExpr*) = 0;
	virtual void OnNopExpr(NopExpr*) = 0;
	virtual void OnReturnExpr(ReturnExpr*) = 0;
	virtual void OnReturnCallExpr(ReturnCallExpr*) = 0;
	virtual void OnReturnCallIndirectExpr(ReturnCallIndirectExpr*) = 0;
	virtual void OnSelectExpr(SelectExpr*) = 0;
	virtual void OnStoreExpr(StoreExpr*) = 0;
	virtual void OnUnaryExpr(UnaryExpr*) = 0;
	virtual void OnUnreachableExpr(UnreachableExpr*) = 0;
	virtual void OnTryExpr(TryExpr*) = 0;
	virtual void OnThrowExpr(ThrowExpr*) = 0;
	virtual void OnRethrowExpr(RethrowExpr*) = 0;
	virtual void OnAtomicWaitExpr(AtomicWaitExpr*) = 0;
	virtual void OnAtomicNotifyExpr(AtomicNotifyExpr*) = 0;
	virtual void OnAtomicLoadExpr(AtomicLoadExpr*) = 0;
	virtual void OnAtomicStoreExpr(AtomicStoreExpr*) = 0;
	virtual void OnAtomicRmwExpr(AtomicRmwExpr*) = 0;
	virtual void OnAtomicRmwCmpxchgExpr(AtomicRmwCmpxchgExpr*) = 0;
	virtual void OnTernaryExpr(TernaryExpr*) = 0;
	virtual void OnSimdLaneOpExpr(SimdLaneOpExpr*) = 0;
	virtual void OnSimdShuffleOpExpr(SimdShuffleOpExpr*) = 0;
	virtual void OnLoadSplatExpr(LoadSplatExpr*) = 0;
};

class GraphWriter : public GraphVisitor {
protected:
	wabt::Stream* _stream;
	Graph* _graph;

	void writePuts(const char*);
	void writeString(const std::string&);
	void writePutsln(const char*);
	void writeStringln(const std::string&);
public:
	GraphWriter(wabt::Stream* stream, Graph* graph) : _stream(stream), _graph(graph) {}

	virtual void writeGraph() = 0;
};

} // end of wasmati namespace
#endif /* WASMATI_VISITOR_H*/