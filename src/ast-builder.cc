#include "ast-builder.h"
namespace wasmati {
void AST::generateAST() {
    Module* m;
    if (mc.module.name.empty()) {
        m = new Module();
    } else {
        m = new Module(mc.module.name);
    }
    graph.insertNode(m);
    graph.setModule(m);

    // Code
    Index func_index = 0;
    for (auto f : mc.module.funcs) {
        debug("[DEBUG][AST][%u/%lu] Function %s\n", func_index,
              mc.module.funcs.size(), f->name.c_str());
        if (!cpgOptions.funcName.empty() &&
            cpgOptions.funcName.compare(f->name) != 0) {
            func_index++;
            continue;
        }
        bool isImport = mc.module.IsImport(ExternalKind::Func, Var(func_index));
        bool isExport = false;
        // check if isEXport
        for (Export* exp : mc.module.exports) {
            if (exp->kind == ExternalKind::Func && exp->var.is_name() &&
                exp->var.name() == f->name) {
                isExport = true;
                break;
            }
        }
        // Function
        Function* func = new Function(f, func_index, isImport, isExport);
        graph.insertNode(func);
        new ASTEdge(m, func);
        funcs[f] = func;
        funcsByName[f->name] = func;
        // Function Signature
        FunctionSignature* fsign = new FunctionSignature();
        graph.insertNode(fsign);
        new ASTEdge(func, fsign);
        std::vector<std::string> localsNames;
        getLocalsNames(f, localsNames);
        //// Parameters
        Index numParameters = f->GetNumParams();
        if (numParameters > 0) {
            Parameters* parameters = new Parameters();
            graph.insertNode(parameters);
            new ASTEdge(fsign, parameters);

            for (Index i = 0; i < numParameters; i++) {
                VarNode* tnode =
                    new VarNode(f->GetParamType(i), i, localsNames[i]);
                graph.insertNode(tnode);
                new ASTEdge(parameters, tnode);
            }
        }
        //// Locals
        Index numLocals = f->GetNumLocals();
        if (numLocals > 0) {
            Locals* locals = new Locals();
            graph.insertNode(locals);
            new ASTEdge(fsign, locals);

            for (Index i = numParameters; i < f->GetNumParamsAndLocals(); i++) {
                VarNode* tnode =
                    new VarNode(f->GetLocalType(i), i, localsNames[i]);
                graph.insertNode(tnode);
                new ASTEdge(locals, tnode);
            }
        }
        //// Results
        Index numResults = f->GetNumResults();
        if (numResults > 0) {
            Results* results = new Results();
            graph.insertNode(results);
            new ASTEdge(fsign, results);

            for (Index i = 0; i < numResults; i++) {
                VarNode* tnode = new VarNode(f->GetResultType(i), i);
                graph.insertNode(tnode);
                new ASTEdge(results, tnode);
            }
        }

        if (!isImport) {
            // Instructions
            Instructions* inst = new Instructions();
            graph.insertNode(inst);
            new ASTEdge(func, inst);

            construct(f->exprs, f->GetNumResults(), inst, f);
        }
        func_index++;
    }
}

void AST::getLocalsNames(Func* f, std::vector<std::string>& names) const {
    Index size = f->GetNumParamsAndLocals();
    names.reserve(size);
    for (Index i = 0; i < size; i++) {
        names.push_back("");
    }
    for (auto local : f->bindings) {
        names[local.second.index] = local.first;
    }
}

void AST::construct(const Expr& e,
                    std::vector<Node*>& expStack,
                    std::vector<Node*>& expList) {
    auto arity = mc.GetExprArity(e);
    assert(expStack.size() >= arity.nargs);
    assert(arity.nreturns <= 1);
    Node* node;
    switch (e.type()) {
    // Base Instruction
    case ExprType::Nop:
        node = new NopInst(e.loc);
        break;
    case ExprType::Unreachable:
        node = new UnreachableInst(e.loc);
        break;
    case ExprType::Return:
        arity.unreachable = false;
        node = new ReturnInst(e.loc);
        break;
    case ExprType::BrTable:
        node = new BrTableInst(e.loc);
        break;
    case ExprType::Drop:
        node = new DropInst(e.loc);
        break;
    case ExprType::Select:
        node = new SelectInst(e.loc);
        break;
    case ExprType::MemorySize:
        node = new MemorySizeInst(e.loc);
        break;
    case ExprType::MemoryGrow:
        node = new MemoryGrowInst(e.loc);
        break;
    // Const
    case ExprType::Const:
        node = new ConstInst(cast<ConstExpr>(&e));
        break;
    // Opcode
    case ExprType::Binary: {
        auto expr = cast<BinaryExpr>(&e);
        node = new BinaryInst(expr->opcode, expr->loc);
        break;
    }
    case ExprType::Compare: {
        auto expr = cast<CompareExpr>(&e);
        node = new CompareInst(expr->opcode, expr->loc);
        break;
    }
    case ExprType::Convert: {
        auto expr = cast<ConvertExpr>(&e);
        node = new ConvertInst(expr->opcode, expr->loc);
        break;
    }
    case ExprType::Unary: {
        auto expr = cast<UnaryExpr>(&e);
        node = new UnaryInst(expr->opcode, expr->loc);
        break;
    }
    // LoadStore
    case ExprType::Load: {
        auto expr = cast<LoadExpr>(&e);
        node = new LoadInst(expr->opcode, expr->offset, expr->loc);
        break;
    }
    case ExprType::Store: {
        auto expr = cast<StoreExpr>(&e);
        node = new StoreInst(expr->opcode, expr->offset, expr->loc);
        break;
    }
    // LabeledInst
    case ExprType::Br: {
        arity.nargs = 0;
        arity.nreturns = 0;
        auto expr = cast<BrExpr>(&e);
        node = new BrInst(expr->var.name(), expr->loc);
        break;
    }
    case ExprType::BrIf: {
        arity.nargs = 1;
        arity.nreturns = 0;
        auto expr = cast<BrIfExpr>(&e);
        node = new BrIfInst(expr->var.name(), expr->loc);
        break;
    }
    case ExprType::LocalGet: {
        auto expr = cast<LocalGetExpr>(&e);
        node = new LocalGetInst(expr->var.name(), expr->loc);
        break;
    }
    case ExprType::LocalSet: {
        auto expr = cast<LocalSetExpr>(&e);
        node = new LocalSetInst(expr->var.name(), expr->loc);
        break;
    }
    case ExprType::GlobalGet: {
        auto expr = cast<GlobalGetExpr>(&e);
        node = new GlobalGetInst(expr->var.name(), expr->loc);
        break;
    }
    case ExprType::GlobalSet: {
        auto expr = cast<GlobalSetExpr>(&e);
        node = new GlobalSetInst(expr->var.name(), expr->loc);
        break;
    }
    case ExprType::LocalTee: {
        auto expr = cast<LocalTeeExpr>(&e);
        node = new LocalTeeInst(expr->var.name(), expr->loc);
        break;
    }
        // Call Base
    case ExprType::Call:
        node = new CallInst(cast<CallExpr>(&e), e.loc, arity.nargs,
                            arity.nreturns);
        break;
    case ExprType::CallIndirect:
        node = new CallIndirectInst(cast<CallIndirectExpr>(&e), e.loc,
                                    arity.nargs, arity.nreturns);
        break;
        // Block Base
    case ExprType::Block: {
        auto block = cast<BlockExpr>(&e);
        node = new BlockInst(block->block.label,
                             block->block.decl.GetNumResults(), block->loc);
        mc.BeginBlock(LabelType::Block, block->block);
        construct(block->block.exprs, block->block.decl.GetNumResults(), node);
        mc.EndBlock();
        auto beginBlock = new BeginBlockInst(block->block.label,
                                             static_cast<BlockInst*>(node));
        graph.insertNode(beginBlock);
        exprNodes[&e] = beginBlock;
        break;
    }
    case ExprType::Loop: {
        auto loop = cast<LoopExpr>(&e);
        node = new LoopInst(loop->block.label, loop->block.decl.GetNumResults(),
                            loop->loc);
        mc.BeginBlock(LabelType::Loop, loop->block);
        construct(loop->block.exprs, loop->block.decl.GetNumResults(), node);
        mc.EndBlock();
        break;
    }
    case ExprType::If: {
        auto ife = cast<IfExpr>(&e);
        node = new IfInst(ife);
        graph.insertNode(node);
        exprNodes[&e] = node;

        // Condition
        auto condition = expStack.back();
        expStack.pop_back();
        new ASTEdge(node, condition);

        mc.BeginBlock(LabelType::Block, ife->true_);
        BlockInst* trueBlock = new BlockInst(ife->true_);
        graph.insertNode(trueBlock);
        Node* beginTrueBlock = new BeginBlockInst(ife->true_.label, trueBlock);
        graph.insertNode(beginTrueBlock);
        new ASTEdge(node, trueBlock);
        ifBlocks[&ife->true_] = beginTrueBlock;
        construct(ife->true_.exprs, ife->true_.decl.GetNumResults(), trueBlock);
        if (!ife->false_.empty()) {
            Node* elseBlock = new Else();
            graph.insertNode(elseBlock);
            new ASTEdge(node, elseBlock);
            construct(ife->false_, ife->true_.decl.GetNumResults(), elseBlock);
        }
        mc.EndBlock();

        if (arity.nreturns == 0) {
            expList.push_back(node);
        } else {
            expStack.push_back(node);
        }

        return;
    }
    default:
        assert(false);
        return;
    }
    graph.insertNode(node);
    if (e.type() != ExprType::Block) {
        if (e.type() == ExprType::Return) {
            exprNodes[&e] = returnFunc[currentFunction];
        } else {
            exprNodes[&e] = node;
        }
    }
    // Args
    for (Index i = 0; i < arity.nargs; i++) {
        auto arg = expStack.back();
        expStack.pop_back();
        new ASTEdge(node, arg);
    }
    // Return
    if (arity.nreturns == 0 || arity.unreachable) {
        expList.push_back(node);
    } else {
        expStack.push_back(node);
    }
}

void AST::construct(const ExprList& es,
                    Index nresults,
                    Node* holder,
                    Func* function) {
    assert(holder != nullptr);
    std::vector<Node*> expStack;
    std::vector<Node*> expList;

    if (function != nullptr) {
        mc.BeginFunc(*function);
        currentFunction = function;
        auto ret = new ReturnInst();
        returnFunc[function] = ret;
        graph.insertNode(ret);
    }

    for (auto& e : es) {
        construct(e, expStack, expList);
    }

    for (auto& node : expList) {
        new ASTEdge(holder, node);
    }

    if (es.rbegin() != es.rend() &&
        es.rbegin()->type() == ExprType::Unreachable &&
        expStack.size() < nresults) {
        while (expStack.size() > 0) {
            new ASTEdge(holder, expStack.back());
            expStack.pop_back();
        }
        if (function != nullptr) {
            auto ret = returnFunc[function];
            new ASTEdge(holder, ret);
            mc.EndFunc();
            currentFunction = nullptr;
        }
        return;
    }
    warning(expStack.size() >= nresults);
    if (function != nullptr) {
        auto ret = returnFunc[function];
        if (nresults == 1) {
            new ASTEdge(ret, expStack.back());
            expStack.pop_back();
        }
        while (expStack.size() > 0) {
            new ASTEdge(holder, expStack.back());
            expStack.pop_back();
        }
        new ASTEdge(holder, ret);
        mc.EndFunc();
        currentFunction = nullptr;
    } else {
        for (auto node : expStack) {
            new ASTEdge(holder, node);
        }
    }
}
}  // namespace wasmati
