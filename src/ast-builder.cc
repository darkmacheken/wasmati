#include "ast-builder.h"
namespace wasmati {
void AST::generateAST(GenerateCPGOptions& options) {
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
        if (!options.funcName.empty() &&
            options.funcName.compare(f->name) != 0) {
            func_index++;
            continue;
        }
        auto test = f->name == "$bof";
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
                    new VarNode(f->GetParamType(i), localsNames[i]);
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
                    new VarNode(f->GetLocalType(i), localsNames[i]);
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
                VarNode* tnode = new VarNode(f->GetResultType(i));
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
    case ExprType::Binary:
        node = new BinaryInst(cast<BinaryExpr>(&e));
        break;
    case ExprType::Compare:
        node = new CompareInst(cast<CompareExpr>(&e));
        break;
    case ExprType::Convert:
        node = new ConvertInst(cast<ConvertExpr>(&e));
        break;
    case ExprType::Unary:
        node = new UnaryInst(cast<UnaryExpr>(&e));
        break;
        // LoadStore
    case ExprType::Load:
        node = new LoadInst(cast<LoadExpr>(&e));
        break;
    case ExprType::Store:
        node = new StoreInst(cast<StoreExpr>(&e));
        break;
        // LabeledInst
    case ExprType::Br:
        arity.nargs = 0;
        arity.nreturns = 0;
        node = new BrInst(cast<BrExpr>(&e));
        break;
    case ExprType::BrIf:
        arity.nargs = 1;
        arity.nreturns = 0;
        node = new BrIfInst(cast<BrIfExpr>(&e));
        break;
    case ExprType::LocalGet:
        node = new LocalGetInst(cast<LocalGetExpr>(&e));
        break;
    case ExprType::LocalSet:
        node = new LocalSetInst(cast<LocalSetExpr>(&e));
        break;
    case ExprType::GlobalGet:
        node = new GlobalGetInst(cast<GlobalGetExpr>(&e));
        break;
    case ExprType::GlobalSet:
        node = new GlobalSetInst(cast<GlobalSetExpr>(&e));
        break;
    case ExprType::LocalTee:
        node = new LocalTeeInst(cast<LocalTeeExpr>(&e));
        break;
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
        node = new BlockInst(block);
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
        node = new LoopInst(loop);
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
    assert(expStack.size() >= nresults);
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
