#include "src/cfg-visitor.h"

namespace wasmati {

Node* CFGvisitor::getLeftMostLeaf(Node* node) const {
    if (node->getNumOutEdges() == 0) {
        return node;
    } else if (node->getOutEdge(0)->type() != EdgeType::AST) {
        return node;
    } else {
        return getLeftMostLeaf(node->getOutEdge(0)->dest());
    }
}

void CFGvisitor::visitASTEdge(ASTEdge* e) {
    e->dest()->accept(this);
}

void CFGvisitor::visitModule(Module* mod) {
    for (auto node : mod->outEdges()) {
        node->accept(this);
    }
}

void CFGvisitor::visitFunction(Function* func) {
    if (func->getNumOutEdges() != 3) {
        fprintf(stderr, "Function without 3 children nodes.\n");
        assert(false);
    }
    func->getOutEdge(2)->accept(this);
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
    if (inst->getNumOutEdges() == 0) {
        return;
    } else if (inst->getNumOutEdges() > 1) {
        assert(false);
    }

    Node* child = inst->getOutEdge(0)->dest();
    child->accept(this);
    if (_lastInstruction == nullptr) {
        // unreachable
        return;
    }
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
        auto unreach =
            _unreachableInsts.find(_lastInstruction->getOutEdge(1)->dest());
        if (unreach != _unreachableInsts.end()) {
            // unreachable
            _unreachableInsts.clear();
            return;
        }
        new CFGEdge(_lastInstruction->getOutEdge(1)->dest(),
                    _blocks.front().second);
        if (_lastInstruction->getNumOutEdges() == 2) {
            new CFGEdge(_lastInstruction->getOutEdge(0)->dest(),
                        _blocks.front().second, "false");
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
        // if there are CFG edges to this block
        // then there are br and is not unreachable from now on
        // otherwise is unreachable
        if (_currentInstruction.top()->hasInEdgesOf(EdgeType::CFG)) {
            _lastInstruction = _currentInstruction.top();
        }
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
        auto unreach =
            _unreachableInsts.find(_lastInstruction->getOutEdge(1)->dest());
        if (unreach != _unreachableInsts.end()) {
            // unreachable
            _unreachableInsts.clear();
            _blocks.pop_front();
            _lastInstruction = nullptr;
            return;
        }
        new CFGEdge(_lastInstruction->getOutEdge(1)->dest(),
                    _currentInstruction.top());
        if (_lastInstruction->getNumOutEdges() == 2) {
            new CFGEdge(_lastInstruction->getOutEdge(0)->dest(),
                        _currentInstruction.top(), "false");
        } else {
            assert(false);
        }
    } else if (_lastInstruction->getType() == ExprType::Block) {
        if (_lastInstruction->hasInEdgesOf(EdgeType::CFG)) {
            new CFGEdge(_lastInstruction, _currentInstruction.top());
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
                new CFGEdge(_currentInstruction.top(), block.second,
                            std::to_string(i));
            }
        }
    }

    for (auto block : _blocks) {
        if (block.first.compare(expr->default_target.name()) == 0) {
            new CFGEdge(_currentInstruction.top(), block.second, "default");
        }
    }
}
void CFGvisitor::OnCallExpr(CallExpr*) {
    visitSequential(_currentInstruction.top());

    if (_lastInstruction == nullptr) {
        // unreachable
        return;
    }

    if (_currentInstruction.top()->getNumOutEdges() > 0) {
        new CFGEdge(_lastInstruction, _currentInstruction.top());
    }

    _lastInstruction = _currentInstruction.top();
}

void CFGvisitor::OnCallIndirectExpr(CallIndirectExpr*) {
    visitSequential(_currentInstruction.top());

    if (_lastInstruction == nullptr) {
        // unreachable
        return;
    }

    new CFGEdge(_lastInstruction, _currentInstruction.top());

    _lastInstruction = _currentInstruction.top();
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
    _lastInstruction = _currentInstruction.top();
}

void CFGvisitor::OnGlobalSetExpr(GlobalSetExpr*) {
    visitArity1();
}

void CFGvisitor::OnIfExpr(IfExpr*) {
    Index numChildren = _currentInstruction.top()->getNumOutEdges();
    if (numChildren < 2 || numChildren > 3) {
        assert(false);
    }
    Instruction* condition =
        cast<Instruction>(_currentInstruction.top()->getOutEdge(0)->dest());
    Node* trueBlockNode = _currentInstruction.top()->getOutEdge(1)->dest();
    bool trueBlockUnreach = false;
    bool falseBlockUnreach = false;

    // Visit condition
    condition->accept(this);
    if (condition->getType() == ExprType::If) {
        condition = cast<Instruction>(condition->getOutEdge(1)->dest());
    } else if (condition->getType() == ExprType::Br) {
        // it should become unreachable
        _lastInstruction = nullptr;
        return;
    }
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
        BlockExpr* trueBlock =
            cast<BlockExpr>(cast<Instruction>(trueBlockNode)->getExpr());
        _blocks.emplace_front(trueBlock->block.label,
                              cast<Instruction>(trueBlockNode));

        Node* falseBlock = _currentInstruction.top()->getOutEdge(2)->dest();
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
    } else if (numChildren == 2 && !trueBlockUnreach) {
        new CFGEdge(condition, trueBlockNode, "false");
    }

    if (trueBlockUnreach && falseBlockUnreach) {
        _lastInstruction = nullptr;
    } else if (trueBlockUnreach && numChildren == 2) {
        _lastInstruction = _currentInstruction.top();
    } else {
        _lastInstruction = cast<Instruction>(trueBlockNode);
    }
}

void CFGvisitor::OnLoadExpr(LoadExpr*) {
    visitArity1();
}
void CFGvisitor::OnLocalGetExpr(LocalGetExpr*) {
    _lastInstruction = _currentInstruction.top();
}

void CFGvisitor::OnLocalSetExpr(LocalSetExpr*) {
    visitArity1();
}

void CFGvisitor::OnLocalTeeExpr(LocalTeeExpr*) {
    visitArity1();
}

void CFGvisitor::OnLoopExpr(LoopExpr* loop) {
    _blocks.emplace_front(loop->block.label, _currentInstruction.top());
    visitSequential(_currentInstruction.top());

    if (_currentInstruction.top()->hasInEdgesOf(EdgeType::CFG)) {
        new CFGEdge(_currentInstruction.top(),
                    getLeftMostLeaf(_currentInstruction.top()));
    }

    // Last instruction of the loop
    if (_lastInstruction->getType() == ExprType::Br) {
        BrExpr* brExpr = cast<BrExpr>(_lastInstruction->getExpr());
        if (brExpr->var.name().compare(loop->block.label) == 0) {
            _lastInstruction = nullptr;
            // becomes unreachable
        }
    }

    _blocks.pop_front();
}

void CFGvisitor::OnMemoryCopyExpr(MemoryCopyExpr*) {
    assert(false);
}

void CFGvisitor::OnDataDropExpr(DataDropExpr*) {
    assert(false);
}

void CFGvisitor::OnMemoryFillExpr(MemoryFillExpr*) {
    assert(false);
}

void CFGvisitor::OnMemoryGrowExpr(MemoryGrowExpr*) {
    visitArity1();
}

void CFGvisitor::OnMemoryInitExpr(MemoryInitExpr*) {
    assert(false);
}

void CFGvisitor::OnMemorySizeExpr(MemorySizeExpr*) {
    _lastInstruction = _currentInstruction.top();
}

void CFGvisitor::OnTableCopyExpr(TableCopyExpr*) {
    assert(false);
}

void CFGvisitor::OnElemDropExpr(ElemDropExpr*) {
    assert(false);
}

void CFGvisitor::OnTableInitExpr(TableInitExpr*) {
    assert(false);
}

void CFGvisitor::OnTableGetExpr(TableGetExpr*) {
    assert(false);
}

void CFGvisitor::OnTableSetExpr(TableSetExpr*) {
    assert(false);
}

void CFGvisitor::OnTableGrowExpr(TableGrowExpr*) {
    assert(false);
}

void CFGvisitor::OnTableSizeExpr(TableSizeExpr*) {
    assert(false);
}

void CFGvisitor::OnTableFillExpr(TableFillExpr*) {
    assert(false);
}

void CFGvisitor::OnRefFuncExpr(RefFuncExpr*) {
    assert(false);
}

void CFGvisitor::OnRefNullExpr(RefNullExpr*) {
    assert(false);
}

void CFGvisitor::OnRefIsNullExpr(RefIsNullExpr*) {
    assert(false);
}

void CFGvisitor::OnNopExpr(NopExpr*) {
    _lastInstruction = _currentInstruction.top();
}

void CFGvisitor::OnReturnExpr(ReturnExpr*) {
    if (_currentInstruction.top()->getNumOutEdges() == 0) {
        _lastInstruction = nullptr;
        return;
    } else if (_currentInstruction.top()->getNumOutEdges() > 1) {
        assert(false);
    }

    Node* child = _currentInstruction.top()->getOutEdge(0)->dest();
    child->accept(this);
    new CFGEdge(_lastInstruction, _currentInstruction.top());
    // further seq instructions get unreachable
    _lastInstruction = nullptr;
}

void CFGvisitor::OnReturnCallExpr(ReturnCallExpr*) {
    assert(false);
}

void CFGvisitor::OnReturnCallIndirectExpr(ReturnCallIndirectExpr*) {
    assert(false);
}

void CFGvisitor::OnSelectExpr(SelectExpr*) {
    visitSequential(_currentInstruction.top());

    if (_lastInstruction == nullptr) {
        // unreachable
        return;
    }

    new CFGEdge(_lastInstruction, _currentInstruction.top());

    _lastInstruction = _currentInstruction.top();
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

void CFGvisitor::OnTernaryExpr(TernaryExpr*) {
    visitSequential(_currentInstruction.top());

    if (_lastInstruction == nullptr) {
        // unreachable
        return;
    }

    new CFGEdge(_lastInstruction, _currentInstruction.top());

    _lastInstruction = _currentInstruction.top();
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
    int numInst = node->getNumOutEdges();
    if (numInst == 0) {
        return;
    }
    const std::vector<Edge*> edges = node->outEdges();
    int i;
    for (i = 0; i < numInst - 1; i++) {
        edges[i]->accept(this);
        if (_lastInstruction == nullptr) {
            // unreachable
            return;
        }
        if (_lastInstruction->getType() == ExprType::BrIf) {
            new CFGEdge(_lastInstruction, getLeftMostLeaf(edges[i + 1]->dest()),
                        "false");
        } else if (_lastInstruction->getType() == ExprType::Br) {
            // rest of instructions become unreachable
            _lastInstruction = nullptr;
            return;
        } else if (_lastInstruction->getType() == ExprType::If) {
            Node* nextLeftMost = getLeftMostLeaf(edges[i + 1]->dest());

            auto unreach =
                _unreachableInsts.find(_lastInstruction->getOutEdge(1)->dest());
            if (unreach == _unreachableInsts.end()) {
                // not unreachable
                new CFGEdge(_lastInstruction->getOutEdge(1)->dest(),
                            nextLeftMost);
            }

            if (_lastInstruction->getNumOutEdges() == 2) {
                new CFGEdge(_lastInstruction->getOutEdge(0)->dest(),
                            nextLeftMost, "false");
            } else {
                assert(false);
            }
        } else {
            new CFGEdge(_lastInstruction,
                        getLeftMostLeaf(edges[i + 1]->dest()));
        }
    }
    if (_lastInstruction == nullptr && i > 1) {
        return;
    }
    edges[i]->accept(this);
}

bool CFGvisitor::visitArity1() {
    if (_currentInstruction.top()->getNumOutEdges() != 1) {
        assert(false);
    }

    Node* child = _currentInstruction.top()->getOutEdge(0)->dest();
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
    if (_currentInstruction.top()->getNumOutEdges() != 2) {
        assert(false);
    }
    Node* left = _currentInstruction.top()->getOutEdge(0)->dest();
    Node* right = _currentInstruction.top()->getOutEdge(1)->dest();

    // visit left
    left->accept(this);
    if (_lastInstruction == nullptr) {
        return false;
    }
    // add CFG edge from result of left visit to the left most leaf of right
    // children
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
}  // namespace wasmati
