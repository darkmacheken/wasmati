#include "src/cfg-builder.h"

namespace wasmati {
void CFG::generateCFG(GenerateCPGOptions& options) {
    Index func_index = 0;
    for (auto f : mc.module.funcs) {
        if (!options.funcName.empty() &&
            options.funcName.compare(f->name) != 0) {
            func_index++;
            continue;
        }
        auto isImport = mc.module.IsImport(ExternalKind::Func, Var(func_index));

        if (!isImport) {
            Node* returnFuncNode = ast.returnFunc.at(f);
            Query::NodeSet instNodeQuery = Query::searchBackwards(
                {returnFuncNode},
                [](Node* node) {
                    return node->type() == NodeType::Instructions;
                },
                Query::allEdges, 1);
            assert(instNodeQuery.size() == 1);

            Node* insts = *instNodeQuery.begin();
            auto lastInst = construct(f->exprs, insts, false);
            if (lastInst != nullptr) {
                new CFGEdge(lastInst, ast.returnFunc.at(f));
            }
        }

        func_index++;
    }
}
Node* CFG::construct(const Expr& e, Node* lastInst, bool ifCondition) {
    assert(lastInst != nullptr);
    Node* currentInst = ast.exprNodes.at(&e);

    bool lastIsBrIf = false;
    bool lastIsIf = false;

    if (e.type() != ExprType::Block && e.type() != ExprType::Loop &&
        e.type() != ExprType::If) {
        if (lastInst->type() == NodeType::Instruction &&
            lastInst->instType() == ExprType::BrIf) {
            new CFGEdge(lastInst, currentInst, "false");
            lastIsBrIf = true;
        }
        if (lastInst->type() == NodeType::Instruction &&
            lastInst->instType() == ExprType::If) {
            lastIsIf = true;
            if (ifCondition) {
                new CFGEdge(lastInst, currentInst, "true");
            } else {
                new CFGEdge(lastInst, currentInst, "false");
            }
        }
    }

    switch (e.type()) {
    case ExprType::Br: {
        auto br = cast<BrExpr>(&e);
        if (!lastIsBrIf && !lastIsIf) {
            new CFGEdge(lastInst, currentInst);
        }

        std::string target = br->var.name();
        for (auto p : _blocks) {
            if (target.compare(p.first) == 0) {
                new CFGEdge(currentInst, p.second);
                return nullptr;
            }
        }
        assert(false);
        break;
    }
    case ExprType::BrIf: {
        auto brif = cast<BrIfExpr>(&e);
        if (!lastIsBrIf && !lastIsIf) {
            new CFGEdge(lastInst, currentInst);
        }
        std::string target = brif->var.name();
        for (auto p : _blocks) {
            if (target.compare(p.first) == 0) {
                new CFGEdge(currentInst, p.second, "true");
                break;
            }
        }

        break;
    }
    case ExprType::BrTable: {
        auto brTable = cast<BrTableExpr>(&e);
        if (!lastIsBrIf && !lastIsIf) {
            new CFGEdge(lastInst, currentInst);
        }
        for (Index i = 0; i < brTable->targets.size(); i++) {
            for (auto block : _blocks) {
                if (block.first.compare(brTable->targets[i].name()) == 0) {
                    new CFGEdge(currentInst, block.second, std::to_string(i));
                }
            }
        }

        for (auto block : _blocks) {
            if (block.first.compare(brTable->default_target.name()) == 0) {
                new CFGEdge(currentInst, block.second, "default");
            }
        }
        return nullptr;
        break;
    }
    case ExprType::If: {
        auto ifExpr = cast<IfExpr>(&e);
        if (!lastIsBrIf && !lastIsIf) {
            new CFGEdge(lastInst, currentInst);
        }

        // Visit True Block
        Node* lastInstTrueBlock =
            construct(ifExpr->true_.exprs, currentInst, true);
        if (lastInstTrueBlock != nullptr &&
            lastInst->type() == NodeType::Instruction &&
            !(lastInst->instType() == ExprType::BrIf)) {
            new CFGEdge(lastInstTrueBlock, ast.ifBlocks.at(&ifExpr->true_));
        }

        Node* lastInstFalseBlock = nullptr;
        // Visit False Block if exists
        if (!ifExpr->false_.empty()) {
            // In the Else "block" is the same block of the true
            // so we need to put the true block in the stack in case there is a
            // br
            _blocks.emplace_front(ifExpr->true_.label,
                                  ast.ifBlocks.at(&ifExpr->true_));
            // Visit False Block
            lastInstFalseBlock = construct(ifExpr->false_, currentInst, false);

            if (lastInstFalseBlock != nullptr &&
                lastInst->type() == NodeType::Instruction &&
                !(lastInst->instType() == ExprType::BrIf)) {
                new CFGEdge(lastInstFalseBlock,
                            ast.ifBlocks.at(&ifExpr->true_));
            }
            // Pop block
            _blocks.pop_front();
        }

        if (lastInstTrueBlock == nullptr && lastInstFalseBlock == nullptr) {
            // unreachable from now on
            return nullptr;
        } else {
            return ast.ifBlocks.at(&ifExpr->true_);
        }
    }
    case ExprType::Block: {
        auto block = cast<BlockExpr>(&e);
        _blocks.emplace_front(block->block.label, currentInst);
        lastInst = construct(block->block.exprs, lastInst, ifCondition);
        if (lastInst == nullptr) {
            // if there are CFG edges to this block
            // then there are br and is not unreachable from now on
            // otherwise is unreachable
            if (currentInst->hasInEdgesOf(EdgeType::CFG)) {
                _blocks.pop_front();
                return currentInst;
            }
        }
        _blocks.pop_front();
        if (!lastIsBrIf && !lastIsIf) {
            new CFGEdge(lastInst, currentInst);
        }
        break;
    }
    case ExprType::Loop: {
        auto loop = cast<LoopExpr>(&e);
        if (!lastIsBrIf && !lastIsIf) {
            new CFGEdge(lastInst, currentInst);
        }
        _blocks.emplace_front(loop->block.label, currentInst);
        lastInst = currentInst;
        lastInst = construct(loop->block.exprs, lastInst, ifCondition);
        if (lastInst->type() == NodeType::Instruction &&
            lastInst->instType() == ExprType::Br) {
            lastInst = nullptr;
        }
        _blocks.pop_front();
        return lastInst;
    }
    case ExprType::Unreachable:
        new CFGEdge(lastInst, currentInst);
        new CFGEdge(currentInst, graph.getTrap());
        return nullptr;
    default:
        if (!lastIsBrIf && !lastIsIf) {
            new CFGEdge(lastInst, currentInst);
        }
        break;
    }
    return currentInst;
}

Node* CFG::construct(const ExprList& es, Node* lastInst, bool ifCondition) {
    if (lastInst == nullptr) {
        return nullptr;
    }
    for (auto& expr : es) {
        lastInst = construct(expr, lastInst, ifCondition);
        if (lastInst == nullptr) {
            break;
        }
    }
    return lastInst;
}
}  // namespace wasmati
