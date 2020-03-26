#ifndef WASMATI_GRAPH_H
#define WASMATI_GRAPH_H
#include "src/cast.h"
#include "src/common.h"
#include "src/decompiler-ast.h"
#include "src/graph-visitor.h"
#include "src/ir-util.h"
#include "src/ir.h"

using namespace wabt;
namespace wasmati {

class Edge;

enum class EdgeType { AST, CFG, PDG };

enum class NodeType {
    Module,
    Function,
    TypeNode,
    FunctionSignature,
    Instructions,
    Instruction,
    Parameters,
    Locals,
    Results,
    Return,
    Else,
    Trap,
    Start,
    IndexNode
};

class Node {
    static int idCount;
    int _id;
    std::vector<Edge*> _inEdges;
    std::vector<Edge*> _outEdges;

public:
    const NodeType _type;

    explicit Node(NodeType type) : _id(idCount++), _type(type) {}
    virtual ~Node();

    inline int getId() { return _id; }
    inline std::vector<Edge*>* inEdges() { return &_inEdges; }
    inline std::vector<Edge*>* outEdges() { return &_outEdges; }

    inline Edge* getOutEdge(Index i) {
        assert(i < _outEdges.size());
        return _outEdges[i];
    }

    inline int getNumOutEdges() { return _outEdges.size(); }
    inline Edge* getInEdge(Index i) {
        assert(i < _inEdges.size());
        return _inEdges[i];
    }

    inline int getNumInEdges() { return _inEdges.size(); }
    inline void addInEdge(Edge* e) { _inEdges.push_back(e); }
    inline void addOutEdge(Edge* e) { _outEdges.push_back(e); }

    bool hasEdgesOf(EdgeType);
    bool hasInEdgesOf(EdgeType);
    bool hasOutEdgesOf(EdgeType);

    virtual void accept(GraphVisitor* visitor) { assert(false); }
    virtual void acceptEdges(GraphVisitor* visitor);
};

class Module : public Node {
    std::string _name;

public:
    Module() : Node(NodeType::Module) {}
    Module(std::string name) : Node(NodeType::Module), _name(name) {}

    static bool classof(const Node* node) {
        return node->_type == NodeType::Module;
    }

    inline std::string getName() { return _name; }

    void accept(GraphVisitor* visitor) { visitor->visitModule(this); }
};

class Function : public Node {
    std::string _name;

public:
    Function() : Node(NodeType::Function) {}
    Function(std::string name) : Node(NodeType::Function), _name(name) {}

    static bool classof(const Node* node) {
        return node->_type == NodeType::Function;
    }

    inline std::string getName() { return _name; }

    void accept(GraphVisitor* visitor) { visitor->visitFunction(this); }
};

class TypeNode : public Node {
    Type __type;
    std::string _name;

public:
    TypeNode(Type type, std::string name = "")
        : Node(NodeType::TypeNode), __type(type), _name(name) {}

    static bool classof(const Node* node) {
        return node->_type == NodeType::TypeNode;
    }

    inline Type getType() { return __type; }
    inline std::string getName() { return _name; }

    void accept(GraphVisitor* visitor) { visitor->visitTypeNode(this); }
};

class SimpleNode : public Node {
    std::string _nodeName;

public:
    SimpleNode(NodeType type, std::string nodeName)
        : Node(type), _nodeName(nodeName) {}

    static bool classof(const Node* node) { return false; }

    inline std::string getNodeName() { return _nodeName; }

    virtual void accept(GraphVisitor* visitor) {
        visitor->visitSimpleNode(this);
    }
};

class FunctionSignature : public SimpleNode {
public:
    FunctionSignature()
        : SimpleNode(NodeType::FunctionSignature, "FunctionSignature") {}

    static bool classof(const Node* node) {
        return node->_type == NodeType::FunctionSignature;
    }
};

class Instructions : public SimpleNode {
public:
    Instructions() : SimpleNode(NodeType::Instructions, "Instructions") {}

    virtual void accept(GraphVisitor* visitor) {
        visitor->visitInstructions(this);
    }

    static bool classof(const Node* node) {
        return node->_type == NodeType::Instructions;
    }
};

class Parameters : public SimpleNode {
public:
    Parameters() : SimpleNode(NodeType::Parameters, "Parameters") {}

    static bool classof(const Node* node) {
        return node->_type == NodeType::Parameters;
    }
};

class Locals : public SimpleNode {
public:
    Locals() : SimpleNode(NodeType::Locals, "Locals") {}

    static bool classof(const Node* node) {
        return node->_type == NodeType::Locals;
    }
};

class Results : public SimpleNode {
public:
    Results() : SimpleNode(NodeType::Results, "Results") {}

    static bool classof(const Node* node) {
        return node->_type == NodeType::Results;
    }
};

class Return : public SimpleNode {
public:
    Return() : SimpleNode(NodeType::Return, "Return") {}

    virtual void accept(GraphVisitor* visitor) { visitor->visitReturn(this); }

    static bool classof(const Node* node) {
        return node->_type == NodeType::Return;
    }
};

class Else : public SimpleNode {
public:
    Else() : SimpleNode(NodeType::Else, "Else") {}

    virtual void accept(GraphVisitor* visitor) { visitor->visitElse(this); }

    static bool classof(const Node* node) {
        return node->_type == NodeType::Else;
    }
};

class Trap : public SimpleNode {
public:
    Trap() : SimpleNode(NodeType::Trap, "Trap") {}

    virtual void accept(GraphVisitor* visitor) { visitor->visitTrap(this); }

    static bool classof(const Node* node) {
        return node->_type == NodeType::Trap;
    }
};

class Start : public SimpleNode {
public:
    Start() : SimpleNode(NodeType::Start, "Start") {}

    virtual void accept(GraphVisitor* visitor) { visitor->visitStart(this); }

    static bool classof(const Node* node) {
        return node->_type == NodeType::Start;
    }
};

class Instruction : public Node {
    ExprType __type;
    Expr* _expr;

public:
    Instruction(ExprType type, Expr* expr)
        : Node(NodeType::Instruction), __type(type), _expr(expr) {}

    static bool classof(const Node* node) {
        return node->_type == NodeType::Instruction;
    }

    inline Expr* getExpr() { return _expr; }

    inline ExprType getType() { return __type; }

    void accept(GraphVisitor* visitor) { visitor->visitInstruction(this); }
};

class IndexNode : public Node {
    Index _index;

public:
    IndexNode(Index index) : Node(NodeType::IndexNode), _index(index) {}

    static bool classof(const Node* node) {
        return node->_type == NodeType::IndexNode;
    }

    inline Index getIndex() { return _index; }

    void accept(GraphVisitor* visitor) { visitor->visitIndexNode(this); }
};

struct Edge {
private:
    Node* _src;
    Node* _dest;
    EdgeType _type;

public:
    Edge(Node* src, Node* dest, EdgeType type)
        : _src(src), _dest(dest), _type(type) {
        assert(src != nullptr && dest != nullptr);
        src->addOutEdge(this);
        dest->addInEdge(this);
    }

    virtual ~Edge() {}

    inline Node* src() { return _src; }
    inline Node* dest() { return _dest; }
    inline EdgeType type() { return _type; }
    virtual void accept(GraphVisitor* visitor) = 0;
};

struct ASTEdge : Edge {
    ASTEdge(Node* src, Node* dest) : Edge(src, dest, EdgeType::AST) {}
    void accept(GraphVisitor* visitor) { visitor->visitASTEdge(this); }
};

struct CFGEdge : Edge {
    std::string _label;

    CFGEdge(Node* src, Node* dest) : Edge(src, dest, EdgeType::CFG) {}
    CFGEdge(Node* src, Node* dest, std::string label)
        : Edge(src, dest, EdgeType::CFG), _label(label) {}
    void accept(GraphVisitor* visitor) { visitor->visitCFGEdge(this); }
};

struct PDGEdge : Edge {
    PDGEdge(Node* src, Node* dest) : Edge(src, dest, EdgeType::PDG) {}
    void accept(GraphVisitor* visitor) { visitor->visitPDGEdge(this); }
};

class Graph {
    wabt::ModuleContext* _mc;
    std::vector<Node*> _nodes;
    Trap* _trap;
    Start* _start;

    void getLocalsNames(Func* f, std::vector<std::string>* names);
    void generateAST(GenerateCPGOptions options);
    void generateCFG(GenerateCPGOptions options);
    void visitWabtNode(wasmati::Node* parentNode, wabt::Node* node);

public:
    Graph(wabt::Module* mc);
    ~Graph();

    inline void insertNode(Node* node) { _nodes.push_back(node); }
    inline std::vector<Node*>* getNodes() { return &_nodes; }
    inline Trap* getTrap() {
        if (_trap == nullptr) {
            _trap = new Trap();
            this->insertNode(_trap);
        }
        return _trap;
    }
    inline Start* getStart() {
        if (_start == nullptr) {
            _start = new Start();
            this->insertNode(_start);
        }
        return _start;
    }

    void generateCPG(GenerateCPGOptions options);
};

}  // namespace wasmati

#endif /* WASMATI_GRAPH_H*/
