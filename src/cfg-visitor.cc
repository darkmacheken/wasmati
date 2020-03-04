#include "src/cfg-visitor.h"

namespace wasmati {

Node* CFGvisitor::getLeftMostLeaf(Node* node) {
	if (node->getEdges().size() == 0) {
		return node;
	} else {
		return getLeftMostLeaf(node->getEdges()[0]->getDest());
	}
}

void CFGvisitor::visitASTEdge(ASTEdge* e) {
	e->getDest()->accept(this);
}

void CFGvisitor::visitModule(Module* mod) {
	for (auto node : mod->getEdges()) {
		node->accept(this);
	}
}

void CFGvisitor::visitFunction(Function* func) {
	if (func->getEdges().size() != 3) {
		fprintf(stderr, "Function without 3 children nodes.\n");
		assert(false);
	}
	func->getEdges()[2]->accept(this);
}

void CFGvisitor::visitInstructions(Instructions* insts) {
	Node* leftMost = getLeftMostLeaf(insts);
	if (insts == leftMost) {
		return;
	}
	new CFGEdge(insts, getLeftMostLeaf(insts));
	for (auto edge : insts->getEdges()) {
		edge->accept(this); // will visit just AST nodes
	}
}

void CFGvisitor::visitInstruction(Instruction* inst) {
	_currentInstruction.push(inst);
	visitExpr(inst->getExpr());
	_currentInstruction.pop();
}

void CFGvisitor::visitReturn(Return* inst) {
	if (inst->getNumEdges() == 0) {
		return;
	} else if (inst->getNumEdges() > 1) {
		assert(false);
	}

	Node* child = inst->getEdges()[0]->getDest();
	child->accept(this);
	new CFGEdge(child, inst);
}

void CFGvisitor::OnBinaryExpr(BinaryExpr*) {
	visitArity2();
}

void CFGvisitor::OnBlockExpr(BlockExpr* block) {
	_blocks.emplace_front(block->block.label, _currentInstruction.top());

	int numExprs = _currentInstruction.top()->getNumEdges();
	std::vector<Edge*> edges = _currentInstruction.top()->getEdges();
	int i;
	for (i = 0; i < numExprs - 1; i++) {
		edges[i]->accept(this);
		new CFGEdge(_lastInstruction, getLeftMostLeaf(edges[i + 1]->getDest()));
	}
	edges[i]->accept(this);

	_lastInstruction = _currentInstruction.top();
	_blocks.pop_front();
}
void CFGvisitor::OnBrExpr(BrExpr* expr) {
	std::string target = expr->var.name();
	for (auto p : _blocks) {
		if (target.compare(p.first) == 0) {
			new CFGEdge(_currentInstruction.top(), p.second);
			return;
		}
	}
	assert(false);
}
void CFGvisitor::OnBrIfExpr(BrIfExpr*) {

}

void CFGvisitor::OnBrOnExnExpr(BrOnExnExpr*) {
}
void CFGvisitor::OnBrTableExpr(BrTableExpr* expr) {
	visitArity1();
}
void CFGvisitor::OnCallExpr(CallExpr*)
{
}
void CFGvisitor::OnCallIndirectExpr(CallIndirectExpr*)
{
}
void CFGvisitor::OnCompareExpr(CompareExpr*) {
	visitArity2();
}

void CFGvisitor::OnConstExpr(ConstExpr*) {
	_lastInstruction = _currentInstruction.top();
}
void CFGvisitor::OnConvertExpr(ConvertExpr*) {
	visitArity1();
}
void CFGvisitor::OnDropExpr(DropExpr*)
{
}
void CFGvisitor::OnGlobalGetExpr(GlobalGetExpr*)
{
}
void CFGvisitor::OnGlobalSetExpr(GlobalSetExpr*)
{
}
void CFGvisitor::OnIfExpr(IfExpr*)
{
}
void CFGvisitor::OnLoadExpr(LoadExpr*) {
	visitArity1();
}
void CFGvisitor::OnLocalGetExpr(LocalGetExpr*) {
	_lastInstruction = _currentInstruction.top();
}

void CFGvisitor::OnLocalSetExpr(LocalSetExpr*){
	visitArity1();
}

void CFGvisitor::OnLocalTeeExpr(LocalTeeExpr*) {
	visitArity1();
}

void CFGvisitor::OnLoopExpr(LoopExpr*)
{
}
void CFGvisitor::OnMemoryCopyExpr(MemoryCopyExpr*)
{
}
void CFGvisitor::OnDataDropExpr(DataDropExpr*)
{
}
void CFGvisitor::OnMemoryFillExpr(MemoryFillExpr*)
{
}
void CFGvisitor::OnMemoryGrowExpr(MemoryGrowExpr*) {
	visitArity1();
}

void CFGvisitor::OnMemoryInitExpr(MemoryInitExpr*)
{
}
void CFGvisitor::OnMemorySizeExpr(MemorySizeExpr*)
{
}
void CFGvisitor::OnTableCopyExpr(TableCopyExpr*)
{
}
void CFGvisitor::OnElemDropExpr(ElemDropExpr*)
{
}
void CFGvisitor::OnTableInitExpr(TableInitExpr*)
{
}
void CFGvisitor::OnTableGetExpr(TableGetExpr*) {
	visitArity1();
}

void CFGvisitor::OnTableSetExpr(TableSetExpr*)
{
}
void CFGvisitor::OnTableGrowExpr(TableGrowExpr*)
{
}
void CFGvisitor::OnTableSizeExpr(TableSizeExpr*)
{
}
void CFGvisitor::OnTableFillExpr(TableFillExpr*)
{
}
void CFGvisitor::OnRefFuncExpr(RefFuncExpr*)
{
}
void CFGvisitor::OnRefNullExpr(RefNullExpr*)
{
}
void CFGvisitor::OnRefIsNullExpr(RefIsNullExpr*) {
	visitArity1();
}

void CFGvisitor::OnNopExpr(NopExpr*)
{
}
void CFGvisitor::OnReturnExpr(ReturnExpr*)
{
}
void CFGvisitor::OnReturnCallExpr(ReturnCallExpr*)
{
}
void CFGvisitor::OnReturnCallIndirectExpr(ReturnCallIndirectExpr*)
{
}
void CFGvisitor::OnSelectExpr(SelectExpr*)
{
}
void CFGvisitor::OnStoreExpr(StoreExpr*) {
	visitArity2();
}
void CFGvisitor::OnUnaryExpr(UnaryExpr*) {
	visitArity1();
}

void CFGvisitor::OnUnreachableExpr(UnreachableExpr*)
{
}
void CFGvisitor::OnTryExpr(TryExpr*)
{
}
void CFGvisitor::OnThrowExpr(ThrowExpr*)
{
}
void CFGvisitor::OnRethrowExpr(RethrowExpr*)
{
}
void CFGvisitor::OnAtomicWaitExpr(AtomicWaitExpr*)
{
}
void CFGvisitor::OnAtomicNotifyExpr(AtomicNotifyExpr*)
{
}
void CFGvisitor::OnAtomicLoadExpr(AtomicLoadExpr*) {
	visitArity1();
}

void CFGvisitor::OnAtomicStoreExpr(AtomicStoreExpr*)
{
}
void CFGvisitor::OnAtomicRmwExpr(AtomicRmwExpr*)
{
}
void CFGvisitor::OnAtomicRmwCmpxchgExpr(AtomicRmwCmpxchgExpr*)
{
}
void CFGvisitor::OnTernaryExpr(TernaryExpr*)
{
}
void CFGvisitor::OnSimdLaneOpExpr(SimdLaneOpExpr*)
{
}
void CFGvisitor::OnSimdShuffleOpExpr(SimdShuffleOpExpr*)
{
}
void CFGvisitor::OnLoadSplatExpr(LoadSplatExpr*)
{
}

void CFGvisitor::visitArity1() {
	if (_currentInstruction.top()->getNumEdges() != 1) {
		assert(false);
	}

	Node* child = _currentInstruction.top()->getEdges()[0]->getDest();
	child->accept(this);
	new CFGEdge(_lastInstruction, _currentInstruction.top());
	_lastInstruction = _currentInstruction.top();
}

void CFGvisitor::visitArity2() {
	if (_currentInstruction.top()->getNumEdges() != 2) {
		assert(false);
	}
	Node* left = _currentInstruction.top()->getEdges()[0]->getDest();
	Node* right = _currentInstruction.top()->getEdges()[1]->getDest();

	// visit left
	left->accept(this);
	// add CFG edge from result of left visit to the left most leaf of right children
	new CFGEdge(_lastInstruction, getLeftMostLeaf(right));

	// visit right
	right->accept(this);
	// add CFG edge between right visit and current
	new CFGEdge(_lastInstruction, _currentInstruction.top());

	_lastInstruction = _currentInstruction.top();
}
}