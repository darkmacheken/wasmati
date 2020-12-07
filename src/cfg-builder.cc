#include "cfg-builder.h"

namespace wasmati {
void CFG::generateCFG() {
    Index func_index = 0;
    for (auto f : mc.module.funcs) {
        debug("[DEBUG][CFG][%u/%lu] Function %s\n", func_index,
              mc.module.funcs.size(), f->name.c_str());
        if (!cpgOptions.funcName.empty() &&
            cpgOptions.funcName.compare(f->name) != 0) {
            func_index++;
            continue;
        }
        auto isImport = mc.module.IsImport(ExternalKind::Func, Var(func_index));

        if (!isImport) {
            Node* returnFuncNode = ast.returnFunc.at(f);
            NodeSet instNodeQuery = Query::BFS(
                {returnFuncNode},
                [](Node* node) {
                    return node->type() == NodeType::Instructions;
                },
                Query::ALL_EDGES, 1, true);
            assert(instNodeQuery.size() == 1);

            Node* insts = *instNodeQuery.begin();
            if (f->exprs.empty()) {
                new CFGEdge(insts, returnFuncNode);
                continue;
            }
            new CFGEdge(insts, ast.exprNodes.at(&f->exprs.front()));

            auto unreachable = construct(f->exprs);

            // Connect return
            if (!unreachable) {
                insertEdgeFromLastExpr(f->exprs, returnFuncNode);
            } else {
                continue;
            }
            auto childlessReturn = Query::BFS(
                {insts},
                [](Node* node) {
                    return node->outEdges(EdgeType::CFG).size() == 0 &&
                           node->inEdges(EdgeType::CFG).size() > 0 &&
                           node->type() == NodeType::Instruction &&
                           node->instType() != ExprType::Return;
                },
                Query::AST_EDGES);
            for (Node* node : childlessReturn) {
                new CFGEdge(node, returnFuncNode);
            }
        }

        func_index++;
    }
}
bool CFG::construct(const ExprList& es) {
    for (auto it = es.begin(); it != es.end(); it++) {
        switch (it->type()) {
        case ExprType::Return: {
            return false;
        }
        case ExprType::Unreachable: {
            Node* inst = ast.exprNodes.at(&*it);
            new CFGEdge(inst, graph.getTrap());
            return true;
        }
        case ExprType::Br: {
            auto expr = cast<BrExpr>(&*it);
            Node* inst = ast.exprNodes.at(expr);
            std::string target = expr->var.name();
            new CFGEdge(inst, getBlock(target));
            return true;
        }
        case ExprType::BrIf: {
            auto expr = cast<BrIfExpr>(&*it);
            Node* inst = ast.exprNodes.at(expr);
            std::string target = expr->var.name();
            new CFGEdge(inst, getBlock(target), "true");
            // if it's not the last
            if (&*it != &es.back()) {
                new CFGEdge(inst, ast.exprNodes.at(&*std::next(it)), "false");
            }
            break;
        }
        case ExprType::BrTable: {
            auto expr = cast<BrTableExpr>(&*it);
            Node* inst = ast.exprNodes.at(expr);
            for (Index i = 0; i < expr->targets.size(); i++) {
                const std::string target = expr->targets[i].name();
                new CFGEdge(inst, getBlock(target), std::to_string(i));
            }

            new CFGEdge(inst, getBlock(expr->default_target.name()), "default");
            break;
        }
        case ExprType::Block: {
            auto expr = cast<BlockExpr>(&*it);
            Node* beginBlockInst = ast.exprNodes.at(expr);
            Node* blockInst = beginBlockInst->block();

            // Push Label
            _blocks.emplace_front(expr->block.label, blockInst);

            // In case the block is empty
            if (expr->block.exprs.empty()) {
                new CFGEdge(beginBlockInst, blockInst);
            } else {
                auto& firstExpr = expr->block.exprs.front();
                new CFGEdge(beginBlockInst, ast.exprNodes.at(&firstExpr));
            }

            // construct
            auto unreachable = construct(expr->block.exprs);

            if (!unreachable) {
                insertEdgeFromLastExpr(expr->block.exprs, blockInst);
            }

            if (!blockInst->hasInEdgesOf(EdgeType::CFG)) {
                _blocks.pop_front();
                return false;
            }

            // if it's not the last
            if (&*it != &es.back()) {
                new CFGEdge(blockInst, ast.exprNodes.at(&*std::next(it)));
            }

            // Pop label
            _blocks.pop_front();
            break;
        }
        case ExprType::Loop: {
            auto expr = cast<LoopExpr>(&*it);
            Node* inst = ast.exprNodes.at(expr);

            // Push Label
            _blocks.emplace_front(expr->block.label, inst);

            // In case the loop is empty
            if (expr->block.exprs.empty()) {
                // if it's not the last
                if (&*it != &es.back()) {
                    new CFGEdge(inst, ast.exprNodes.at(&*std::next(it)));
                }
            } else {
                auto& firstExpr = expr->block.exprs.front();
                new CFGEdge(inst, ast.exprNodes.at(&firstExpr));
            }

            // construct
            construct(expr->block.exprs);

            // if it's not the last
            if (&*it != &es.back()) {
                insertEdgeFromLastExpr(expr->block.exprs,
                                       ast.exprNodes.at(&*std::next(it)));
            }

            // Pop label
            _blocks.pop_front();
            break;
        }
        case ExprType::If: {
            auto expr = cast<IfExpr>(&*it);
            Node* inst = ast.exprNodes.at(expr);

            // True Condition
            auto trueBeginInst = ast.ifBlocks.at(&expr->true_);
            auto trueBlockInst = trueBeginInst->block();
            new CFGEdge(inst, trueBeginInst, "true");

            _blocks.emplace_front(expr->true_.label, trueBlockInst);
            auto unreachable = construct(expr->true_.exprs);
            if (!expr->true_.exprs.empty()) {
                new CFGEdge(trueBeginInst,
                            ast.exprNodes.at(&expr->true_.exprs.front()));
            }
            if (!unreachable) {
                insertEdgeFromLastExpr(expr->true_.exprs, trueBlockInst);
            }
            _blocks.pop_front();

            // False Condition
            if (!expr->false_.empty()) {
                // In the Else "block" is the same block of the true
                // so we need to put the true block in the stack in case there
                // is a br
                _blocks.emplace_front(expr->true_.label, trueBlockInst);
                // Visit False Block
                auto fUnreachable = construct(expr->false_);
                auto falseBeginInst = new BeginBlockInst(
                    expr->true_.label, static_cast<BlockInst*>(trueBlockInst));
                graph.insertNode(falseBeginInst);
                new CFGEdge(inst, falseBeginInst, "false");
                new CFGEdge(falseBeginInst,
                            ast.exprNodes.at(&expr->false_.front()));
                if (!fUnreachable) {
                    insertEdgeFromLastExpr(expr->false_, trueBlockInst);
                }
                // Pop block
                _blocks.pop_front();
                if (unreachable && fUnreachable) {
                    return true;
                }
            } else {
                auto falseBeginInst = new BeginBlockInst(
                    expr->true_.label, static_cast<BlockInst*>(trueBlockInst));
                graph.insertNode(falseBeginInst);
                new CFGEdge(inst, falseBeginInst, "false");
                new CFGEdge(falseBeginInst, trueBlockInst);
            }

            // if it's not the last
            if (&*it != &es.back()) {
                if (trueBlockInst->hasInEdgesOf(EdgeType::CFG)) {
                    new CFGEdge(trueBlockInst,
                                ast.exprNodes.at(&*std::next(it)));
                }
            }
            break;
        }
        case ExprType::Call: {
            Node* inst = ast.exprNodes.at(&*it);
            // if it's not the last
            if (&*it != &es.back()) {
                new CFGEdge(inst, ast.exprNodes.at(&*std::next(it)));
            }

            // query function by name
            auto query = Query::functions(
                [&](Node* node) { return node->name() == inst->label(); });

            if (query.size() == 1) {
                Node* func = *query.begin();
                new CGEdge(inst, func);

                // insert PG
                auto funcParams = Query::parameters({func});
                auto callParams = Query::children({inst}, Query::AST_EDGES);

                for (auto itf = funcParams.begin(), itc = callParams.begin();
                     itf != funcParams.end(); ++itf, ++itc) {
                    new PGEdge(*itc, *itf);
                }
            }
            break;
        }
        case ExprType::CallIndirect: {
            Node* inst = ast.exprNodes.at(&*it);
            // if it's not the last
            if (&*it != &es.back()) {
                new CFGEdge(inst, ast.exprNodes.at(&*std::next(it)));
            }

            // get all functions possible
            auto expr = cast<CallIndirectExpr>(&*it);
            std::set<std::string> funcs;
            for (auto elems : mc.module.elem_segments) {
                if (expr->table.name() == elems->table_var.name()) {
                    for (auto elem : elems->elem_exprs) {
                        assert(elem.kind == ElemExprKind::RefFunc);
                        auto func = mc.module.GetFunc(elem.var);
                        if (expr->decl.sig == func->decl.sig) {
                            funcs.insert(func->name);
                        }
                    }
                }
            }
            // query possible functions
            auto query = Query::functions(
                [&](Node* node) { return funcs.count(node->name()) == 1; });

            for (Node* func : query) {
                new CGEdge(inst, func);

                // insert PG
                auto funcParams = Query::parameters({func});
                auto callParams = Query::children({inst}, Query::AST_EDGES);

                for (auto itf = funcParams.begin(), itc = callParams.begin();
                     itf != funcParams.end(); ++itf, ++itc) {
                    new PGEdge(*itc, *itf);
                }
            }

            break;
        }
        default:
            Node* inst = ast.exprNodes.at(&*it);
            // if it's not the last
            if (&*it != &es.back()) {
                new CFGEdge(inst, ast.exprNodes.at(&*std::next(it)));
            }
        }
    }
    return false;
}
void CFG::insertEdgeFromLastExpr(const wabt::ExprList& es,
                                 wasmati::Node* blockInst) {
    // Edge cases for last instruction:
    // In case of a Br || BrTable || Loop || return || unreachable: Do nothing
    // In case of a BrIf: Handle false case to this block.
    // In case of Block: Point the BlockInst node to this block
    // In case of an If: If has else: point inner blockInst to this
    // block
    //      If not: handle false case to this block
    // Other cases: point inst to this block
    auto& lastExpr = es.back();
    if (lastExpr.type() == ExprType::Br ||
        lastExpr.type() == ExprType::BrTable ||
        lastExpr.type() == ExprType::Return ||
        lastExpr.type() == ExprType::Unreachable) {
        // do nothing
    } else if (lastExpr.type() == ExprType::Loop) {
        auto loop = cast<LoopExpr>(&lastExpr);
        insertEdgeFromLastExpr(loop->block.exprs, blockInst);
    } else if (lastExpr.type() == ExprType::BrIf) {
        new CFGEdge(ast.exprNodes.at(&lastExpr), blockInst, "false");
    } else if (lastExpr.type() == ExprType::Block) {
        auto lastBlockInst = ast.exprNodes.at(&lastExpr)->block();
        new CFGEdge(lastBlockInst, blockInst);
    } else if (lastExpr.type() == ExprType::If) {
        auto ifExpr = cast<IfExpr>(&lastExpr);
        new CFGEdge(ast.ifBlocks.at(&ifExpr->true_)->block(), blockInst);
    } else {
        new CFGEdge(ast.exprNodes.at(&lastExpr), blockInst);
    }
}
Node* CFG::getBlock(const std::string& target) {
    for (auto p : _blocks) {
        if (target.compare(p.first) == 0) {
            return p.second;
        }
    }
    assert(false);
    return nullptr;
}
}  // namespace wasmati
