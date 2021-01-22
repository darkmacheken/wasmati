#ifndef WASMATI_EVALUATOR_H
#define WASMATI_EVALUATOR_H
#define ASSERT_EXPR_NOT_NULL                                            \
    if (_resultVisit == nullptr) {                                      \
        std::cerr << "Unknown error evaluating line " << node->lineno() \
                  << std::endl;                                         \
        exit(1);                                                        \
    }

#define ASSERT_EXPR_TYPE(TYPE)                                             \
    ASSERT_EXPR_NOT_NULL                                                   \
    if (_resultVisit->type() != TYPE) {                                    \
        std::cerr << "Expected " << EXPR_TYPE_MAP.at(TYPE) << " but got "  \
                  << EXPR_TYPE_MAP.at(_resultVisit->type()) << " in line " \
                  << node->lineno() << std::endl;                          \
        exit(1);                                                           \
    }

#define ASSERT_EXPR_NOT_NULL_(EXPR)                                     \
    if (EXPR == nullptr) {                                              \
        std::cerr << "Unknown error evaluating line " << EXPR->lineno() \
                  << std::endl;                                         \
        exit(1);                                                        \
    }

#define ASSERT_EXPR_TYPE_(EXPR, TYPE)                                     \
    ASSERT_EXPR_NOT_NULL_(EXPR)                                           \
    if (EXPR->type() != TYPE) {                                           \
        std::cerr << "Expected " << EXPR_TYPE_MAP.at(TYPE) << " but got " \
                  << EXPR_TYPE_MAP.at(EXPR->type()) << " in line "        \
                  << EXPR->lineno() << std::endl;                         \
        exit(1);                                                          \
    }

#define ASSERT_NUM_ARGS_(NARGS)                                    \
    if (args->size() < NARGS) {                                    \
        std::cerr << "Expected atleast " << NARGS << " but got "   \
                  << args->size() << " in line " << args->lineno() \
                  << std::endl;                                    \
        exit(1);                                                   \
    }

#include <iostream>
#include "src/interpreter/nodes.h"
#include "src/interpreter/symbol-table.h"
#include "src/query.h"

namespace wasmati {
extern const std::map<ExprType, std::string> EXPR_TYPE_MAP;
extern std::map<
    std::string,
    std::function<std::shared_ptr<LiteralNode>(int, std::shared_ptr<ListNode>)>>
    functionsMap;

class Printer : public VisitorTemplate<std::string> {
public:
    std::string toString() { return _resultVisit; }
    void visitSequenceNode(SequenceNode* node) {}
    void visitBlockNode(BlockNode* node) {}

    void visitIdentifierNode(IdentifierNode* node) {}

    void visitForeach(Foreach* node) {}

    void visitIfNode(IfNode* node) {}

    void visitIfElseNode(IfElseNode* node) {}

    void visitAssignNode(AssignNode* node) {}

    std::string visitRValueNode(RValueNode* node) { return ""; }
    std::string visitFunctionCall(FunctionCall* node) { return ""; }
    std::string visitRangeExprNode(RangeExprNode* node) { return ""; }
    std::string visitAtNode(AtNode* node) { return ""; }
    std::string visitIntNode(IntNode* node) {
        return std::to_string(node->value());
    }
    std::string visitFloatNode(FloatNode* node) {
        return std::to_string(node->value());
    }
    std::string visitStringNode(StringNode* node) { return node->value(); }
    std::string visitBoolNode(BoolNode* node) {
        return node->value() ? "true" : "false";
    }
    std::string visitListNode(ListNode* node) {
        std::string res = "[";
        for (auto lit : node->nodes()) {
            lit->accept(this);
            res += _resultVisit + ",";
        }
        res += "]";
        return res;
    }

    std::string visitNilNode(NilNode* node) { return "nil"; }
    std::string visitNodePointer(NodePointer* node) {
        return "[Node::" + NODE_TYPE_MAP.at(node->value()->type()) + "_" +
               std::to_string(node->value()->id()) + "]";
    }
    std::string visitEdgePointer(EdgePointer* node) {
        return "[EDGE::" + EDGE_TYPES_MAP.at(node->value()->type()) + "_" +
               std::to_string(node->value()->src()->id()) + "->" +
               std::to_string(node->value()->dest()->id()) + "]";
    }
    std::string visitEqualNode(EqualNode* node) { return ""; }
    std::string visitNotEqualNode(NotEqualNode* node) { return ""; }
    std::string visitAndNode(AndNode* node) { return ""; }
    std::string visitOrNode(OrNode* node) { return ""; }
    std::string visitInNode(InNode* node) { return ""; }
    std::string visitNotNode(NotNode* node) { return ""; }
    std::string visitSequenceExprNode(SequenceExprNode* node) { return ""; }
    std::string visitSequenceLiteralNode(SequenceLiteralNode* node) {
        return "";
    }
};

class Evaluator : public VisitorTemplate<std::shared_ptr<LiteralNode>> {
    SymbolTable _symTab;
    std::string _lookUp;

public:
    std::string resultToString() {
        Printer printer;
        _resultVisit->accept(&printer);
        return printer.toString();
    }

    void visitSequenceNode(SequenceNode* node) {
        for (auto basicNode : node->nodes()) {
            basicNode->accept(this);
        }
    }
    void visitBlockNode(BlockNode* node) {
        _symTab.push();
        node->statements()->accept(this);
        _symTab.pop();
    }

    void visitIdentifierNode(IdentifierNode* node) { _lookUp = node->name(); }

    void visitForeach(Foreach* node) {
        node->expr()->accept(this);
        ASSERT_EXPR_TYPE(ExprType::List);
        auto list = std::dynamic_pointer_cast<ListNode>(_resultVisit);
        _symTab.push();
        _symTab.insert(node->identifier(),
                       std::make_shared<NilNode>(node->lineno()));
        for (auto lit : list->value()->nodes()) {
            _symTab.replace_local(node->identifier(),
                                  std::shared_ptr<LiteralNode>(lit));
            node->insts()->accept(this);
        }
        _symTab.pop();
    }

    void visitIfNode(IfNode* node) {
        node->condition()->accept(this);
        ASSERT_EXPR_TYPE(ExprType::Bool);
        auto cond = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        if (cond->value()) {
            node->block()->accept(this);
        }
    }

    void visitIfElseNode(IfElseNode* node) {
        node->condition()->accept(this);
        ASSERT_EXPR_TYPE(ExprType::Bool);
        auto cond = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        if (cond->value()) {
            node->thenblock()->accept(this);
        } else {
            node->elseblock()->accept(this);
        }
    }

    void visitAssignNode(AssignNode* node) {
        node->rvalue()->accept(this);
        ASSERT_EXPR_NOT_NULL;
        node->lvalue()->accept(this);
        if (!_symTab.replace(_lookUp, _resultVisit)) {
            _symTab.insert(_lookUp, _resultVisit);
        }
    }

    std::shared_ptr<LiteralNode> visitRValueNode(RValueNode* node) {
        node->lvalue()->accept(this);
        auto lit = _symTab.find(_lookUp);
        if (_symTab.find(_lookUp) == nullptr) {
            std::cerr << "Unbounded variable " << _lookUp << " in line "
                      << node->lineno() << std::endl;
            return std::make_shared<NilNode>(node->lineno());
        }
        return lit;
    }
    std::shared_ptr<LiteralNode> visitFunctionCall(FunctionCall* node) {
        if (functionsMap.count(node->identifier()) == 0) {
            std::cerr << "Unbounded function " << node->identifier()
                      << " in line " << node->lineno() << std::endl;
            return std::make_shared<NilNode>(node->lineno());
        }
        node->parameters()->accept(this);
        ASSERT_EXPR_TYPE(ExprType::List);
        auto list = std::dynamic_pointer_cast<ListNode>(_resultVisit);
        return functionsMap[node->identifier()](node->lineno(), list);
    }
    std::shared_ptr<LiteralNode> visitRangeExprNode(RangeExprNode* node) {
        node->domain()->accept(this);
        ASSERT_EXPR_TYPE(ExprType::List);
        auto list = std::dynamic_pointer_cast<ListNode>(_resultVisit);
        auto result = std::make_shared<SequenceLiteralNode>(node->lineno());

        _symTab.push();
        _symTab.insert(node->var(), std::make_shared<NilNode>(node->lineno()));
        for (auto lit : list->value()->nodes()) {
            _symTab.replace(node->var(), lit);
            node->predicate()->accept(this);
            ASSERT_EXPR_TYPE(ExprType::Bool);
            auto pred = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
            if (pred->value()) {
                result->insert(lit);
            }
        }
        _symTab.pop();
        return std::make_shared<ListNode>(node->lineno(), result);
    }
    std::shared_ptr<LiteralNode> visitAtNode(AtNode* node) {
        node->expr()->accept(this);
        ASSERT_EXPR_TYPE(ExprType::List);
        auto list = std::dynamic_pointer_cast<ListNode>(_resultVisit);

        node->index()->accept(this);
        ASSERT_EXPR_TYPE(ExprType::Int);
        auto index = std::dynamic_pointer_cast<IntNode>(_resultVisit);

        if (static_cast<size_t>(index->value()) >= list->size()) {
            std::cout << "Index out of bounds in line " << node->lineno()
                      << std::endl;
            return std::make_shared<NilNode>(node->lineno(), node);
        }
        return list->node(index->value());
    }
    std::shared_ptr<LiteralNode> visitIntNode(IntNode* node) {
        return std::make_shared<IntNode>(node->lineno(), node);
    }
    std::shared_ptr<LiteralNode> visitFloatNode(FloatNode* node) {
        return std::make_shared<FloatNode>(node->lineno(), node);
    }
    std::shared_ptr<LiteralNode> visitStringNode(StringNode* node) {
        return std::make_shared<StringNode>(node->lineno(), node);
    }
    std::shared_ptr<LiteralNode> visitBoolNode(BoolNode* node) {
        return std::make_shared<BoolNode>(node->lineno(), node);
    }
    std::shared_ptr<LiteralNode> visitListNode(ListNode* node) {
        return std::make_shared<ListNode>(node->lineno(), node);
    }
    std::shared_ptr<LiteralNode> visitNilNode(NilNode* node) {
        return std::make_shared<NilNode>(node->lineno(), node);
    }
    std::shared_ptr<LiteralNode> visitNodePointer(NodePointer* node) {
        return std::make_shared<NodePointer>(node->lineno(), node);
    }
    std::shared_ptr<LiteralNode> visitEdgePointer(EdgePointer* node) {
        return std::make_shared<EdgePointer>(node->lineno(), node);
    }
    std::shared_ptr<LiteralNode> visitEqualNode(EqualNode* node) {
        node->left()->accept(this);
        ASSERT_EXPR_NOT_NULL;
        auto left = _resultVisit;
        node->right()->accept(this);
        ASSERT_EXPR_NOT_NULL;
        auto right = _resultVisit;
        return std::make_shared<BoolNode>(node->lineno(), left->equals(right));
    }
    std::shared_ptr<LiteralNode> visitNotEqualNode(NotEqualNode* node) {
        node->left()->accept(this);
        ASSERT_EXPR_NOT_NULL;
        auto left = _resultVisit;
        node->right()->accept(this);
        ASSERT_EXPR_NOT_NULL;
        auto right = _resultVisit;
        return std::make_shared<BoolNode>(node->lineno(), !left->equals(right));
    }
    std::shared_ptr<LiteralNode> visitAndNode(AndNode* node) {
        node->left()->accept(this);
        ASSERT_EXPR_TYPE(ExprType::Bool);
        auto left = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        if (!left->value()) {
            return std::make_shared<BoolNode>(node->lineno(), false);
        }
        node->right()->accept(this);
        ASSERT_EXPR_TYPE(ExprType::Bool);
        auto right = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        return std::make_shared<BoolNode>(node->lineno(),
                                          left->value() && right->value());
    }
    std::shared_ptr<LiteralNode> visitOrNode(OrNode* node) {
        node->left()->accept(this);
        ASSERT_EXPR_TYPE(ExprType::Bool);
        auto left = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        node->right()->accept(this);
        ASSERT_EXPR_TYPE(ExprType::Bool);
        auto right = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        return std::make_shared<BoolNode>(node->lineno(),
                                          left->value() || right->value());
    }
    std::shared_ptr<LiteralNode> visitInNode(InNode* node) {
        node->left()->accept(this);
        ASSERT_EXPR_NOT_NULL;
        auto left = _resultVisit;
        node->right()->accept(this);
        ASSERT_EXPR_TYPE(ExprType::List);
        auto right = std::dynamic_pointer_cast<ListNode>(_resultVisit);

        for (auto lit : right->value()->nodes()) {
            if (left->equals(lit)) {
                return std::make_shared<BoolNode>(node->lineno(), true);
            }
        }

        return std::make_shared<BoolNode>(node->lineno(), false);
    }
    std::shared_ptr<LiteralNode> visitNotNode(NotNode* node) {
        node->argument()->accept(this);
        ASSERT_EXPR_TYPE(ExprType::Bool);
        auto arg = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        return std::make_shared<BoolNode>(node->lineno(), !arg->value());
    }
    std::shared_ptr<LiteralNode> visitSequenceExprNode(SequenceExprNode* node) {
        auto seq = std::make_shared<SequenceLiteralNode>(node->lineno());
        for (auto expr : node->nodes()) {
            expr->accept(this);
            ASSERT_EXPR_NOT_NULL;
            seq->insert(_resultVisit);
        }
        return std::make_shared<ListNode>(node->lineno(), seq);
    }
    std::shared_ptr<LiteralNode> visitSequenceLiteralNode(
        SequenceLiteralNode* node) {
        return std::make_shared<ListNode>(
            node->lineno(), std::shared_ptr<SequenceLiteralNode>(node));
    }
};

struct Functions {
    static std::shared_ptr<ListNode> nodeSetToList(int lineno, NodeSet& nodes) {
        auto list = Query::map<std::shared_ptr<LiteralNode>>(
            nodes,
            [](Node* node) { return std::make_shared<NodePointer>(0, node); });
        auto seq = std::make_shared<SequenceLiteralNode>(lineno, list);
        return std::make_shared<ListNode>(lineno, seq);
    }

    static std::shared_ptr<LiteralNode> functions(
        int lineno,
        std::shared_ptr<ListNode> args) {
        auto nodes = Query::functions();
        return nodeSetToList(lineno, nodes);
    }

    static std::shared_ptr<LiteralNode> instructions(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        auto nodes = Query::instructions({node->value()}, Query::ALL_NODES);
        return nodeSetToList(lineno, nodes);
    }

    static std::shared_ptr<LiteralNode> name(int lineno,
                                             std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        return std::make_shared<StringNode>(lineno, node->value()->name());
    }

    static std::shared_ptr<LiteralNode> index(int lineno,
                                              std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        return std::make_shared<IntNode>(lineno, node->value()->index());
    }

    static std::shared_ptr<LiteralNode> nargs(int lineno,
                                              std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        return std::make_shared<IntNode>(lineno, node->value()->nargs());
    }

    static std::shared_ptr<LiteralNode> nlocals(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        return std::make_shared<IntNode>(lineno, node->value()->nlocals());
    }

    static std::shared_ptr<LiteralNode> nresults(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        return std::make_shared<IntNode>(lineno, node->value()->nresults());
    }

    static std::shared_ptr<LiteralNode> isImport(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        return std::make_shared<BoolNode>(lineno, node->value()->isImport());
    }

    static std::shared_ptr<LiteralNode> isExport(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        return std::make_shared<BoolNode>(lineno, node->value()->isExport());
    }

    static std::shared_ptr<LiteralNode> varType(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        return std::make_shared<StringNode>(
            lineno, Utils::writeConstType(node->value()->varType()));
    }

    static std::shared_ptr<LiteralNode> opcode(int lineno,
                                               std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        return std::make_shared<StringNode>(lineno,
                                            node->value()->opcode().GetName());
    }

    static std::shared_ptr<LiteralNode> hasElse(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        return std::make_shared<BoolNode>(lineno, node->value()->hasElse());
    }

    static std::shared_ptr<LiteralNode> offset(int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        return std::make_shared<IntNode>(lineno, node->value()->offset());
    }

    static std::shared_ptr<LiteralNode> instType(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        return std::make_shared<StringNode>(
            lineno, INST_TYPE_MAP.at(node->value()->instType()));
    }

    static std::shared_ptr<LiteralNode> label(int lineno,
                                              std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        return std::make_shared<StringNode>(lineno, node->value()->label());
    }

    static std::shared_ptr<LiteralNode> child(int lineno,
                                              std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(3);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        ASSERT_EXPR_TYPE_(args->node(1), ExprType::Int);
        ASSERT_EXPR_TYPE_(args->node(2), ExprType::String);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));
        auto i = std::dynamic_pointer_cast<IntNode>(args->node(1));
        auto edgeType = std::dynamic_pointer_cast<StringNode>(args->node(2));

        auto child =
            NodeStream(node->value())
                .child(i->value(), EDGE_TYPES_MAP_R.at(edgeType->value()))
                .findFirst();

        if (child.isPresent()) {
            return std::make_shared<NodePointer>(lineno, child.get());
        }

        return std::make_shared<NilNode>(lineno);
    }

    static std::shared_ptr<LiteralNode> PDGEdge(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(3);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        ASSERT_EXPR_TYPE_(args->node(1), ExprType::Node);
        ASSERT_EXPR_TYPE_(args->node(2), ExprType::String);
        auto src = std::dynamic_pointer_cast<NodePointer>(args->node(0));
        auto dest = std::dynamic_pointer_cast<NodePointer>(args->node(1));
        auto pdgType = std::dynamic_pointer_cast<StringNode>(args->node(2));

        auto edge = EdgeStream(src->value()->outEdges(EdgeType::PDG))
                        .filter([&](Edge* e) {
                            return e->dest()->id() == dest->value()->id();
                        })
                        .filterPDG(PDG_TYPE_MAP_R.at(pdgType->value()))
                        .findFirst();

        if (edge.isPresent()) {
            std::make_shared<EdgePointer>(lineno, edge.get());
        }
        return std::make_shared<NilNode>(lineno);
    }

    static std::shared_ptr<LiteralNode> descendantsCFG(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        auto node = std::dynamic_pointer_cast<NodePointer>(args->node(0));

        auto nodes =
            Query::BFS({node->value()}, Query::ALL_NODES, Query::CFG_EDGES);
        return nodeSetToList(lineno, nodes);
    }

    static std::shared_ptr<LiteralNode> reachesPDG(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(4);
        ASSERT_EXPR_TYPE_(args->node(0), ExprType::Node);
        ASSERT_EXPR_TYPE_(args->node(1), ExprType::Node);
        ASSERT_EXPR_TYPE_(args->node(2), ExprType::String);
        ASSERT_EXPR_TYPE_(args->node(3), ExprType::String);
        auto src = std::dynamic_pointer_cast<NodePointer>(args->node(0));
        auto dest = std::dynamic_pointer_cast<NodePointer>(args->node(1));
        auto pdgType = std::dynamic_pointer_cast<StringNode>(args->node(2));
        auto label = std::dynamic_pointer_cast<StringNode>(args->node(3));

        bool isPresent =
            NodeStream(src->value())
                .BFS(Predicate().id(dest->value()->id()),
                     [&](Edge* e) {
                         return e->type() == EdgeType::PDG &&
                                e->pdgType() ==
                                    PDG_TYPE_MAP_R.at(pdgType->value()) &&
                                e->label() == label->value();
                     })
                .findFirst()
                .isPresent();
        return std::make_shared<BoolNode>(lineno, isPresent);
    }

    static std::shared_ptr<LiteralNode> vulnerability(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(1);
        json vuln;
        Printer printer;
        args->node(0)->accept(&printer);
        vuln["type"] = printer.toString();

        if (args->size() >= 2) {
            args->node(1)->accept(&printer);
            vuln["function"] = printer.toString();
        }
        if (args->size() >= 3) {
            args->node(2)->accept(&printer);
            vuln["caller"] = printer.toString();
        }
        if (args->size() >= 4) {
            args->node(3)->accept(&printer);
            vuln["description"] = printer.toString();
        }
        std::cout << vuln.dump(2) << std::endl;
        return std::make_shared<NilNode>(lineno);
    }
};
}  // namespace wasmati

#endif
