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
        if (inst->getType() != ExprType::BrIf &&
            (label == "true" || label == "false")) {
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

void PDGvisitor::visitReturn(Return* node) {
    auto reachDefs = _reachDef[node];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() <= 1);

    if (reachDef->stackSize() == 1) {
        reachDef->peek()->insertPDGEdge(node);
    }
}

void PDGvisitor::visitElse(Else*) {
    assert(false);
}

void PDGvisitor::OnBinaryExpr(BinaryExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= 2);

    auto arg1 = reachDef->pop();
    auto arg2 = reachDef->pop();
    arg1->unionDef(arg2);
    arg1->insertPDGEdge(_currentInstruction);
    arg1->clear(_currentInstruction);
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
void PDGvisitor::OnBrExpr(BrExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);
}

void PDGvisitor::OnBrIfExpr(BrIfExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);
    auto reachDef = reachDefs[0];

    auto arg = reachDef->pop();
    arg->insertPDGEdge(_currentInstruction);
}

void PDGvisitor::OnBrOnExnExpr(BrOnExnExpr*) {
    assert(false);
}

void PDGvisitor::OnBrTableExpr(BrTableExpr*) {
    // TODO
    assert(false);
}

void PDGvisitor::OnCallExpr(CallExpr* expr) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto arity = _graph->getModuleContext()->GetExprArity(*expr);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= arity.nargs);

    // Pop args
    for (Index i = 0; i < arity.nargs; i++) {
        auto arg = reachDef->pop();
        arg->insertPDGEdge(_currentInstruction);
    }

    // push returns
    for (Index i = 0; i < arity.nreturns; i++) {
        reachDef->push();
    }
}

void PDGvisitor::OnCallIndirectExpr(CallIndirectExpr* expr) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto arity = _graph->getModuleContext()->GetExprArity(*expr);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= arity.nargs);

    // pop func index
    auto index = reachDef->pop();
    index->insertPDGEdge(_currentInstruction);

    // Pop args
    for (Index i = 0; i < arity.nargs - 1; i++) {
        auto arg = reachDef->pop();
        arg->insertPDGEdge(_currentInstruction);
    }

    // push returns
    for (Index i = 0; i < arity.nreturns; i++) {
        reachDef->push();
    }
}

void PDGvisitor::OnCompareExpr(CompareExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= 2);

    auto arg1 = reachDef->pop();
    auto arg2 = reachDef->pop();
    arg1->unionDef(arg2);
    arg1->insertPDGEdge(_currentInstruction);
    arg1->clear(_currentInstruction);
    reachDef->push(arg1);
}

void PDGvisitor::OnConstExpr(ConstExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);
    auto reachDef = reachDefs[0];

    reachDef->push();  // push empty def
}

void PDGvisitor::OnConvertExpr(ConvertExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= 1);

    // write dependencies of arg in top of the stack to this inst
    reachDef->peek()->insertPDGEdge(_currentInstruction);

    // following inst using this value depend of the result in this inst
    reachDef->peek()->clear(_currentInstruction);
}

void PDGvisitor::OnDropExpr(DropExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= 1);

    reachDef->pop();  // pop last element of stack
}

void PDGvisitor::OnGlobalGetExpr(GlobalGetExpr* expr) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);
    auto reachDef = reachDefs[0];

    reachDef->push(reachDef->getGlobal(expr->var.name()));
    auto varDef = reachDef->peek();
    if (varDef->isEmpty()) {
        // set is empty, thus the var depends on itself
        varDef->insert(expr->var.name(), _currentInstruction);
    }
}

void PDGvisitor::OnGlobalSetExpr(GlobalSetExpr* expr) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= 1);

    reachDef->insertGlobal(expr->var.name(), reachDef->pop());
}

void PDGvisitor::OnIfExpr(IfExpr*) {
    assert(false);
}

void PDGvisitor::OnLoadExpr(LoadExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= 1);

    // pop index and wirte dependencies
    reachDef->pop()->insertPDGEdge(_currentInstruction);

    // push a value to the stack
    reachDef->push();
}

void PDGvisitor::OnLocalGetExpr(LocalGetExpr* expr) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);
    auto reachDef = reachDefs[0];

    reachDef->push(reachDef->getLocal(expr->var.name()));
    auto varDef = reachDef->peek();
    if (varDef->isEmpty()) {
        // set is empty, thus the var depends on itself
        varDef->insert(expr->var.name(), _currentInstruction);
    }
}

void PDGvisitor::OnLocalSetExpr(LocalSetExpr* expr) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= 1);

    reachDef->insertLocal(expr->var.name(), reachDef->pop());
}

void PDGvisitor::OnLocalTeeExpr(LocalTeeExpr* expr) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= 1);

    // pop value
    auto arg = reachDef->pop();

    // perform a local.set of value
    reachDef->insertLocal(expr->var.name, arg);

    // push back value to stack
    reachDef->push(arg);
}

void PDGvisitor::OnLoopExpr(LoopExpr*) {
    // TODO
    assert(false);
}

void PDGvisitor::OnMemoryCopyExpr(MemoryCopyExpr*) {
    assert(false);
}

void PDGvisitor::OnDataDropExpr(DataDropExpr*) {
    assert(false);
}

void PDGvisitor::OnMemoryFillExpr(MemoryFillExpr*) {
    assert(false);
}

void PDGvisitor::OnMemoryGrowExpr(MemoryGrowExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= 1);

    auto n = reachDef->pop();
    // this inst depends on n
    n->insertPDGEdge(_currentInstruction);

    // memory.grow pushes new size of memory if OK or else an error number
    reachDef->push();
}

void PDGvisitor::OnMemoryInitExpr(MemoryInitExpr*) {
    assert(false);
}

void PDGvisitor::OnMemorySizeExpr(MemorySizeExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];

    // push size of memory to stack
    reachDef->push();
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

void PDGvisitor::OnNopExpr(NopExpr*) {
    // no operation => does nothing
}

void PDGvisitor::OnReturnExpr(ReturnExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() <= 1);

    if (reachDef->stackSize() == 1) {
        reachDef->peek()->insertPDGEdge(_currentInstruction);
    }
}

void PDGvisitor::OnReturnCallExpr(ReturnCallExpr*) {
    assert(false);
}

void PDGvisitor::OnReturnCallIndirectExpr(ReturnCallIndirectExpr*) {
    assert(false);
}

void PDGvisitor::OnSelectExpr(SelectExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= 3);

    auto c = reachDef->pop();
    auto val2 = reachDef->pop();
    auto val1 = reachDef->pop();

    // selects works: if c = 0 then val2 else val1
    // select depends on c, the following instructions will depend val1 and val2
    c->insertPDGEdge(_currentInstruction);

    // union of vals and push
    val1->unionDef(val2);
    reachDef->push(val1);
}

void PDGvisitor::OnStoreExpr(StoreExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= 2);

    auto c = reachDef->pop();
    auto i = reachDef->pop();

    // write dependencies to this inst
    c->insertPDGEdge(_currentInstruction);
    i->insertPDGEdge(_currentInstruction);
}

void PDGvisitor::OnUnaryExpr(UnaryExpr*) {
    auto reachDefs = _reachDef[_currentInstruction];
    assert(reachDefs.size() == 1);

    auto reachDef = reachDefs[0];
    assert(reachDef->stackSize() >= 1);

    auto arg = reachDef->peek();
    // write dependecies
    arg->insertPDGEdge(_currentInstruction);

    // Set dependencies to this inst
    arg->clear(_currentInstruction);
}

void PDGvisitor::OnUnreachableExpr(UnreachableExpr*) {
    // the program dies here with an exception
}

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

void PDGvisitor::OnTernaryExpr(TernaryExpr*) {
    // guess there are no ternary expresions in WebAssembly 1.0
    assert(false);
}

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
