#include "pdg-builder.h"

namespace wasmati {
void PDG::generatePDG() {
    if (!cpgOptions.loopName.empty()) {
        _verboseLoops = Queries::loopsInsts(cpgOptions.loopName);
    }
    Index counter = 0;
    for (Node* func : Query::functions()) {
        if (func->isImport()) {
            continue;
        }
        debug("[DEBUG][PDG][%u/%lu] Function %s\n", counter++,
              mc.module.funcs.size(), func->name().c_str());

        currentFunction = func->getFunc();

        auto filterInsts =
            NodeStream(func).children(Query::AST_EDGES).filter([](Node* n) {
                return n->type() == NodeType::Instructions;
            });
        assert(filterInsts.size() == 1);

        // clear
        _reachDef.clear();
        _loops.clear();
        _loopsInsts.clear();
        _loopsStack = {};

        visitInstructions(
            dynamic_cast<Instructions*>(filterInsts.findFirst().get()));

        while (_dfsList.size() != 0) {
            auto firstInst = _dfsList.front();
            _dfsList.pop_front();

            // set loopsStack
            _loopsStack = *(std::get<1>(firstInst));
            // set last node
            _lastNode = std::get<2>(firstInst);

            switch (std::get<0>(firstInst)->instType()) {
            case ExprType::Nop:
                visitNopInst(dynamic_cast<NopInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Unreachable:
                visitUnreachableInst(
                    dynamic_cast<UnreachableInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Return:
                visitReturnInst(
                    dynamic_cast<ReturnInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::BrTable:
                visitBrTableInst(
                    dynamic_cast<BrTableInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::CallIndirect:
                visitCallIndirectInst(
                    dynamic_cast<CallIndirectInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Drop:
                visitDropInst(dynamic_cast<DropInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Select:
                visitSelectInst(
                    dynamic_cast<SelectInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::MemorySize:
                visitMemorySizeInst(
                    dynamic_cast<MemorySizeInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::MemoryGrow:
                visitMemoryGrowInst(
                    dynamic_cast<MemoryGrowInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Const:
                visitConstInst(
                    dynamic_cast<ConstInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Binary:
                visitBinaryInst(
                    dynamic_cast<BinaryInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Compare:
                visitCompareInst(
                    dynamic_cast<CompareInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Convert:
                visitConvertInst(
                    dynamic_cast<ConvertInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Unary:
                visitUnaryInst(
                    dynamic_cast<UnaryInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Load:
                visitLoadInst(dynamic_cast<LoadInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Store:
                visitStoreInst(
                    dynamic_cast<StoreInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Br:
                visitBrInst(dynamic_cast<BrInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::BrIf:
                visitBrIfInst(dynamic_cast<BrIfInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Call:
                visitCallInst(dynamic_cast<CallInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::GlobalGet:
                visitGlobalGetInst(
                    dynamic_cast<GlobalGetInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::GlobalSet:
                visitGlobalSetInst(
                    dynamic_cast<GlobalSetInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::LocalGet:
                visitLocalGetInst(
                    dynamic_cast<LocalGetInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::LocalSet:
                visitLocalSetInst(
                    dynamic_cast<LocalSetInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::LocalTee:
                visitLocalTeeInst(
                    dynamic_cast<LocalTeeInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::Block:
                if (std::get<0>(firstInst)->isBeginBlock()) {
                    visitBeginBlockInst(
                        dynamic_cast<BeginBlockInst*>(std::get<0>(firstInst)));
                } else {
                    visitBlockInst(
                        dynamic_cast<BlockInst*>(std::get<0>(firstInst)));
                }
                break;
            case ExprType::Loop:
                visitLoopInst(dynamic_cast<LoopInst*>(std::get<0>(firstInst)));
                break;
            case ExprType::If:
                visitIfInst(dynamic_cast<IfInst*>(std::get<0>(firstInst)));
                break;
            default:
                assert(false);
                break;
            }
        }
    }
}

void PDG::visitCFGEdge(Edge* e, std::shared_ptr<std::stack<LoopInst*>> stack) {
    assert(e->type() == EdgeType::CFG);
    if (!e->label().empty()) {
        // it is true or false label or (br_table)
        auto outEdges = e->src()->outEdges(EdgeType::PDG);
        auto filterQuery = Query::filterEdges(outEdges, [&](Edge* edge) {
            return e->label().compare(edge->label()) == 0;
        });
        if (filterQuery.size() == 0) {
            new PDGEdge(dynamic_cast<CFGEdge*>(e));
        }
    }

    _dfsList.emplace_front(e->dest(), stack, e->src());
}

void PDG::visitInstructions(Instructions* node) {
    auto reachDefs = std::make_shared<ReachDefinition>();
    // globals
    for (auto global : mc.module.global_bindings) {
        reachDefs->insertGlobal(global.first);
    }
    // locals
    for (auto local : currentFunction->bindings) {
        reachDefs->insertLocal(local.first);
    }

    auto outEdges = EdgeStream(node->outEdges(EdgeType::CFG));
    assert(outEdges.size() == 1);
    auto first = outEdges.findFirst();

    _reachDef[first.get()->dest()].insert(reachDefs);

    visitCFGEdge(first.get(),
                 std::make_shared<std::stack<LoopInst*>>(_loopsStack));
}

void PDG::visitNopInst(NopInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    // no operation => does nothing
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    advance(node, reachDef);
}

void PDG::visitUnreachableInst(UnreachableInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    // the program dies here with an exception
}

void PDG::visitReturnInst(ReturnInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() <= 1);
    assert(currentFunction->GetNumResults() == reachDef->stackSize());
    if (reachDef->stackSize() == 1) {
        reachDef->peek()->insertPDGEdge(node);
    }
    // ---------------------------------------
    advance(node, reachDef);
}

void PDG::visitBrTableInst(BrTableInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 1);

    auto arg = reachDef->pop();
    arg->insertPDGEdge(node);

    // ---------------------------------------
    advance(node, reachDef);
}

void PDG::visitDropInst(DropInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 1);
    // pop last element of stack
    reachDef->pop();
    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitSelectInst(SelectInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 3);

    auto c = reachDef->pop();
    auto val2 = reachDef->pop();
    auto val1 = reachDef->pop();

    // selects works: if c = 0 then val2 else val1
    // select depends on c, the following instructions will depend val1 and val2
    c->insertPDGEdge(node);

    // union of vals and push
    val1->unionDef(val2);
    reachDef->push(val1);

    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitMemorySizeInst(MemorySizeInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    // push size of memory to stack
    reachDef->push();
    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitMemoryGrowInst(MemoryGrowInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 1);

    auto n = reachDef->pop();
    // this inst depends on n
    n->insertPDGEdge(node);

    // memory.grow pushes new size of memory if OK or else an error number
    reachDef->push();

    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitConstInst(ConstInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);

    reachDef->push();
    auto def = reachDef->peek();
    def->insert(&node->value(), node);
    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitBinaryInst(BinaryInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 2);

    auto arg1 = reachDef->pop();
    auto arg2 = reachDef->pop();
    arg1->unionDef(arg2);
    arg1->insertPDGEdge(node);
    arg1->removeConsts();
    arg1->clear(node);
    reachDef->push(arg1);
    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitCompareInst(CompareInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 2);

    auto arg1 = reachDef->pop();
    auto arg2 = reachDef->pop();
    arg1->unionDef(arg2);
    arg1->insertPDGEdge(node);
    arg1->removeConsts();
    arg1->clear(node);
    reachDef->push(arg1);
    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitConvertInst(ConvertInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 1);

    // write dependencies of arg in top of the stack to this inst
    reachDef->peek()->insertPDGEdge(node);

    // following inst using this value depend of the result in this inst
    reachDef->peek()->clear(node);

    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitUnaryInst(UnaryInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 1);

    auto arg = reachDef->peek();
    // write dependecies
    arg->insertPDGEdge(node);
    arg->removeConsts();

    // Set dependencies to this inst
    arg->clear(node);

    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitLoadInst(LoadInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 1);

    // pop index and write dependencies
    reachDef->pop()->insertPDGEdge(node);

    // push a value to the stack
    reachDef->push();

    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitStoreInst(StoreInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 2);

    auto c = reachDef->pop();
    auto i = reachDef->pop();

    // write dependencies to this inst
    c->insertPDGEdge(node);
    i->insertPDGEdge(node);
    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitBrInst(BrInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    // it expects the jump block to pop labels
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    advance(node, reachDef);
}
void PDG::visitBrIfInst(BrIfInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 1);

    auto arg = reachDef->pop();
    arg->insertPDGEdge(node);

    // it expects the jumpp block (if true) to pop labels
    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitGlobalGetInst(GlobalGetInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);

    reachDef->push(reachDef->getGlobal(node->label()));
    auto varDef = reachDef->peek();
    varDef->insertPDGEdge(node);
    varDef->clear(node);
    if (varDef->isEmpty()) {
        // set is empty, thus the var depends on itself
        varDef->insert(node->label(), PDGType::Global, node);
    }
    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitGlobalSetInst(GlobalSetInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 1);

    auto arg = reachDef->pop();
    arg->insertPDGEdge(node);
    arg->clear(node);
    reachDef->insertGlobal(node->label(), arg);

    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitLocalGetInst(LocalGetInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);

    reachDef->push(reachDef->getLocal(node->label()));
    auto varDef = reachDef->peek();
    varDef->insertPDGEdge(node);
    varDef->clear(node);
    if (varDef->isEmpty()) {
        // set is empty, thus the var depends on itself
        varDef->insert(node->label(), PDGType::Local, node);
    }

    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitLocalSetInst(LocalSetInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 1);

    auto arg = reachDef->pop();
    arg->insertPDGEdge(node);
    arg->clear(node);

    reachDef->insertLocal(node->label(), arg);
    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitLocalTeeInst(LocalTeeInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 1);

    // pop value
    auto arg = reachDef->pop();
    arg->insertPDGEdge(node);

    // perform a local.set of value
    reachDef->insertLocal(node->label(), arg);

    arg->clear(node);

    // push back value to stack
    reachDef->push(arg);

    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitCallInst(CallInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= node->nargs());

    // Pop args
    for (Index i = 0; i < node->nargs(); i++) {
        auto arg = reachDef->pop();
        arg->insertPDGEdge(node);
    }

    // push returns
    assert(node->nresults() <= 1);
    for (Index i = 0; i < node->nresults(); i++) {
        reachDef->push();
        auto def = reachDef->peek();
        def->insert(node->label(), PDGType::Function, node);
    }

    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitCallIndirectInst(CallIndirectInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= node->nargs());

    // pop func index
    auto index = reachDef->pop();
    index->insertPDGEdge(node);

    // Pop args
    for (Index i = 0; i < node->nargs() - 1; i++) {
        auto arg = reachDef->pop();
        arg->insertPDGEdge(node);
    }

    // push returns
    assert(node->nresults() <= 1);
    for (Index i = 0; i < node->nresults(); i++) {
        reachDef->push();
        auto def = reachDef->peek();
        def->insert(node->label(), PDGType::Function, node);
    }
    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitBeginBlockInst(BeginBlockInst* node) {
    if (!_loopsStack.empty() && _loopsInsts.count(_loopsStack.top()) == 1) {
        _loopsInsts[_loopsStack.top()].insert(node);
    }
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    reachDef->pushLabel(node->label());
    // logDefinition(node, reachDef);
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitBlockInst(BlockInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    for (auto& reachDef : _reachDef[node]) {
        assert(reachDef->stackSize() >= node->nresults());

        // gets results
        auto results = reachDef->pop(node->nresults());
        // pop label
        reachDef->popLabel(node->label());
        // push back the results
        reachDef->push(results);
    }
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    // ---------------------------------------
    advance(node, reachDef);
}
void PDG::visitLoopInst(LoopInst* node) {
    if (_loopsInsts.count(node) == 0) {
        _loopsInsts[node] =
            Query::BFSincludes({node}, Query::ALL_NODES, Query::AST_EDGES);
    }
    int count = _loops.count(node);
    if (waitPaths(node, true)) {
        return;
    } else if (count == 0) {
        _loopsStack.push(node);
    }
    // ---------------------------------------
    //debug("loop %s : dfslist: %u\n", node->label().c_str(), _dfsList.size());
    auto test = node->label() == "$L73";
    auto reachDef = getReachDef(node);
    if (count == 1) {
        reachDef->unionDef(_loops[node]);
        // logDefinition(node, reachDef);
        if (reachDef->equals(*_loops[node])) {
            _loopsEntrances[node].second =
                std::make_shared<ReachDefinition>(*reachDef);
            if (_loopsInsts[node].count(_lastNode) == 1 &&
                (_loopsStack.empty() || _loopsStack.top() != node)) {
                return;
            }
            if (!_loopsStack.empty() && _loopsStack.top() == node) {
                // pop stack
                _loopsStack.pop();
            }

            _reachDef.erase(node);
            advance(node, reachDef);
            return;
        } else if (_loopsStack.empty() || _loopsStack.top() != node) {
            if (_loopsInsts[node].count(_lastNode) == 0) {
                if (reachDef->equals(*_loopsEntrances[node].first)) {
                    reachDef = std::make_shared<ReachDefinition>(
                        *_loopsEntrances[node].second);
                } else {
                    _loopsEntrances[node] = std::make_pair(
                        std::make_shared<ReachDefinition>(*reachDef),
                        std::make_shared<ReachDefinition>(*reachDef));
                }
            }
            _loopsStack.push(node);
        }
    } else {
        _loopsEntrances[node] =
            std::make_pair(std::make_shared<ReachDefinition>(*reachDef),
                           std::make_shared<ReachDefinition>(*reachDef));
        // logDefinition(node, reachDef);
    }
    // Save loop def
    _loops[node] = std::make_shared<ReachDefinition>(*reachDef);
    _reachDef.erase(node);
    // ---------------------------------------
    advance(node, reachDef);
}

void PDG::visitIfInst(IfInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // logDefinition(node, reachDef);
    assert(reachDef->stackSize() >= 1);

    auto condition = reachDef->pop();
    condition->insertPDGEdge(node);
    // ---------------------------------------
    advance(node, getReachDef(node));
}

inline bool PDG::waitPaths(Instruction* inst, bool isLoop) {
    Index inEdgesNum = inst->inEdges(EdgeType::CFG).size();
    if (isLoop) {
        // There are 2 cases:
        // 1) It comes from outside the loop - in this case we need to wait for
        // all external.
        // 2) It comes from inside the loop - in this case we need to wait for
        // all internal
        auto inEdges = inst->inEdges(EdgeType::CFG);
        EdgeSet filterQuery;
        // Case 1
        if (_loopsInsts[inst].count(_lastNode) == 0) {
            filterQuery = Query::filterEdges(inEdges, [&](Edge* e) {
                return _loopsInsts[inst].count(e->src()) == 0;
            });
        } else {  // Case 2
            filterQuery = Query::filterEdges(inEdges, [&](Edge* e) {
                return _loopsInsts[inst].count(e->src()) == 1;
            });
        }
        inEdgesNum = filterQuery.size();
    }
    if (!_loopsStack.empty()) {
        return _loopsInsts[_loopsStack.top()].count(inst) != 1 ||
               _reachDef[inst].size() < inEdgesNum;
    }
    return _reachDef[inst].size() < inEdgesNum;
}

inline std::shared_ptr<ReachDefinition> PDG::getReachDef(Instruction* inst) {
    auto& reachDefs = _reachDef[inst];
    int numReachDefs = reachDefs.size();

    assert(numReachDefs > 0);

    if (inst->instType() == ExprType::Loop) {
        for (auto reachDef : reachDefs) {
            Index nresults = inst->nresults();
            warning(reachDef->stackSize() >= inst->nresults());

            if (reachDef->stackSize() < inst->nresults()) {
                nresults = reachDef->stackSize();
            }

            // gets results
            auto results = reachDef->pop(nresults);
            if (_loops.count(inst) == 1 &&
                reachDef->containsLabel(inst->label())) {
                // pop label
                reachDef->popLabel(inst->label());
            }
            // push label
            reachDef->pushLabel(inst->label());
            // push back the results
            reachDef->push(results);
        }
    } else if (inst->instType() == ExprType::Return) {
        for (auto reachDef : reachDefs) {
            assert(reachDef->stackSize() >= currentFunction->GetNumResults());
            // gets results
            auto results = reachDef->pop(currentFunction->GetNumResults());
            // pop labels
            reachDef->popAllLabel();
            // push back the results
            reachDef->push(results);
        }
    }

    auto reachDef = *reachDefs.begin();
    if (numReachDefs > 1) {
        for (auto it = std::next(reachDefs.begin()); it != reachDefs.end();
             ++it) {
            reachDef->unionDef(**it);
        }

        reachDefs.clear();
        reachDefs.insert(reachDef);
    }

    assert(reachDefs.size() == 1);
    return reachDef;
}

inline void PDG::advance(Instruction* inst,
                         std::shared_ptr<ReachDefinition> resultReachDef) {
    // WARNING: resultReachDef might change when advancing
    auto outEdges = inst->outEdges(EdgeType::CFG);

    if (outEdges.size() >= 1) {
        _reachDef[(*outEdges.begin())->dest()].insert(resultReachDef);
    }

    auto loopsStack = std::make_shared<std::stack<LoopInst*>>(_loopsStack);

    if (outEdges.size() > 1) {
        for (auto it = std::next(outEdges.begin()); it != outEdges.end();
             ++it) {
            auto newReachDef =
                std::make_shared<ReachDefinition>(*resultReachDef);
            _reachDef[(*it)->dest()].insert(newReachDef);
        }
    }

    _reachDef.erase(inst);

    // visit edges
    for (auto e : outEdges) {
        visitCFGEdge(e, loopsStack);
    }
}

void PDG::logDefinition(Node* inst, std::shared_ptr<ReachDefinition> def) {
    if (!cpgOptions.verbose ||
        (!cpgOptions.loopName.empty() && _verboseLoops.count(inst) == 0)) {
        return;
    }
    json instLog;
    instLog["id"] = inst->getId();
    instLog["lastInstId"] = _lastNode->getId();
    instLog["def"] = *def;
    _verbose.push_back(instLog);
}

}  // namespace wasmati
