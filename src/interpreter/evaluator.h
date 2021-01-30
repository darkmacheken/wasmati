#ifndef WASMATI_EVALUATOR_H
#define WASMATI_EVALUATOR_H
#include <exception>
#include "src/interpreter/interpreter.h"
#define ASSERT_EXPR_TYPE(TYPE)                                        \
    if (_resultVisit->type() != TYPE) {                               \
        throw InterpreterException(                                   \
            "Expected " + LITERAL_TYPE_MAP.at(TYPE) + " but got " +   \
            LITERAL_TYPE_MAP.at(_resultVisit->type()) + " in line " + \
            std::to_string(node->lineno()));                          \
    }

#define ASSERT_EXPR_TYPE_(EXPR, TYPE)                               \
    if (EXPR->type() != TYPE) {                                     \
        throw InterpreterException(                                 \
            "Expected " + LITERAL_TYPE_MAP.at(TYPE) + " but got " + \
            LITERAL_TYPE_MAP.at(EXPR->type()) + " in line " +       \
            std::to_string(EXPR->lineno()));                        \
    }

#define ASSERT_NUM_ARGS_(args, NARGS)                                   \
    if (args->value()->size() < NARGS) {                                \
        throw InterpreterException(                                     \
            "Expected atleast " + std::to_string(NARGS) + " but got " + \
            std::to_string(args->value()->size()) + " in line " +       \
            std::to_string(args->lineno()));                            \
    }

#define RETURN(ret)     \
    _resultVisit = ret; \
    return true;

#define NONE(line) std::make_shared<NoneNode>(line);
#define NIL(line) std::make_shared<NilNode>(line);

#define BINARY_OP(OP)                                                         \
    node->left()->accept(this);                                               \
    auto left = _resultVisit;                                                 \
    node->right()->accept(this);                                              \
    auto right = _resultVisit;                                                \
    if (operatorsMap[left->type()].count(OP) == 0) {                          \
        throw InterpreterException(                                           \
            LITERAL_TYPE_MAP.at(left->type()) + " does not have operator '" + \
            OP + "' in line " + std::to_string(node->lineno()));              \
    }                                                                         \
    RETURN(operatorsMap[left->type()][OP](node->lineno(), left, right));

#define BINARY_OP_R(OP)                                                        \
    node->left()->accept(this);                                                \
    auto left = _resultVisit;                                                  \
    node->right()->accept(this);                                               \
    auto right = _resultVisit;                                                 \
    if (operatorsMap[right->type()].count(OP) == 0) {                          \
        throw InterpreterException(                                            \
            LITERAL_TYPE_MAP.at(right->type()) + " does not have operator '" + \
            OP + "' in line " + std::to_string(node->lineno()));               \
    }                                                                          \
    RETURN(operatorsMap[right->type()][OP](node->lineno(), right, left));

#include <iostream>
#include "src/interpreter/nodes.h"
#include "src/interpreter/symbol-table.h"
#include "src/query.h"

namespace wasmati {
extern const std::map<LiteralType, std::string> LITERAL_TYPE_MAP;
extern const std::map<BinopType, std::string> BINARY_TYPE_MAP;
extern const std::map<UnopType, std::string> UNARY_TYPE_MAP;

typedef std::function<std::shared_ptr<LiteralNode>(
    int,
    std::shared_ptr<LiteralNode>,
    std::shared_ptr<LiteralNode>)>
    OperatorsFunction;
typedef std::function<
    std::shared_ptr<LiteralNode>(int, std::shared_ptr<LiteralNode>)>
    AttributeFunction;
typedef std::function<std::shared_ptr<LiteralNode>(int,
                                                   std::shared_ptr<LiteralNode>,
                                                   std::shared_ptr<ListNode>)>
    MemberFunction;

typedef std::function<std::shared_ptr<LiteralNode>(int,
                                                   std::shared_ptr<ListNode>)>
    Func;

extern std::map<LiteralType, std::map<std::string, OperatorsFunction>>
    operatorsMap;

extern std::map<LiteralType, std::map<std::string, AttributeFunction>>
    attributesMap;

extern std::map<LiteralType, std::map<std::string, MemberFunction>>
    memberFunctionsMap;

extern std::map<std::string, Func> functionsMap;

class InterpreterException : public std::exception {
    std::string _msg;

public:
    InterpreterException(std::string msg) : _msg(msg) {}

    virtual const char* what() const throw() { return _msg.c_str(); }
};

class Printer : public Visitor {
    bool _strWithQuotes;
    std::string _resultVisit;

public:
    Printer(bool strWithQuotes = true) : _strWithQuotes(strWithQuotes) {}

public:
    std::string toString() { return _resultVisit; }
    // Statements
    bool visitBlockNode(BlockNode* node) override { RETURN(""); }
    bool visitForeach(Foreach* node) override { RETURN(""); }
    bool visitIfNode(IfNode* node) override { RETURN(""); }
    bool visitIfElseNode(IfElseNode* node) override { RETURN(""); }
    bool visitFunction(FunctionNode* node) override { RETURN(""); }
    bool visitReturnNode(ReturnNode* node) override { RETURN(""); }
    bool visitImportNode(ImportNode* node) override { RETURN(""); }

    // basic expr
    bool visitAssignExpr(AssignExpr* node) override { RETURN(""); }
    bool visitRValueNode(RValueNode* node) override { RETURN(""); }
    bool visitFunctionCall(FunctionCall* node) override { RETURN(""); }
    bool visitAttributeCall(AttributeCall* node) { RETURN(""); }
    bool visitMemberFunctionCall(MemberFunctionCall* node) { RETURN(""); }
    bool visitFilterExprNode(FilterExprNode* node) override { RETURN(""); }
    bool visitAtNode(AtNode* node) override { RETURN(""); }
    bool visitTimeNode(TimeNode* node) override { RETURN(""); }

    // objects
    bool visitNodePointer(NodePointer* node) override {
        RETURN("[Node::" + NODE_TYPE_MAP.at(node->value()->type()) + "_" +
               std::to_string(node->value()->id()) + "]");
    }
    bool visitEdgePointer(EdgePointer* node) override {
        RETURN("[EDGE::" + EDGE_TYPES_MAP.at(node->value()->type()) + "_" +
               std::to_string(node->value()->src()->id()) + "->" +
               std::to_string(node->value()->dest()->id()) + "]");
    }

    // literals
    bool visitIntNode(IntNode* node) override {
        RETURN(std::to_string(node->value()));
    }
    bool visitFloatNode(FloatNode* node) override {
        RETURN(std::to_string(node->value()));
    }
    bool visitStringNode(StringNode* node) override {
        if (_strWithQuotes) {
            RETURN('"' + node->value() + '"');
        }
        RETURN(node->value());
    }
    bool visitBoolNode(BoolNode* node) override {
        RETURN(node->value() ? "true" : "false");
    }
    bool visitListNode(ListNode* node) override {
        std::string res = "[";
        for (auto lit : node->value()->nodes()) {
            lit->accept(this);
            res += _resultVisit + ",";
        }
        if (res.size() > 1) {
            res.pop_back();
        }
        res += "]";
        RETURN(res);
    }
    bool visitMapNode(MapNode* node) override {
        std::string res = "{";
        for (auto kv : node->value()) {
            res += '"' + kv.first + "\": ";
            kv.second->accept(this);
            res += _resultVisit + ",";
        }
        if (res.size() > 1) {
            res.pop_back();
        }
        res += "}";
        RETURN(res);
    }
    bool visitNilNode(NilNode* node) override { RETURN("nil"); }
    bool visitNoneNode(NoneNode* node) override { RETURN(""); }

    // binary expr
    bool visitEqualNode(EqualNode* node) override { RETURN(""); }
    bool visitNotEqualNode(NotEqualNode* node) override { RETURN(""); }
    bool visitAndNode(AndNode* node) override { RETURN(""); }
    bool visitOrNode(OrNode* node) override { RETURN(""); }
    bool visitInNode(InNode* node) override { RETURN(""); }
    bool visitLessNode(LessNode* node) { RETURN(""); }
    bool visitLessEqualNode(LessEqualNode* node) { RETURN(""); }
    bool visitGreaterNode(GreaterNode* node) { RETURN(""); }
    bool visitGreaterEqualNode(GreaterEqualNode* node) { RETURN(""); }
    bool visitAddNode(AddNode* node) { RETURN(""); }
    bool visitSubNode(SubNode* node) { RETURN(""); }
    bool visitMulNode(MulNode* node) { RETURN(""); }
    bool visitDivNode(DivNode* node) { RETURN(""); }
    bool visitModNode(ModNode* node) { RETURN(""); }

    // unary expr
    bool visitNotNode(NotNode* node) override { RETURN(""); }

    // sequences
    bool visitSequenceNode(SequenceNode* node) override { RETURN(""); }
    bool visitSequenceExprNode(SequenceExprNode* node) override { RETURN(""); }
    bool visitSequenceLiteralNode(SequenceLiteralNode* node) override {
        RETURN("");
    }
};

class Evaluator : public Visitor {
    Path _file;

    std::shared_ptr<LiteralNode> _resultVisit;

    SymbolTable<std::shared_ptr<LiteralNode>> _symTab;
    SymbolTable<Func> _funcTab;
    std::string _lookUp;

public:
    Evaluator(json& config, std::string file = "") : _file(Path(file)) {
        _symTab.insert("config", jsonToLiteral(config));
    }

public:
    std::string resultToString() {
        Printer printer;
        _resultVisit->accept(&printer);
        return printer.toString();
    }

    // statements
    bool visitBlockNode(BlockNode* node) override {
        _funcTab.push();
        _symTab.push();
        node->statements()->accept(this);
        _symTab.pop();
        _funcTab.pop();
        RETURN(NONE(node->lineno()));
    }

    bool visitForeach(Foreach* node) override {
        node->expr()->accept(this);
        ASSERT_EXPR_TYPE(LiteralType::List);
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
        RETURN(NONE(node->lineno()));
    }

    bool visitIfNode(IfNode* node) override {
        node->condition()->accept(this);
        ASSERT_EXPR_TYPE(LiteralType::Bool);
        auto cond = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        if (cond->value()) {
            node->block()->accept(this);
        }
        RETURN(NONE(node->lineno()));
    }

    bool visitIfElseNode(IfElseNode* node) override {
        node->condition()->accept(this);
        ASSERT_EXPR_TYPE(LiteralType::Bool);
        auto cond = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        if (cond->value()) {
            node->thenblock()->accept(this);
        } else {
            node->elseblock()->accept(this);
        }
        RETURN(NONE(node->lineno()));
    }

    bool visitFunction(FunctionNode* node) override {
        auto identifiers = node->identifiers();
        auto block = node->block();
        Func func = [&, identifiers, block](int lineno,
                                            std::shared_ptr<ListNode> args) {
            ASSERT_NUM_ARGS_(args, identifiers.size());
            for (size_t i = 0; i < identifiers.size(); i++) {
                if (!_symTab.insert(identifiers[i], args->value()->node(i))) {
                    _symTab.replace(identifiers[i], args->value()->node(i));
                }
            }
            try {
                block->accept(this);

            } catch (std::shared_ptr<LiteralNode>& res) {
                return res;
            }
            return _resultVisit;
        };
        if (!_funcTab.insert(node->name(), func)) {
            _funcTab.replace(node->name(), func);
        }
        RETURN(NONE(node->lineno()));
    }

    bool visitReturnNode(ReturnNode* node) override {
        if (node->expr() == nullptr) {
            throw NIL(node->lineno());
        }
        node->expr()->accept(this);
        throw _resultVisit;
        RETURN(NONE(node->lineno()));
    }
    bool visitImportNode(ImportNode* node) override {
        std::string directory;
        if (_file.empty()) {
            directory = Path::getCurrentDir();
        } else {
            directory = _file.directory();
        }

        Interpreter interp;
        bool res = interp.parse_file(directory + "/" + node->file());
        interp.evaluate(this);
        RETURN(std::make_shared<BoolNode>(node->lineno(), res));
    }

    // expr
    bool visitAssignExpr(AssignExpr* node) override {
        node->expr()->accept(this);
        if (!_symTab.replace(node->identifier(), _resultVisit)) {
            _symTab.insert(node->identifier(), _resultVisit);
        }
        RETURN(_resultVisit);
    }

    bool visitRValueNode(RValueNode* node) override {
        auto lit = _symTab.find(node->name());
        if (_symTab.find(node->name()) == nullptr) {
            throw InterpreterException("Unbounded variable " + node->name() +
                                       " in line " +
                                       std::to_string(node->lineno()));
        }
        RETURN(lit);
    }
    bool visitFunctionCall(FunctionCall* node) override {
        auto func = functionsMap.count(node->name()) == 1
                        ? functionsMap[node->name()]
                        : _funcTab.find(node->name());
        if (func == nullptr) {
            throw InterpreterException("Unbounded function " + node->name() +
                                       " in line " +
                                       std::to_string(node->lineno()));
        }
        node->parameters()->accept(this);
        ASSERT_EXPR_TYPE(LiteralType::List);
        auto args = std::dynamic_pointer_cast<ListNode>(_resultVisit);
        _symTab.push();
        auto res = func(node->lineno(), args);
        _symTab.pop();
        RETURN(res);
    }

    bool visitAttributeCall(AttributeCall* node) {
        node->expr()->accept(this);
        auto expr = _resultVisit;
        if (attributesMap[expr->type()].count(node->name()) == 0) {
            throw InterpreterException(LITERAL_TYPE_MAP.at(expr->type()) +
                                       " does not have attribute " +
                                       node->name() + " in line " +
                                       std::to_string(node->lineno()));
        }
        RETURN(attributesMap[expr->type()][node->name()](node->lineno(), expr));
    }

    bool visitMemberFunctionCall(MemberFunctionCall* node) {
        node->expr()->accept(this);
        auto expr = _resultVisit;
        if (memberFunctionsMap[expr->type()].count(node->name()) == 0) {
            throw InterpreterException(LITERAL_TYPE_MAP.at(expr->type()) +
                                       " does not have member function " +
                                       node->name() + " in line " +
                                       std::to_string(node->lineno()));
            RETURN(std::make_shared<NilNode>(node->lineno()));
        }
        node->parameters()->accept(this);
        ASSERT_EXPR_TYPE(LiteralType::List);
        auto parameters = std::dynamic_pointer_cast<ListNode>(_resultVisit);
        RETURN(memberFunctionsMap[expr->type()][node->name()](
            node->lineno(), expr, parameters));
    }

    bool visitFilterExprNode(FilterExprNode* node) override {
        node->domain()->accept(this);
        ASSERT_EXPR_TYPE(LiteralType::List);
        auto list = std::dynamic_pointer_cast<ListNode>(_resultVisit);
        auto result = std::make_shared<SequenceLiteralNode>(node->lineno());

        _symTab.push();
        _symTab.insert(node->var(), std::make_shared<NilNode>(node->lineno()));
        for (auto lit : list->value()->nodes()) {
            _symTab.replace(node->var(), lit);
            node->predicate()->accept(this);
            ASSERT_EXPR_TYPE(LiteralType::Bool);
            auto pred = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
            if (pred->value()) {
                result->insert(lit);
            }
        }
        _symTab.pop();
        RETURN(std::make_shared<ListNode>(node->lineno(), result));
    }
    bool visitAtNode(AtNode* node) override {
        node->expr()->accept(this);
        auto expr = _resultVisit;
        node->index()->accept(this);
        auto index = _resultVisit;
        if (operatorsMap[expr->type()].count("[]") == 0) {
            throw InterpreterException(LITERAL_TYPE_MAP.at(expr->type()) +
                                       " does not have operator '[]' in line " +
                                       std::to_string(node->lineno()));
        }
        RETURN(operatorsMap[expr->type()]["[]"](node->lineno(), expr, index));
    }

    bool visitTimeNode(TimeNode* node) override {
        auto start = std::chrono::high_resolution_clock::now();
        node->expr()->accept(this);
        auto end = std::chrono::high_resolution_clock::now();
        RETURN(std::make_shared<IntNode>(
            node->lineno(),
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count()));
    }

    // objects
    bool visitNodePointer(NodePointer* node) override {
        RETURN(std::make_shared<NodePointer>(node->lineno(), node));
    }
    bool visitEdgePointer(EdgePointer* node) override {
        RETURN(std::make_shared<EdgePointer>(node->lineno(), node));
    }

    // literals
    bool visitIntNode(IntNode* node) override {
        RETURN(std::make_shared<IntNode>(node->lineno(), node));
    }
    bool visitFloatNode(FloatNode* node) override {
        RETURN(std::make_shared<FloatNode>(node->lineno(), node));
    }
    bool visitStringNode(StringNode* node) override {
        RETURN(std::make_shared<StringNode>(node->lineno(), node));
    }
    bool visitBoolNode(BoolNode* node) override {
        RETURN(std::make_shared<BoolNode>(node->lineno(), node));
    }
    bool visitListNode(ListNode* node) override {
        RETURN(std::make_shared<ListNode>(node->lineno(), node));
    }
    bool visitMapNode(MapNode* node) override {
        RETURN(std::make_shared<MapNode>(node->lineno(), node));
    }
    bool visitNilNode(NilNode* node) override {
        RETURN(std::make_shared<NilNode>(node->lineno(), node));
    }
    bool visitNoneNode(NoneNode* node) override {
        RETURN(std::make_shared<NoneNode>(node->lineno(), node));
    }

    // binary expr
    bool visitEqualNode(EqualNode* node) override {
        node->left()->accept(this);
        auto left = _resultVisit;
        node->right()->accept(this);
        auto right = _resultVisit;
        RETURN(std::make_shared<BoolNode>(node->lineno(), left->equals(right)));
    }
    bool visitNotEqualNode(NotEqualNode* node) override {
        node->left()->accept(this);
        auto left = _resultVisit;
        node->right()->accept(this);
        auto right = _resultVisit;
        RETURN(
            std::make_shared<BoolNode>(node->lineno(), !left->equals(right)));
    }
    bool visitAndNode(AndNode* node) override {
        node->left()->accept(this);
        ASSERT_EXPR_TYPE(LiteralType::Bool);
        auto left = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        if (!left->value()) {
            RETURN(std::make_shared<BoolNode>(node->lineno(), false));
        }
        node->right()->accept(this);
        ASSERT_EXPR_TYPE(LiteralType::Bool);
        auto right = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        RETURN(std::make_shared<BoolNode>(node->lineno(),
                                          left->value() && right->value()));
    }
    bool visitOrNode(OrNode* node) override {
        node->left()->accept(this);
        ASSERT_EXPR_TYPE(LiteralType::Bool);
        auto left = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        node->right()->accept(this);
        ASSERT_EXPR_TYPE(LiteralType::Bool);
        auto right = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        RETURN(std::make_shared<BoolNode>(node->lineno(),
                                          left->value() || right->value()));
    }
    bool visitInNode(InNode* node) override { BINARY_OP_R("in"); }

    bool visitLessNode(LessNode* node) { BINARY_OP("<"); }
    bool visitLessEqualNode(LessEqualNode* node) { BINARY_OP("<="); }
    bool visitGreaterNode(GreaterNode* node) { BINARY_OP(">"); }
    bool visitGreaterEqualNode(GreaterEqualNode* node) { BINARY_OP(">="); }
    bool visitAddNode(AddNode* node) { BINARY_OP("+"); }
    bool visitSubNode(SubNode* node) { BINARY_OP("-"); }
    bool visitMulNode(MulNode* node) { BINARY_OP("*"); }
    bool visitDivNode(DivNode* node) { BINARY_OP("/"); }
    bool visitModNode(ModNode* node) { BINARY_OP("%"); }

    // unary expr
    bool visitNotNode(NotNode* node) override {
        node->argument()->accept(this);
        ASSERT_EXPR_TYPE(LiteralType::Bool);
        auto arg = std::dynamic_pointer_cast<BoolNode>(_resultVisit);
        RETURN(std::make_shared<BoolNode>(node->lineno(), !arg->value()));
    }

    // sequences
    bool visitSequenceNode(SequenceNode* node) override {
        for (auto basicNode : node->nodes()) {
            if (basicNode == nullptr) {
                continue;
            }
            basicNode->accept(this);
        }
        RETURN(NONE(node->lineno()));
    }

    bool visitSequenceExprNode(SequenceExprNode* node) {
        auto seq = std::make_shared<SequenceLiteralNode>(node->lineno());
        for (auto expr : node->nodes()) {
            expr->accept(this);
            seq->insert(_resultVisit);
        }
        RETURN(std::make_shared<ListNode>(node->lineno(), seq));
    }
    bool visitSequenceLiteralNode(SequenceLiteralNode* node) override {
        RETURN(std::make_shared<ListNode>(
            node->lineno(), std::shared_ptr<SequenceLiteralNode>(node)));
    }

private:
    std::shared_ptr<LiteralNode> jsonToLiteral(json& object) {
        switch (object.type()) {
        case json::value_t::number_integer:
        case json::value_t::number_unsigned:
            return std::make_shared<IntNode>(0, object);
        case json::value_t::number_float:
            return std::make_shared<FloatNode>(0, object);
        case json::value_t::boolean:
            return std::make_shared<BoolNode>(0, object);
        case json::value_t::string:
            return std::make_shared<StringNode>(0, object);
        case json::value_t::array: {
            SequenceLiteralNode::SequenceType list;
            for (auto item : object) {
                list.push_back(jsonToLiteral(item));
            }
            return std::make_shared<ListNode>(
                0, std::make_shared<SequenceLiteralNode>(0, list));
        }
        case json::value_t::object: {
            auto res = std::make_shared<MapNode>(0);
            for (auto it = object.begin(); it != object.end(); it++) {
                res->value()[it.key()] = jsonToLiteral(object.at(it.key()));
            }
            return res;
        }
        default:
            return NIL(0);
        }
    }
};

}  // namespace wasmati

#endif
