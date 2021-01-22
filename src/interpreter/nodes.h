#ifndef WASMATI_NODES_H
#define WASMATI_NODES_H

#include <cmath>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace wasmati {
class SequenceNode;
class BlockNode;
class IdentifierNode;
class Foreach;
class IfNode;
class IfElseNode;
class IfElseNode;
class AssignNode;
class ExpressionNode;
class Node;
class Edge;

class Visitor {
public:
    virtual void visitSequenceNode(SequenceNode* node) = 0;
    virtual void visitBlockNode(BlockNode* node) = 0;
    virtual void visitIdentifierNode(IdentifierNode* node) = 0;
    virtual void visitForeach(Foreach* node) = 0;
    virtual void visitIfNode(IfNode* node) = 0;
    virtual void visitIfElseNode(IfElseNode* node) = 0;
    virtual void visitAssignNode(AssignNode* node) = 0;
    virtual void visitExpressionNode(ExpressionNode* expr) = 0;
};

enum class ExprType {
    RValue,
    Sequence,
    SequenceLiteral,
    FunctionCall,
    Range,
    At,
    Int,
    Float,
    String,
    Bool,
    List,
    Nil,
    Node,
    Edge,
    NotEqual,
    Equal,
    And,
    Or,
    In,
    Not,
};

class BasicNode {
    int _lineno;  // source line

public:
    /**
     * Simple constructor.
     *
     * @param lineno the source code line number corresponding to the node
     */
    BasicNode(int lineno) : _lineno(lineno) {}

    virtual ~BasicNode() {}

public:
    /** @return the line number of the corresponding source code */
    int lineno() const { return _lineno; }

    /**
     * @return the label of the node (i.e., it's class)
     */
    std::string label() const {
        std::string fullname = typeid(*this).name();
        int last = fullname.find_last_of("0123456789");
        return fullname.substr(last + 1, fullname.length() - last - 1 - 1);
    }

    virtual void accept(Visitor* visitor) = 0;
};

class SequenceNode : public BasicNode {
    typedef std::vector<std::shared_ptr<BasicNode>> SequenceType;
    SequenceType _nodes;

public:
    SequenceNode(int lineno) : BasicNode(lineno) {}

    /**
     * Example: constructor for a left recursive production node:
     * <pre>
     * sequence: item {$$ = new Sequence(LINE, $1);}* | sequence item {$$ = new
     * Sequence(LINE, $2, $1);}* </pre> The constructor of a sequence node takes
     * the same first two arguments as any other node. The third argument is the
     * number of child nodes: this argument is followed by the child nodes
     * themselves. Note that no effort is made to ensure that the given number
     * of children matches the actual children passed to the function. <b>You
     * have been warned...</b>
     *
     * @param lineno the source code line number that originated the node
     * @param item is the single element to be added to the sequence
     * @param sequence is a previous sequence (nodes will be imported)
     */
    SequenceNode(int lineno, BasicNode* item, SequenceNode* sequence = nullptr)
        : BasicNode(lineno) {
        if (sequence != nullptr)
            _nodes = sequence->nodes();
        _nodes.push_back(std::shared_ptr<BasicNode>(item));
    }

    SequenceNode(int lineno, SequenceType nodes)
        : BasicNode(lineno), _nodes(nodes) {}

public:
    std::shared_ptr<BasicNode> node(size_t i) { return _nodes[i]; }
    SequenceType& nodes() { return _nodes; }
    size_t size() { return _nodes.size(); }

    void accept(Visitor* visitor) override { visitor->visitSequenceNode(this); }
};

class BlockNode : public BasicNode {
    std::shared_ptr<SequenceNode> _stmt;

public:
    inline BlockNode(int lineno, SequenceNode* stmt)
        : BasicNode(lineno), _stmt(std::shared_ptr<SequenceNode>(stmt)) {}

public:
    inline std::shared_ptr<SequenceNode> statements() { return _stmt; }

    void accept(Visitor* visitor) override { visitor->visitBlockNode(this); }
};

class LValueNode : public BasicNode {
protected:
    LValueNode(int lineno) : BasicNode(lineno) {}
};

class IdentifierNode : public LValueNode {
    std::string _name;

public:
    IdentifierNode(int lineno, const char* s) : LValueNode(lineno), _name(s) {}
    IdentifierNode(int lineno, const std::string& s)
        : LValueNode(lineno), _name(s) {}
    IdentifierNode(int lineno, const std::string* s)
        : LValueNode(lineno), _name(*s) {}

public:
    const std::string& name() const { return _name; }

    void accept(Visitor* visitor) override {
        visitor->visitIdentifierNode(this);
    }
};

class ExpressionNode : public BasicNode {
protected:
    /**
     * @param lineno the source code line corresponding to the node
     */
    ExpressionNode(int lineno) : BasicNode(lineno) {}

public:
    virtual ExprType type() = 0;

    void accept(Visitor* visitor) override {
        visitor->visitExpressionNode(this);
    }
};

class Foreach : public BasicNode {
    std::string _identifier;
    ExpressionNode* _expr;
    SequenceNode* _insts;

public:
    inline Foreach(int lineno,
                   std::string* identifier,
                   ExpressionNode* expr,
                   SequenceNode* insts)
        : BasicNode(lineno),
          _identifier(*identifier),
          _expr(expr),
          _insts(insts) {}

public:
    inline std::string identifier() { return _identifier; }

    inline ExpressionNode* expr() { return _expr; }

    inline SequenceNode* insts() { return _insts; }

    void accept(Visitor* visitor) override { visitor->visitForeach(this); }
};

class IfNode : public BasicNode {
    std::shared_ptr<ExpressionNode> _condition;
    std::shared_ptr<BasicNode> _block;

public:
    inline IfNode(int lineno, ExpressionNode* condition, BasicNode* block)
        : BasicNode(lineno), _condition(condition), _block(block) {}

public:
    inline std::shared_ptr<ExpressionNode> condition() { return _condition; }
    inline std::shared_ptr<BasicNode> block() { return _block; }

    void accept(Visitor* visitor) override { visitor->visitIfNode(this); }
};

class IfElseNode : public BasicNode {
    std::shared_ptr<ExpressionNode> _condition;
    std::shared_ptr<BasicNode> _thenblock, _elseblock;

public:
    inline IfElseNode(int lineno,
                      ExpressionNode* condition,
                      BasicNode* thenblock,
                      BasicNode* elseblock)
        : BasicNode(lineno),
          _condition(condition),
          _thenblock(thenblock),
          _elseblock(elseblock) {}

public:
    inline std::shared_ptr<ExpressionNode> condition() { return _condition; }
    inline std::shared_ptr<BasicNode> thenblock() { return _thenblock; }
    inline std::shared_ptr<BasicNode> elseblock() { return _elseblock; }

    void accept(Visitor* visitor) override { visitor->visitIfElseNode(this); }
};

class AssignNode : public BasicNode {
    std::shared_ptr<LValueNode> _lvalue;
    std::shared_ptr<ExpressionNode> _rvalue;

public:
    AssignNode(int lineno, LValueNode* lvalue, ExpressionNode* rvalue)
        : BasicNode(lineno), _lvalue(lvalue), _rvalue(rvalue) {}

public:
    std::shared_ptr<LValueNode> lvalue() { return _lvalue; }
    std::shared_ptr<ExpressionNode> rvalue() { return _rvalue; }

    void accept(Visitor* visitor) override { visitor->visitAssignNode(this); }
};

/*** Expressions ***/
class SequenceExprNode : public ExpressionNode {
    typedef std::vector<std::shared_ptr<ExpressionNode>> SequenceType;
    SequenceType _nodes;

public:
    SequenceExprNode(int lineno) : ExpressionNode(lineno) {}

    SequenceExprNode(int lineno,
                     ExpressionNode* item,
                     SequenceExprNode* sequence = nullptr)
        : ExpressionNode(lineno) {
        if (sequence != nullptr)
            _nodes = sequence->nodes();
        _nodes.push_back(std::shared_ptr<ExpressionNode>(item));
    }

    SequenceExprNode(int lineno, SequenceType nodes)
        : ExpressionNode(lineno), _nodes(nodes) {}

public:
    std::shared_ptr<ExpressionNode> node(size_t i) { return _nodes[i]; }
    SequenceType& nodes() { return _nodes; }
    size_t size() { return _nodes.size(); }

    ExprType type() override { return ExprType::Sequence; }
};

class RValueNode : public ExpressionNode {
    std::shared_ptr<LValueNode> _lvalue;

public:
    RValueNode(int lineno, LValueNode* lvalue)
        : ExpressionNode(lineno), _lvalue(lvalue) {}

public:
    std::shared_ptr<LValueNode> lvalue() { return _lvalue; }

    ExprType type() override { return ExprType::RValue; }
};

class FunctionCall : public ExpressionNode {
    std::string _identifier;
    std::shared_ptr<SequenceExprNode> _parameters;

public:
    inline FunctionCall(int lineno,
                        std::string* identifier,
                        SequenceExprNode* parameters)
        : ExpressionNode(lineno),
          _identifier(*identifier),
          _parameters(parameters) {}

    inline FunctionCall(int lineno,
                        std::string* identifier,
                        ExpressionNode* parameter)
        : ExpressionNode(lineno),
          _identifier(*identifier),
          _parameters(new SequenceExprNode(lineno, parameter)) {}

public:
    inline std::string identifier() { return _identifier; }

    inline std::shared_ptr<SequenceExprNode> parameters() {
        return _parameters;
    }

    ExprType type() override { return ExprType::FunctionCall; }
};

class RangeExprNode : public ExpressionNode {
    std::shared_ptr<ExpressionNode> _expr;
    std::string _var;
    std::shared_ptr<ExpressionNode> _domain;
    std::shared_ptr<ExpressionNode> _predicate;

public:
    inline RangeExprNode(int lineno,
                         ExpressionNode* expr,
                         std::string* var,
                         ExpressionNode* domain,
                         ExpressionNode* predicate)
        : ExpressionNode(lineno),
          _expr(expr),
          _var(*var),
          _domain(domain),
          _predicate(predicate) {}

public:
    inline std::shared_ptr<ExpressionNode> expr() { return _expr; }
    inline std::string var() { return _var; }
    inline std::shared_ptr<ExpressionNode> domain() { return _domain; }
    inline std::shared_ptr<ExpressionNode> predicate() { return _predicate; }
    ExprType type() override { return ExprType::Range; }
};

class AtNode : public ExpressionNode {
    std::shared_ptr<ExpressionNode> _expr;
    std::shared_ptr<ExpressionNode> _index;

public:
    inline AtNode(int lineno, ExpressionNode* expr, ExpressionNode* index)
        : ExpressionNode(lineno), _expr(expr), _index(index) {}

public:
    inline std::shared_ptr<ExpressionNode> expr() { return _expr; }

    inline std::shared_ptr<ExpressionNode> index() { return _index; }

    ExprType type() override { return ExprType::At; }
};

class LiteralNode : public ExpressionNode {
protected:
    /**
     * @param lineno the source code line corresponding to the node
     */
    LiteralNode(int lineno) : ExpressionNode(lineno) {}

public:
    virtual bool equals(std::shared_ptr<LiteralNode> other) = 0;
};

class SequenceLiteralNode : public ExpressionNode {
public:
    typedef std::vector<std::shared_ptr<LiteralNode>> SequenceType;

private:
    SequenceType _nodes;

public:
    SequenceLiteralNode(int lineno) : ExpressionNode(lineno) {}

    SequenceLiteralNode(int lineno,
                        LiteralNode* item,
                        SequenceLiteralNode* sequence = nullptr)
        : ExpressionNode(lineno) {
        if (sequence != nullptr)
            _nodes = sequence->nodes();
        _nodes.push_back(std::shared_ptr<LiteralNode>(item));
    }

    SequenceLiteralNode(int lineno, SequenceType nodes)
        : ExpressionNode(lineno), _nodes(nodes) {}

    SequenceLiteralNode(int lineno,
                        std::list<std::shared_ptr<LiteralNode>> nodes)
        : ExpressionNode(lineno),
          _nodes(SequenceType(nodes.begin(), nodes.end())) {}

public:
    std::shared_ptr<LiteralNode> node(size_t i) { return _nodes[i]; }
    SequenceType& nodes() { return _nodes; }
    size_t size() { return _nodes.size(); }

    void insert(std::shared_ptr<LiteralNode> item) { _nodes.push_back(item); }

    ExprType type() override { return ExprType::SequenceLiteral; }
};

template <typename StoredType, ExprType T>
class Literal : public LiteralNode {
    StoredType _value;

public:
    Literal(int lineno) : LiteralNode(lineno) {}

    Literal(int lineno, const StoredType& value)
        : LiteralNode(lineno), _value(value) {}

    Literal(int lineno, Literal<StoredType, T>* lit)
        : LiteralNode(lit->lineno()), _value(lit->value()) {}

    const StoredType& value() const { return _value; }

    ExprType type() override { return T; }

    bool equals(std::shared_ptr<LiteralNode> other) override {
        if (this->type() != other->type()) {
            return false;
        }
        auto o = std::dynamic_pointer_cast<Literal<StoredType, T>>(other);
        return this->value() == o->value();
    }
};

typedef Literal<long int, ExprType::Int> IntNode;
typedef Literal<double, ExprType::Float> FloatNode;
typedef Literal<std::string, ExprType::String> StringNode;
typedef Literal<bool, ExprType::Bool> BoolNode;
typedef Literal<void*, ExprType::Nil> NilNode;
typedef Literal<Node*, ExprType::Node> NodePointer;
typedef Literal<Edge*, ExprType::Edge> EdgePointer;

class ListNode
    : public Literal<std::shared_ptr<SequenceLiteralNode>, ExprType::List> {
    typedef std::shared_ptr<SequenceLiteralNode> StoredType;

public:
    ListNode(int lineno) : Literal<StoredType, ExprType::List>(lineno) {}

    ListNode(int lineno, const StoredType& value)
        : Literal<StoredType, ExprType::List>(lineno, value) {}

    ListNode(int lineno, Literal<StoredType, ExprType::List>* lit)
        : Literal<StoredType, ExprType::List>(lineno, lit) {}

    ListNode(int lineno, SequenceLiteralNode* value)
        : Literal<StoredType, ExprType::List>(lineno, StoredType(value)) {}

public:
    std::shared_ptr<LiteralNode> node(size_t i) { return value()->node(i); }
    SequenceLiteralNode::SequenceType& nodes() { return value()->nodes(); }
    size_t size() { return value()->nodes().size(); }
};

class BinopNode : public ExpressionNode {
    ExpressionNode *_left, *_right;

public:
    /**
     * @param lineno source code line number for this node
     * @param left first operand
     * @param right second operand
     */
    BinopNode(int lineno, ExpressionNode* left, ExpressionNode* right)
        : ExpressionNode(lineno), _left(left), _right(right) {}

    ExpressionNode* left() { return _left; }
    ExpressionNode* right() { return _right; }
};

template <ExprType T>
class BinopExprNode : public BinopNode {
public:
    BinopExprNode(int lineno, ExpressionNode* left, ExpressionNode* right)
        : BinopNode(lineno, left, right) {}

    ExprType type() override { return T; }
};

typedef BinopExprNode<ExprType::Equal> EqualNode;
typedef BinopExprNode<ExprType::NotEqual> NotEqualNode;
typedef BinopExprNode<ExprType::And> AndNode;
typedef BinopExprNode<ExprType::Or> OrNode;
typedef BinopExprNode<ExprType::In> InNode;

class UnopNode : public ExpressionNode {
    ExpressionNode* _argument;

public:
    UnopNode(int lineno, ExpressionNode* arg)
        : ExpressionNode(lineno), _argument(arg) {}

    ExpressionNode* argument() { return _argument; }
};

template <ExprType T>
class UnopExprNode : public UnopNode {
public:
    UnopExprNode(int lineno, ExpressionNode* argument)
        : UnopNode(lineno, argument) {}

    ExprType type() override { return T; }
};
typedef UnopExprNode<ExprType::Not> NotNode;

template <typename T>
class VisitorTemplate : public Visitor {
protected:
    T _resultVisit;

public:
    virtual T visitRValueNode(RValueNode* node) = 0;
    virtual T visitFunctionCall(FunctionCall* node) = 0;
    virtual T visitRangeExprNode(RangeExprNode* node) = 0;
    virtual T visitAtNode(AtNode* node) = 0;
    virtual T visitIntNode(IntNode* node) = 0;
    virtual T visitFloatNode(FloatNode* node) = 0;
    virtual T visitStringNode(StringNode* node) = 0;
    virtual T visitBoolNode(BoolNode* node) = 0;
    virtual T visitListNode(ListNode* node) = 0;
    virtual T visitNilNode(NilNode* node) = 0;
    virtual T visitNodePointer(NodePointer* node) = 0;
    virtual T visitEdgePointer(EdgePointer* node) = 0;
    virtual T visitEqualNode(EqualNode* node) = 0;
    virtual T visitNotEqualNode(NotEqualNode* node) = 0;
    virtual T visitAndNode(AndNode* node) = 0;
    virtual T visitOrNode(OrNode* node) = 0;
    virtual T visitInNode(InNode* node) = 0;
    virtual T visitNotNode(NotNode* node) = 0;
    virtual T visitSequenceExprNode(SequenceExprNode* node) = 0;
    virtual T visitSequenceLiteralNode(SequenceLiteralNode* node) = 0;

    void visitExpressionNode(ExpressionNode* expr) override {
        switch (expr->type()) {
        case ExprType::And:
            _resultVisit = visitAndNode(dynamic_cast<AndNode*>(expr));
            break;
        case ExprType::At:
            _resultVisit = visitAtNode(dynamic_cast<AtNode*>(expr));
            break;
        case ExprType::Bool:
            _resultVisit = visitBoolNode(dynamic_cast<BoolNode*>(expr));
            break;
        case ExprType::Equal:
            _resultVisit = visitEqualNode(dynamic_cast<EqualNode*>(expr));
            break;
        case ExprType::NotEqual:
            _resultVisit = visitNotEqualNode(dynamic_cast<NotEqualNode*>(expr));
            break;
        case ExprType::Float:
            _resultVisit = visitFloatNode(dynamic_cast<FloatNode*>(expr));
            break;
        case ExprType::FunctionCall:
            _resultVisit = visitFunctionCall(dynamic_cast<FunctionCall*>(expr));
            break;
        case ExprType::In:
            _resultVisit = visitInNode(dynamic_cast<InNode*>(expr));
            break;
        case ExprType::Int:
            _resultVisit = visitIntNode(dynamic_cast<IntNode*>(expr));
            break;
        case ExprType::List:
            _resultVisit = visitListNode(dynamic_cast<ListNode*>(expr));
            break;
        case ExprType::Nil:
            _resultVisit = visitNilNode(dynamic_cast<NilNode*>(expr));
            break;
        case ExprType::Node:
            _resultVisit = visitNodePointer(dynamic_cast<NodePointer*>(expr));
            break;
        case ExprType::Edge:
            _resultVisit = visitEdgePointer(dynamic_cast<EdgePointer*>(expr));
            break;
        case ExprType::Not:
            _resultVisit = visitNotNode(dynamic_cast<NotNode*>(expr));
            break;
        case ExprType::Or:
            _resultVisit = visitOrNode(dynamic_cast<OrNode*>(expr));
            break;
        case ExprType::Range:
            _resultVisit =
                visitRangeExprNode(dynamic_cast<RangeExprNode*>(expr));
            break;
        case ExprType::RValue:
            _resultVisit = visitRValueNode(dynamic_cast<RValueNode*>(expr));
            break;
        case ExprType::Sequence:
            _resultVisit =
                visitSequenceExprNode(dynamic_cast<SequenceExprNode*>(expr));
            break;
        case ExprType::SequenceLiteral:
            _resultVisit = visitSequenceLiteralNode(
                dynamic_cast<SequenceLiteralNode*>(expr));
            break;
        case ExprType::String:
            _resultVisit = visitStringNode(dynamic_cast<StringNode*>(expr));
            break;
        default:
            break;
        }
    }
};

}  // namespace wasmati

#endif  // WASMATI_NODES_H
