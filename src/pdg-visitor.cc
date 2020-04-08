#include "src/pdg-visitor.h"

namespace wasmati {

void PDGvisitor::visitCFGEdge(CFGEdge* e) {
    if (!e->_label.empty()) {
        // it is true or false label
        new PDGEdge(e);
    }
    e->dest()->accept(this);
}

void PDGvisitor::visitModule(Module* mod) {
    for (auto e : mod->outEdges()) {
        e->dest()->accept(this);
    }
}

void PDGvisitor::visitFunction(Function* f) {
    _currentFunction = f->getFunctionExpr();
    f->getOutEdge(2)->dest()->accept(this);
}

void PDGvisitor::visitInstructions(Instructions* insts) {
    auto reachDefs = std::make_shared<ReachDefinition>();
    // globals
    for (auto global : _graph->getModuleContext()->module.global_bindings) {
        reachDefs->insertGlobal(global.first);
    }
    // locals
    for (auto local : _currentFunction->bindings) {
        reachDefs->insertLocal(local.first);
    }

    std::vector<Edge*> outEdges = insts->outEdges(EdgeType::CFG);
    assert(outEdges.size() == 1);
    _reachDef[outEdges[0]->dest()].push_back(reachDefs);

    outEdges[0]->accept(this);
}

void PDGvisitor::visitInstruction(Instruction* inst) {
    Index inEdgesNum = inst->inEdges(EdgeType::CFG).size();
    if (_reachDef[inst].size() != inEdgesNum) {
        // wait until all paths reach here
        return;
    }
    _currentInstruction = inst;
    visitExpr(inst->getExpr());
    auto resultReachDef = _reachDef[inst][0];
    std::vector<Edge*> outEdges = inst->outEdges(EdgeType::CFG);

    if (outEdges.size() >= 1) {
        _reachDef[outEdges[0]->dest()].push_back(resultReachDef);
        const std::string& label = cast<CFGEdge>(outEdges[0])->_label;
        if (label == "true" || label == "false" ) {
            resultReachDef->pop();
        }
    }

    if (outEdges.size() > 1) {
        for (Index i = 1; i < outEdges.size(); i++) {
            _reachDef[outEdges[i]->dest()].push_back(
                std::make_shared<ReachDefinition>(*resultReachDef));
        }
    }

    _reachDef.erase(inst);

    // visit edges
    for (auto e : outEdges) {
        e->accept(this);
    }
}

void PDGvisitor::visitReturn(Return*) {}

void PDGvisitor::visitElse(Else*) {
    assert(false);
}

void PDGvisitor::OnBinaryExpr(BinaryExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= 2);

    Definition arg1 = reachDef->pop();
    Definition arg2 = reachDef->pop();
    arg1.unionDef(arg2);
    arg1.insertPDGEdge(_currentInstruction);
    arg1.clear(_currentInstruction);
    reachDef->push(arg1);
}

void PDGvisitor::OnBlockExpr(BlockExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    int numReachDefs = reachDefs.size();

    if (numReachDefs <= 1) {
        return;
    }

    for (int i = 1; i < numReachDefs; i++) {
        reachDefs[0]->unionDef(*reachDefs[i]);
    }
}
void PDGvisitor::OnBrExpr(BrExpr*) {}
void PDGvisitor::OnBrIfExpr(BrIfExpr*) {}
void PDGvisitor::OnBrOnExnExpr(BrOnExnExpr*) {
    assert(false);
}

void PDGvisitor::OnBrTableExpr(BrTableExpr*) {}
void PDGvisitor::OnCallExpr(CallExpr*) {}
void PDGvisitor::OnCallIndirectExpr(CallIndirectExpr*) {}
void PDGvisitor::OnCompareExpr(CompareExpr*) {}
void PDGvisitor::OnConstExpr(ConstExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);
    auto reachDef = reachDefs[0];

    reachDef->push();  // push empty def
}
void PDGvisitor::OnConvertExpr(ConvertExpr*) {}
void PDGvisitor::OnDropExpr(DropExpr*) {}
void PDGvisitor::OnGlobalGetExpr(GlobalGetExpr*) {}
void PDGvisitor::OnGlobalSetExpr(GlobalSetExpr*) {}
void PDGvisitor::OnIfExpr(IfExpr*) {}
void PDGvisitor::OnLoadExpr(LoadExpr*) {}

void PDGvisitor::OnLocalGetExpr(LocalGetExpr* expr) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);
    auto reachDef = reachDefs[0];

    reachDef->push(reachDef->getLocal(expr->var.name()));
    Definition& varDef = reachDef->peek();
    if (varDef.isEmpty()) {
        // set is empty, thus the var depends on itself
        varDef.insert(expr->var.name(), _currentInstruction);
    }
}

void PDGvisitor::OnLocalSetExpr(LocalSetExpr*) {}
void PDGvisitor::OnLocalTeeExpr(LocalTeeExpr*) {}
void PDGvisitor::OnLoopExpr(LoopExpr*) {}
void PDGvisitor::OnMemoryCopyExpr(MemoryCopyExpr*) {
    assert(false);
}

void PDGvisitor::OnDataDropExpr(DataDropExpr*) {
    assert(false);
}

void PDGvisitor::OnMemoryFillExpr(MemoryFillExpr*) {
    assert(false);
}

void PDGvisitor::OnMemoryGrowExpr(MemoryGrowExpr*) {}
void PDGvisitor::OnMemoryInitExpr(MemoryInitExpr*) {
    assert(false);
}

void PDGvisitor::OnMemorySizeExpr(MemorySizeExpr*) {
    assert(false);
}

void PDGvisitor::OnTableCopyExpr(TableCopyExpr*) {
    assert(false);
}

void PDGvisitor::OnElemDropExpr(ElemDropExpr*) {
    assert(false);
}

void PDGvisitor::OnTableInitExpr(TableInitExpr*) {
    assert(false);
}

void PDGvisitor::OnTableGetExpr(TableGetExpr*) {
    assert(false);
}

void PDGvisitor::OnTableSetExpr(TableSetExpr*) {
    assert(false);
}

void PDGvisitor::OnTableGrowExpr(TableGrowExpr*) {
    assert(false);
}

void PDGvisitor::OnTableSizeExpr(TableSizeExpr*) {
    assert(false);
}

void PDGvisitor::OnTableFillExpr(TableFillExpr*) {
    assert(false);
}

void PDGvisitor::OnRefFuncExpr(RefFuncExpr*) {
    assert(false);
}

void PDGvisitor::OnRefNullExpr(RefNullExpr*) {
    assert(false);
}

void PDGvisitor::OnRefIsNullExpr(RefIsNullExpr*) {
    assert(false);
}

void PDGvisitor::OnNopExpr(NopExpr*) {}
void PDGvisitor::OnReturnExpr(ReturnExpr*) {}
void PDGvisitor::OnReturnCallExpr(ReturnCallExpr*) {
    assert(false);
}

void PDGvisitor::OnReturnCallIndirectExpr(ReturnCallIndirectExpr*) {
    assert(false);
}

void PDGvisitor::OnSelectExpr(SelectExpr*) {}
void PDGvisitor::OnStoreExpr(StoreExpr*) {}
void PDGvisitor::OnUnaryExpr(UnaryExpr*) {}
void PDGvisitor::OnUnreachableExpr(UnreachableExpr*) {}
void PDGvisitor::OnTryExpr(TryExpr*) {
    assert(false);
}

void PDGvisitor::OnThrowExpr(ThrowExpr*) {
    assert(false);
}

void PDGvisitor::OnRethrowExpr(RethrowExpr*) {
    assert(false);
}

void PDGvisitor::OnAtomicWaitExpr(AtomicWaitExpr*) {
    assert(false);
}

void PDGvisitor::OnAtomicNotifyExpr(AtomicNotifyExpr*) {
    assert(false);
}

void PDGvisitor::OnAtomicLoadExpr(AtomicLoadExpr*) {
    assert(false);
}

void PDGvisitor::OnAtomicStoreExpr(AtomicStoreExpr*) {
    assert(false);
}

void PDGvisitor::OnAtomicRmwExpr(AtomicRmwExpr*) {
    assert(false);
}

void PDGvisitor::OnAtomicRmwCmpxchgExpr(AtomicRmwCmpxchgExpr*) {
    assert(false);
}

void PDGvisitor::OnTernaryExpr(TernaryExpr*) {}
void PDGvisitor::OnSimdLaneOpExpr(SimdLaneOpExpr*) {
    assert(false);
}

void PDGvisitor::OnSimdShuffleOpExpr(SimdShuffleOpExpr*) {
    assert(false);
}

void PDGvisitor::OnLoadSplatExpr(LoadSplatExpr*) {
    assert(false);
}

}  // namespace wasmati
