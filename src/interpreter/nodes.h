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
class Visitor;
class Node;
class Edge;

enum class LiteralType {
    None,
    Int,
    Float,
    String,
    Bool,
    List,
    Map,
    Nil,
    Node,
    Edge,
};

enum class BinopType {
    Equal,
    NotEqual,
    And,
    Or,
    In,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
};

enum class UnopType {
    Not,
};

class BasicNode {
    /// @brief source line
    int _lineno;

public:
    BasicNode(int lineno) : _lineno(lineno) {}

    virtual ~BasicNode() {}

public:
    int lineno() const { return _lineno; }

    virtual bool accept(Visitor* visitor) = 0;
};

class ExpressionNode : public BasicNode {
protected:
    ExpressionNode(int lineno) : BasicNode(lineno) {}
};

class LiteralNode : public ExpressionNode {
protected:
    LiteralNode(int lineno) : ExpressionNode(lineno) {}

public:
    virtual LiteralType type() = 0;
    virtual bool equals(std::shared_ptr<LiteralNode> other) = 0;
};

template <typename Item, typename T>
class Sequence : public T {
public:
    typedef std::vector<Item> SequenceType;

private:
    SequenceType _nodes;

public:
    Sequence(int lineno) : T(lineno) {}

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
    Sequence(int lineno, Item item, Sequence<Item, T>* sequence = nullptr)
        : T(lineno) {
        if (sequence != nullptr)
            _nodes = sequence->nodes();
        _nodes.push_back(item);
    }

    Sequence(int lineno, SequenceType& nodes) : T(lineno), _nodes(nodes) {}

    Sequence(int lineno, std::list<Item> nodes)
        : T(lineno), _nodes(SequenceType(nodes.begin(), nodes.end())) {}

public:
    Item node(size_t i) { return _nodes[i]; }
    Item last() { return _nodes.back(); }
    const SequenceType& nodes() { return _nodes; }
    size_t size() { return _nodes.size(); }
    void insert(Item item) { _nodes.push_back(item); }
    void insert(const SequenceType& nodes) {
        _nodes.insert(_nodes.end(), nodes.begin(), nodes.end());
    }
    virtual bool accept(Visitor* visitor) override;
};

typedef Sequence<BasicNode*, BasicNode> SequenceNode;
typedef Sequence<ExpressionNode*, ExpressionNode> SequenceExprNode;
typedef Sequence<std::shared_ptr<LiteralNode>, ExpressionNode>
    SequenceLiteralNode;

class BlockNode : public BasicNode {
    SequenceNode* _stmts;

public:
    inline BlockNode(int lineno)
        : BasicNode(lineno), _stmts(new SequenceNode(lineno)) {}

    inline BlockNode(int lineno, SequenceNode* stmts)
        : BasicNode(lineno), _stmts(stmts) {}

public:
    inline SequenceNode* statements() { return _stmts; }

    bool accept(Visitor* visitor) override;
};

class Foreach : public BasicNode {
    std::string _identifier;
    ExpressionNode* _expr;
    BasicNode* _stmt;

public:
    inline Foreach(int lineno,
                   std::string* identifier,
                   ExpressionNode* expr,
                   BasicNode* stmt)
        : BasicNode(lineno),
          _identifier(*identifier),
          _expr(expr),
          _stmt(stmt) {}

public:
    inline std::string identifier() { return _identifier; }

    inline ExpressionNode* expr() { return _expr; }

    inline BasicNode* insts() { return _stmt; }

    bool accept(Visitor* visitor) override;
};

class IfNode : public BasicNode {
    ExpressionNode* _condition;
    BasicNode* _thenStmt;

public:
    inline IfNode(int lineno, ExpressionNode* condition, BasicNode* thenStmt)
        : BasicNode(lineno), _condition(condition), _thenStmt(thenStmt) {}

public:
    inline ExpressionNode* condition() { return _condition; }
    inline BasicNode* block() { return _thenStmt; }

    bool accept(Visitor* visitor) override;
};

class IfElseNode : public BasicNode {
    ExpressionNode* _condition;
    BasicNode *_thenStmt, *_elseStmt;

public:
    inline IfElseNode(int lineno,
                      ExpressionNode* condition,
                      BasicNode* thenStmt,
                      BasicNode* elseStmt)
        : BasicNode(lineno),
          _condition(condition),
          _thenStmt(thenStmt),
          _elseStmt(elseStmt) {}

public:
    inline ExpressionNode* condition() { return _condition; }
    inline BasicNode* thenblock() { return _thenStmt; }
    inline BasicNode* elseblock() { return _elseStmt; }

    bool accept(Visitor* visitor) override;
};

class FunctionNode : public BasicNode {
    std::string _name;
    BlockNode* _block;
    std::vector<std::string> _identifiers;

public:
    inline FunctionNode(int lineno,
                        std::string* name,
                        BlockNode* block,
                        std::vector<std::string>* identifiers = nullptr)
        : BasicNode(lineno), _name(*name), _block(block) {
        if (identifiers != nullptr) {
            _identifiers = *identifiers;
            delete identifiers;
        }
    }

public:
    inline std::string name() { return _name; }
    inline BlockNode* block() { return _block; }
    inline const std::vector<std::string>& identifiers() {
        return _identifiers;
    }

    bool accept(Visitor* visitor) override;
};

class ReturnNode : public BasicNode {
    ExpressionNode* _expr;

public:
    inline ReturnNode(int lineno, ExpressionNode* expr = nullptr)
        : BasicNode(lineno), _expr(expr) {}

public:
    inline ExpressionNode* expr() { return _expr; }

    bool accept(Visitor* visitor) override;
};

class ImportNode : public BasicNode {
    std::string _file;

public:
    inline ImportNode(int lineno, std::string* file)
        : BasicNode(lineno), _file(*file) {}

public:
    inline std::string file() { return _file; }

    bool accept(Visitor* visitor) override;
};

/*** Expressions ***/
class RValueNode : public ExpressionNode {
    std::string _name;

public:
    RValueNode(int lineno, std::string* name)
        : ExpressionNode(lineno), _name(*name) {}

public:
    std::string name() { return _name; }

    bool accept(Visitor* visitor) override;
};

class AssignExpr : public ExpressionNode {
    std::string _identifier;
    ExpressionNode* _expr;

public:
    AssignExpr(int lineno, std::string* identifier, ExpressionNode* expr)
        : ExpressionNode(lineno), _identifier(*identifier), _expr(expr) {}

public:
    std::string identifier() { return _identifier; }
    ExpressionNode* expr() { return _expr; }

    bool accept(Visitor* visitor) override;
};

class FunctionCall : public ExpressionNode {
    std::string _name;
    SequenceExprNode* _parameters;

public:
    inline FunctionCall(int lineno, std::string* name)
        : ExpressionNode(lineno),
          _name(*name),
          _parameters(new SequenceExprNode(lineno)) {}

    inline FunctionCall(int lineno,
                        std::string* name,
                        ExpressionNode* parameter)
        : ExpressionNode(lineno),
          _name(*name),
          _parameters(new SequenceExprNode(lineno, parameter)) {}

    inline FunctionCall(int lineno,
                        std::string* name,
                        SequenceExprNode* parameters)
        : ExpressionNode(lineno), _name(*name), _parameters(parameters) {}

public:
    inline std::string name() { return _name; }

    inline SequenceExprNode* parameters() { return _parameters; }

    bool accept(Visitor* visitor) override;
};

class AttributeCall : public ExpressionNode {
    std::string _name;
    ExpressionNode* _expr;

public:
    inline AttributeCall(int lineno, std::string* name, ExpressionNode* expr)
        : ExpressionNode(lineno), _name(*name), _expr(expr) {}

public:
    inline std::string name() { return _name; }
    inline ExpressionNode* expr() { return _expr; }

    bool accept(Visitor* visitor) override;
};

class MemberFunctionCall : public ExpressionNode {
    std::string _name;
    ExpressionNode* _expr;
    SequenceExprNode* _parameters;

public:
    inline MemberFunctionCall(int lineno,
                              std::string* name,
                              ExpressionNode* expr,
                              SequenceExprNode* parameters = nullptr)
        : ExpressionNode(lineno), _name(*name), _expr(expr) {
        if (parameters == nullptr) {
            _parameters = new SequenceExprNode(lineno);
        } else {
            _parameters = parameters;
        }
    }

public:
    inline std::string name() { return _name; }
    inline ExpressionNode* expr() { return _expr; }
    inline SequenceExprNode* parameters() { return _parameters; }

    bool accept(Visitor* visitor) override;
};

class FilterExprNode : public ExpressionNode {
    std::string _var;
    ExpressionNode* _domain;
    ExpressionNode* _predicate;

public:
    inline FilterExprNode(int lineno,
                          std::string* var,
                          ExpressionNode* domain,
                          ExpressionNode* predicate)
        : ExpressionNode(lineno),
          _var(*var),
          _domain(domain),
          _predicate(predicate) {}

public:
    inline std::string var() { return _var; }
    inline ExpressionNode* domain() { return _domain; }
    inline ExpressionNode* predicate() { return _predicate; }

    bool accept(Visitor* visitor) override;
};

class AtNode : public ExpressionNode {
    ExpressionNode* _expr;
    ExpressionNode* _index;

public:
    inline AtNode(int lineno, ExpressionNode* expr, ExpressionNode* index)
        : ExpressionNode(lineno), _expr(expr), _index(index) {}

public:
    inline ExpressionNode* expr() { return _expr; }

    inline ExpressionNode* index() { return _index; }

    bool accept(Visitor* visitor) override;
};

class TimeNode : public ExpressionNode {
    ExpressionNode* _expr;

public:
    inline TimeNode(int lineno, ExpressionNode* expr)
        : ExpressionNode(lineno), _expr(expr) {}

public:
    inline ExpressionNode* expr() { return _expr; }

    bool accept(Visitor* visitor) override;
};

template <typename StoredType, LiteralType T>
class Literal : public LiteralNode {
    StoredType _value;

public:
    Literal(int lineno) : LiteralNode(lineno) {}

    Literal(int lineno, const StoredType& value)
        : LiteralNode(lineno), _value(value) {}

    Literal(int lineno, Literal<StoredType, T>* lit)
        : LiteralNode(lit->lineno()), _value(lit->value()) {}

public:
    StoredType& value() { return _value; }

    LiteralType type() override { return T; }

    bool equals(std::shared_ptr<LiteralNode> other) override {
        if (this->type() != other->type()) {
            return false;
        }
        auto o = std::dynamic_pointer_cast<Literal<StoredType, T>>(other);
        return this->value() == o->value();
    }

    bool accept(Visitor* visitor) override;
};

typedef Literal<long int, LiteralType::Int> IntNode;
typedef Literal<double, LiteralType::Float> FloatNode;
typedef Literal<std::string, LiteralType::String> StringNode;
typedef Literal<bool, LiteralType::Bool> BoolNode;
typedef Literal<void*, LiteralType::Nil> NilNode;
typedef Literal<void*, LiteralType::None> NoneNode;
typedef Literal<std::shared_ptr<SequenceLiteralNode>, LiteralType::List>
    ListNode;
typedef Literal<std::map<std::string, std::shared_ptr<LiteralNode>>,
                LiteralType::Map>
    MapNode;
typedef Literal<Node*, LiteralType::Node> NodePointer;
typedef Literal<Edge*, LiteralType::Edge> EdgePointer;

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

    virtual BinopType type() = 0;
};

template <BinopType T>
class BinopExprNode : public BinopNode {
public:
    BinopExprNode(int lineno, ExpressionNode* left, ExpressionNode* right)
        : BinopNode(lineno, left, right) {}

    BinopType type() override { return T; }

    bool accept(Visitor* visitor) override;
};

typedef BinopExprNode<BinopType::Equal> EqualNode;
typedef BinopExprNode<BinopType::NotEqual> NotEqualNode;
typedef BinopExprNode<BinopType::And> AndNode;
typedef BinopExprNode<BinopType::Or> OrNode;
typedef BinopExprNode<BinopType::In> InNode;
typedef BinopExprNode<BinopType::Less> LessNode;
typedef BinopExprNode<BinopType::LessEqual> LessEqualNode;
typedef BinopExprNode<BinopType::Greater> GreaterNode;
typedef BinopExprNode<BinopType::GreaterEqual> GreaterEqualNode;
typedef BinopExprNode<BinopType::Add> AddNode;
typedef BinopExprNode<BinopType::Sub> SubNode;
typedef BinopExprNode<BinopType::Mul> MulNode;
typedef BinopExprNode<BinopType::Div> DivNode;
typedef BinopExprNode<BinopType::Mod> ModNode;

class UnopNode : public ExpressionNode {
    ExpressionNode* _argument;

public:
    UnopNode(int lineno, ExpressionNode* arg)
        : ExpressionNode(lineno), _argument(arg) {}

    ExpressionNode* argument() { return _argument; }

    virtual UnopType type() = 0;
};

template <UnopType T>
class UnopExprNode : public UnopNode {
public:
    UnopExprNode(int lineno, ExpressionNode* argument)
        : UnopNode(lineno, argument) {}

    UnopType type() override { return T; }

    bool accept(Visitor* visitor) override;
};
typedef UnopExprNode<UnopType::Not> NotNode;

class Visitor {
public:
    // Statements
    virtual bool visitBlockNode(BlockNode* node) = 0;
    virtual bool visitForeach(Foreach* node) = 0;
    virtual bool visitIfNode(IfNode* node) = 0;
    virtual bool visitIfElseNode(IfElseNode* node) = 0;
    virtual bool visitFunction(FunctionNode* node) = 0;
    virtual bool visitReturnNode(ReturnNode* node) = 0;
    virtual bool visitImportNode(ImportNode* node) = 0;
    // basic expr
    virtual bool visitAssignExpr(AssignExpr* node) = 0;
    virtual bool visitRValueNode(RValueNode* node) = 0;
    virtual bool visitFunctionCall(FunctionCall* node) = 0;
    virtual bool visitAttributeCall(AttributeCall* node) = 0;
    virtual bool visitMemberFunctionCall(MemberFunctionCall* node) = 0;
    virtual bool visitFilterExprNode(FilterExprNode* node) = 0;
    virtual bool visitAtNode(AtNode* node) = 0;
    virtual bool visitTimeNode(TimeNode* node) = 0;
    // objects
    virtual bool visitNodePointer(NodePointer* node) = 0;
    virtual bool visitEdgePointer(EdgePointer* node) = 0;
    // literals
    virtual bool visitIntNode(IntNode* node) = 0;
    virtual bool visitFloatNode(FloatNode* node) = 0;
    virtual bool visitBoolNode(BoolNode* node) = 0;
    virtual bool visitStringNode(StringNode* node) = 0;
    virtual bool visitListNode(ListNode* node) = 0;
    virtual bool visitMapNode(MapNode* node) = 0;
    virtual bool visitNilNode(NilNode* node) = 0;
    virtual bool visitNoneNode(NoneNode* node) = 0;
    // binary expr
    virtual bool visitEqualNode(EqualNode* node) = 0;
    virtual bool visitNotEqualNode(NotEqualNode* node) = 0;
    virtual bool visitAndNode(AndNode* node) = 0;
    virtual bool visitOrNode(OrNode* node) = 0;
    virtual bool visitInNode(InNode* node) = 0;
    virtual bool visitLessNode(LessNode* node) = 0;
    virtual bool visitLessEqualNode(LessEqualNode* node) = 0;
    virtual bool visitGreaterNode(GreaterNode* node) = 0;
    virtual bool visitGreaterEqualNode(GreaterEqualNode* node) = 0;
    virtual bool visitAddNode(AddNode* node) = 0;
    virtual bool visitSubNode(SubNode* node) = 0;
    virtual bool visitMulNode(MulNode* node) = 0;
    virtual bool visitDivNode(DivNode* node) = 0;
    virtual bool visitModNode(ModNode* node) = 0;
    // unary expr
    virtual bool visitNotNode(NotNode* node) = 0;
    // sequences
    virtual bool visitSequenceNode(SequenceNode* node) = 0;
    virtual bool visitSequenceExprNode(SequenceExprNode* node) = 0;
    virtual bool visitSequenceLiteralNode(SequenceLiteralNode* node) = 0;
};
}  // namespace wasmati

#endif  // WASMATI_NODES_H
