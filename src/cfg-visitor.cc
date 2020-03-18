#include "src/cfg-visitor.h"

namespace wasmati {

Node* CFGvisitor::getLeftMostLeaf(Node* node) {
	if (node->getNumEdges() == 0) {
		return node;
	} else {
		return getLeftMostLeaf(node->getEdge(0)->getDest());
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
	visitSequential(insts);

	new CFGEdge(insts, leftMost);
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
	new CFGEdge(_lastInstruction, inst);
}

void CFGvisitor::visitElse(Else* node) {
	visitSequential(node);

	if (_lastInstruction == nullptr) {
		// unreachable
		return;
	}

	// Last instruction of the Else
	if (_lastInstruction->getType() == ExprType::BrIf) {
		new CFGEdge(_lastInstruction, _blocks.front().second, "false");
	} else if (_lastInstruction->getType() == ExprType::Br) {
		// Do nothing
	} else if (_lastInstruction->getType() == ExprType::BrTable) {
		// Do nothing
	} else if (_lastInstruction->getType() == ExprType::If) {
		auto unreach = _unreachableInsts.find(_lastInstruction->getEdge(1)->getDest());
		if (unreach != _unreachableInsts.end()) {
			// unreachable
			_unreachableInsts.clear();
			return;
		}
		new CFGEdge(_lastInstruction->getEdge(1)->getDest(), _blocks.front().second);
		if (_lastInstruction->getNumEdges() == 2) {
			new CFGEdge(_lastInstruction->getEdge(0)->getDest(), _blocks.front().second, "false");
		} else {
			assert(false);
		}
	} else {
		new CFGEdge(_lastInstruction, _blocks.front().second);
	}
}

void CFGvisitor::OnBinaryExpr(BinaryExpr*) {
	visitArity2();
}

void CFGvisitor::OnBlockExpr(BlockExpr* block) {
	_blocks.emplace_front(block->block.label, _currentInstruction.top());

	visitSequential(_currentInstruction.top());

	if (_lastInstruction == nullptr) {
		// unreachable
		_blocks.pop_front();
		return;
	}

	// Last instruction of the block
	if (_lastInstruction->getType() == ExprType::BrIf) {
		new CFGEdge(_lastInstruction, _currentInstruction.top(), "false");
	} else if (_lastInstruction->getType() == ExprType::Br) {
		// Do nothing
	} else if (_lastInstruction->getType() == ExprType::BrTable) {
		// Do nothing
	} else if (_lastInstruction->getType() == ExprType::If) {
		auto unreach = _unreachableInsts.find(_lastInstruction->getEdge(1)->getDest());
		if (unreach != _unreachableInsts.end()) {
			// unreachable
			_unreachableInsts.clear();
			_blocks.pop_front();
			_lastInstruction = nullptr;
			return;
		}
		new CFGEdge(_lastInstruction->getEdge(1)->getDest(), _currentInstruction.top());
		if (_lastInstruction->getNumEdges() == 2) {
			new CFGEdge(_lastInstruction->getEdge(0)->getDest(), _currentInstruction.top(), "false");
		} else {
			assert(false);
		}
	} else {
		new CFGEdge(_lastInstruction, _currentInstruction.top());
	}

	_lastInstruction = _currentInstruction.top();
	_blocks.pop_front();
}

void CFGvisitor::OnBrExpr(BrExpr* expr) {
	std::string target = expr->var.name();
	for (auto p : _blocks) {
		if (target.compare(p.first) == 0) {
			new CFGEdge(_currentInstruction.top(), p.second);
			_lastInstruction = _currentInstruction.top();
			return;
		}
	}
	assert(false);
}
void CFGvisitor::OnBrIfExpr(BrIfExpr* expr) {
	if (!visitArity1()) {
		return;
	}
	for (auto p : _blocks) {
		if (expr->var.name().compare(p.first) == 0) {
			new CFGEdge(_currentInstruction.top(), p.second, "true");
			break;
		}
	}
	_lastInstruction = _currentInstruction.top();
}

void CFGvisitor::OnBrOnExnExpr(BrOnExnExpr*) {
	assert(false);
}

void CFGvisitor::OnBrTableExpr(BrTableExpr* expr) {
	if (!visitArity1()) {
		return;
	}
	for (Index i = 0; i < expr->targets.size(); i++) {
		for (auto block : _blocks) {
			if (block.first.compare(expr->targets[i].name()) == 0) {
				new CFGEdge(_currentInstruction.top(), block.second, std::to_string(i));
			}
		}
	}

	for (auto block : _blocks) {
		if (block.first.compare(expr->default_target.name()) == 0) {
			new CFGEdge(_currentInstruction.top(), block.second, "default");
		}
	}
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
void CFGvisitor::OnDropExpr(DropExpr*) {
	visitArity1();
}

void CFGvisitor::OnGlobalGetExpr(GlobalGetExpr*) {
}

void CFGvisitor::OnGlobalSetExpr(GlobalSetExpr*) {
}

void CFGvisitor::OnIfExpr(IfExpr*) {
	Index numChildren = _currentInstruction.top()->getNumEdges();
	if (numChildren < 2 || numChildren > 3) {
		assert(false);
	}
	Node* condition = _currentInstruction.top()->getEdge(0)->getDest();
	Node* trueBlockNode = _currentInstruction.top()->getEdge(1)->getDest();
	bool trueBlockUnreach = false;
	bool falseBlockUnreach = false;

	// Visit condition
	condition->accept(this);
	new CFGEdge(condition, getLeftMostLeaf(trueBlockNode), "true");

	// Visit True Block
	trueBlockNode->accept(this);
	if (_lastInstruction == nullptr) {
		// case unreachable
		_unreachableInsts.emplace(trueBlockNode);
		trueBlockUnreach = true;
	}

	if (numChildren == 3) {
		// In the Else "block" is the same block of the true
		// so we need to put the true block in the stack in case there is a br
		BlockExpr* trueBlock = cast<BlockExpr>(static_cast<Instruction*>(trueBlockNode)->getExpr());
		_blocks.emplace_front(trueBlock->block.label, static_cast<Instruction*>(trueBlockNode));

		Node* falseBlock = _currentInstruction.top()->getEdge(2)->getDest();
		new CFGEdge(condition, getLeftMostLeaf(falseBlock), "false");

		// Visit False Block
		falseBlock->accept(this);
		if (_lastInstruction == nullptr) {
			// case unreachable
			_unreachableInsts.emplace(falseBlock);
			falseBlockUnreach = true;
		}

		// Pop block
		_blocks.pop_front();
	}

	if (trueBlockUnreach && falseBlockUnreach) {
		_lastInstruction = nullptr;
	} else if (trueBlockUnreach && numChildren == 2) {
		_lastInstruction = _currentInstruction.top();
	} else {
		_lastInstruction = static_cast<Instruction*>(trueBlockNode);
	}
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

void CFGvisitor::OnNopExpr(NopExpr*) {
	_lastInstruction = _currentInstruction.top();
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

void CFGvisitor::OnUnreachableExpr(UnreachableExpr*) {
	new CFGEdge(_currentInstruction.top(), _graph->getTrap());
	_lastInstruction = nullptr;
}

void CFGvisitor::OnTryExpr(TryExpr*) {
	assert(false);
}
void CFGvisitor::OnThrowExpr(ThrowExpr*) {
	assert(false);
}

void CFGvisitor::OnRethrowExpr(RethrowExpr*) {
	assert(false);
}

void CFGvisitor::OnAtomicWaitExpr(AtomicWaitExpr*) {
	assert(false);
}

void CFGvisitor::OnAtomicNotifyExpr(AtomicNotifyExpr*) {
	assert(false);
}

void CFGvisitor::OnAtomicLoadExpr(AtomicLoadExpr*) {
	assert(false);
}

void CFGvisitor::OnAtomicStoreExpr(AtomicStoreExpr*) {
	assert(false);
}

void CFGvisitor::OnAtomicRmwExpr(AtomicRmwExpr*) {
	assert(false);
}

void CFGvisitor::OnAtomicRmwCmpxchgExpr(AtomicRmwCmpxchgExpr*) {
	assert(false);
}

void CFGvisitor::OnTernaryExpr(TernaryExpr*)
{
}
void CFGvisitor::OnSimdLaneOpExpr(SimdLaneOpExpr*) {
	assert(false);
}

void CFGvisitor::OnSimdShuffleOpExpr(SimdShuffleOpExpr*) {
	assert(false);
}

void CFGvisitor::OnLoadSplatExpr(LoadSplatExpr*) {
	assert(false);
}

void CFGvisitor::visitSequential(Node* node) {
	int numInst = node->getNumEdges();
	std::vector<Edge*> edges = node->getEdges();
	int i;
	for (i = 0; i < numInst - 1; i++) {
		edges[i]->accept(this);
		if (_lastInstruction == nullptr) {
			// unreachable
			return;
		}
		if (_lastInstruction->getType() == ExprType::BrIf) {
			new CFGEdge(_lastInstruction, getLeftMostLeaf(edges[i + 1]->getDest()), "false");
		}
		else if (_lastInstruction->getType() == ExprType::If) {
			Node* nextLeftMost = getLeftMostLeaf(edges[i + 1]->getDest());

			auto unreach = _unreachableInsts.find(_lastInstruction->getEdge(1)->getDest());
			if (unreach == _unreachableInsts.end()) {
				// not unreachable
				new CFGEdge(_lastInstruction->getEdge(1)->getDest(), nextLeftMost);
			}

			if (_lastInstruction->getNumEdges() == 2) {
				new CFGEdge(_lastInstruction->getEdge(0)->getDest(), nextLeftMost, "false");
			}
			else {
				assert(false);
			}
		}
		else {
			new CFGEdge(_lastInstruction, getLeftMostLeaf(edges[i + 1]->getDest()));
		}
	}
	if (_lastInstruction == nullptr && i > 1) {
		return;
	}
	edges[i]->accept(this);
}

bool CFGvisitor::visitArity1() {
	if (_currentInstruction.top()->getNumEdges() != 1) {
		assert(false);
	}

	Node* child = _currentInstruction.top()->getEdges()[0]->getDest();
	child->accept(this);
	if (_lastInstruction == nullptr) {
		return false;
	}
	if (_lastInstruction->getType() == ExprType::BrIf) {
		new CFGEdge(_lastInstruction, _currentInstruction.top(), "false");
	} else {
		new CFGEdge(_lastInstruction, _currentInstruction.top());
	}
	_lastInstruction = _currentInstruction.top();
	return true;
}

bool CFGvisitor::visitArity2() {
	if (_currentInstruction.top()->getNumEdges() != 2) {
		assert(false);
	}
	Node* left = _currentInstruction.top()->getEdges()[0]->getDest();
	Node* right = _currentInstruction.top()->getEdges()[1]->getDest();

	// visit left
	left->accept(this);
	if (_lastInstruction == nullptr) {
		return false;
	}
	// add CFG edge from result of left visit to the left most leaf of right children
	if (_lastInstruction->getType() == ExprType::BrIf) {
		new CFGEdge(_lastInstruction, getLeftMostLeaf(right), "false");
	} else {
		new CFGEdge(_lastInstruction, getLeftMostLeaf(right));
	}

	// visit right
	if (_lastInstruction == nullptr) {
		return false;
	}
	right->accept(this);
	// add CFG edge between right visit and current
	if (_lastInstruction->getType() == ExprType::BrIf) {
		new CFGEdge(_lastInstruction, _currentInstruction.top(), "false");
	} else {
		new CFGEdge(_lastInstruction, _currentInstruction.top());
	}

	_lastInstruction = _currentInstruction.top();
	return true;
}
}