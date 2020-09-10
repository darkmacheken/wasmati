#include "src/pdg-builder.h"

namespace wasmati {
void PDG::generatePDG(GenerateCPGOptions& options) {
    graph.getModule()->accept(this);
}

void PDG::visitASTEdge(ASTEdge* e) {
    assert(false);
}

void PDG::visitCFGEdge(CFGEdge* e) {
    if (!e->_label.empty()) {
        // it is true or false label or (br_table)
        new PDGEdge(e);
    }
    e->dest()->accept(this);
}

void PDG::visitPDGEdge(PDGEdge* e) {
    assert(false);
}

void PDG::visitModule(Module* node) {
    for (auto e : node->outEdges(EdgeType::AST)) {
        e->dest()->accept(this);
    }
}

void PDG::visitFunction(Function* node) {
    if (node->isImport()) {
        return;
    }
    currentFunction = node->getFunc();
    auto childrens = Query::children(
        {node}, [](Edge* e) { return e->type() == EdgeType::AST; });
    auto filterInsts = Query::filter(
        childrens, [](Node* n) { return n->type() == NodeType::Instructions; });
    assert(filterInsts.size() == 1);
    // Visit instructions
    (*filterInsts.begin())->accept(this);
}

void PDG::visitFunctionSignature(FunctionSignature* node) {
    assert(false);
}
void PDG::visitParameters(Parameters* node) {
    assert(false);
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

    std::vector<Edge*> outEdges = node->outEdges(EdgeType::CFG);
    assert(outEdges.size() == 1);
    _reachDef[outEdges[0]->dest()].push_back(reachDefs);

    outEdges[0]->accept(this);
}
void PDG::visitLocals(Locals* node) {
    assert(false);
}

void PDG::visitResults(Results* node) {
    assert(false);
}

void PDG::visitElse(Else* node) {
    assert(false);
}

void PDG::visitStart(Start* node) {
    assert(false);
}

void PDG::visitTrap(Trap* node) {
    assert(false);
}

void PDG::visitVarNode(VarNode* node) {
    assert(false);
}

void PDG::visitNopInst(NopInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    // no operation => does nothing
    // ---------------------------------------
    advance(node, getReachDef(node));
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
    assert(reachDef->stackSize() >= 1);

    auto arg = reachDef->pop();
    arg->insertPDGEdge(node);

    // ---------------------------------------
    advance(node, getReachDef(node));
}

void PDG::visitDropInst(DropInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= 1);
    // pop last element of stack
    reachDef->pop();
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitSelectInst(SelectInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
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
    advance(node, getReachDef(node));
}
void PDG::visitMemorySizeInst(MemorySizeInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    // push size of memory to stack
    reachDef->push();
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitMemoryGrowInst(MemoryGrowInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= 1);

    auto n = reachDef->pop();
    // this inst depends on n
    n->insertPDGEdge(node);

    // memory.grow pushes new size of memory if OK or else an error number
    reachDef->push();

    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitConstInst(ConstInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);

    reachDef->push();  // push empty def
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitBinaryInst(BinaryInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= 2);

    auto arg1 = reachDef->pop();
    auto arg2 = reachDef->pop();
    arg1->unionDef(arg2);
    arg1->insertPDGEdge(node);
    arg1->clear(node);
    reachDef->push(arg1);
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitCompareInst(CompareInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= 2);

    auto arg1 = reachDef->pop();
    auto arg2 = reachDef->pop();
    arg1->unionDef(arg2);
    arg1->insertPDGEdge(node);
    arg1->clear(node);
    reachDef->push(arg1);
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitConvertInst(ConvertInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= 1);

    // write dependencies of arg in top of the stack to this inst
    reachDef->peek()->insertPDGEdge(node);

    // following inst using this value depend of the result in this inst
    reachDef->peek()->clear(node);

    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitUnaryInst(UnaryInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= 1);

    auto arg = reachDef->peek();
    // write dependecies
    arg->insertPDGEdge(node);

    // Set dependencies to this inst
    arg->clear(node);

    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitLoadInst(LoadInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= 1);

    // pop index and write dependencies
    reachDef->pop()->insertPDGEdge(node);

    // push a value to the stack
    reachDef->push();

    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitStoreInst(StoreInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= 2);

    auto c = reachDef->pop();
    auto i = reachDef->pop();

    // write dependencies to this inst
    c->insertPDGEdge(node);
    i->insertPDGEdge(node);
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitBrInst(BrInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    // it expects the jump block to pop labels
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitBrIfInst(BrIfInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= 1);

    auto arg = reachDef->pop();
    arg->insertPDGEdge(node);

    // it expects the jumpp block (if true) to pop labels
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitGlobalGetInst(GlobalGetInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);

    reachDef->push(reachDef->getGlobal(node->label()));
    auto varDef = reachDef->peek();
    if (varDef->isEmpty()) {
        // set is empty, thus the var depends on itself
        varDef->insert(node->label(), PDGEdge::Type::Global, node);
    }
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitGlobalSetInst(GlobalSetInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= 1);

    reachDef->insertGlobal(node->label(), reachDef->pop());

    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitLocalGetInst(LocalGetInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);

    reachDef->push(reachDef->getLocal(node->label()));
    auto varDef = reachDef->peek();
    if (varDef->isEmpty()) {
        // set is empty, thus the var depends on itself
        varDef->insert(node->label(), PDGEdge::Type::Local, node);
    }

    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitLocalSetInst(LocalSetInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= 1);

    reachDef->insertLocal(node->label(), reachDef->pop());
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitLocalTeeInst(LocalTeeInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= 1);

    // pop value
    auto arg = reachDef->pop();

    // perform a local.set of value
    reachDef->insertLocal(node->label(), arg);

    // push back value to stack
    reachDef->push(arg);
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitCallInst(CallInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
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
        def->insert(node->label(), PDGEdge::Type::Function, node);
    }

    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitCallIndirectInst(CallIndirectInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
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
        def->insert(node->label(), PDGEdge::Type::Function, node);
    }
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitBeginBlockInst(BeginBlockInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    reachDef->pushLabel(node->label());
    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitBlockInst(BlockInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= node->nresults());
    // gets results
    auto results = reachDef->pop(node->nresults());
    // pop label
    reachDef->popLabel(node->label());
    // push back the results
    reachDef->push(results);

    // ---------------------------------------
    advance(node, getReachDef(node));
}
void PDG::visitLoopInst(LoopInst* node) {
    assert(false);
    // TODO
}

void PDG::visitIfInst(IfInst* node) {
    if (waitPaths(node)) {
        return;
    }
    // ---------------------------------------
    auto reachDef = getReachDef(node);
    assert(reachDef->stackSize() >= 1);

    auto condition = reachDef->pop();
    condition->insertPDGEdge(node);
    // ---------------------------------------
    advance(node, getReachDef(node));
}

inline bool PDG::waitPaths(Instruction* inst) {
    Index inEdgesNum = inst->inEdges(EdgeType::CFG).size();
    return _reachDef[inst].size() != inEdgesNum;
}

inline std::shared_ptr<ReachDefinition> PDG::getReachDef(Instruction* inst) {
    auto reachDefs = _reachDef[inst];
    int numReachDefs = reachDefs.size();

    assert(numReachDefs > 0);
    if (numReachDefs == 1) {
        return reachDefs[0];
    }

    auto reachDef = reachDefs[0];
    for (int i = 1; i < numReachDefs; i++) {
        reachDef->unionDef(*reachDefs[i]);
    }

    reachDefs.clear();
    reachDefs.push_back(reachDef);
    assert(reachDefs.size() == 1);

    return reachDef;
}

inline void PDG::advance(Instruction* inst,
                         std::shared_ptr<ReachDefinition> resultReachDef) {
    auto outEdges = inst->outEdges(EdgeType::CFG);

    if (outEdges.size() >= 1) {
        _reachDef[outEdges[0]->dest()].push_back(resultReachDef);
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

}  // namespace wasmati
