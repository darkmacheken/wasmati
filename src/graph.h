#ifndef WASMATI_GRAPH_H
#define WASMATI_GRAPH_H
#include "src/common.h"
#include "src/ir.h"
#include "src/ir-util.h"
#include "src/decompiler-ast.h"
#include "src/graph-visitor.h"

using namespace wabt;
namespace wasmati {

class Edge;

struct GenerateASTOptions {
	std::string funcName;
};

class Node {
	static int idCount;
	std::vector<Edge*> _edges;
public:
	int _id;

	explicit Node() : _id(idCount++) {}
	virtual ~Node();

	inline int getId() {
		return _id;
	}

	inline std::vector<Edge*> getEdges() {
		return _edges;
	}

	inline void addEdge(Edge* e) {
		_edges.push_back(e);
	}

	virtual void accept(GraphVisitor* visitor) { assert(false); }

	virtual void acceptEdges(GraphVisitor* visitor);
};

class Module : public Node {
	std::string _name;
public:
	Module() {}
	Module(std::string name) : _name(name) {}

	inline std::string getName() {
		return _name;
	}

	void accept(GraphVisitor* visitor) { visitor->visitModule(this); }
};

class Function : public Node {
	std::string _name;
public:
	Function() {}
	Function(std::string name) : _name(name) {}

	inline std::string getName() {
		return _name;
	}

	void accept(GraphVisitor* visitor) { visitor->visitFunction(this); }
};

class TypeNode : public Node {
	Type _type;
	std::string _name;
public:
	TypeNode(Type type, std::string name = "") : _type(type), _name(name) {}

	inline Type getType() { return _type; }
	inline std::string getName() { return _name; }

	void accept(GraphVisitor* visitor) { visitor->visitTypeNode(this); }

};

class SimpleNode : public Node {
	std::string _nodeName;
public:
	SimpleNode(std::string nodeName) : _nodeName(nodeName) {}

	inline std::string getNodeName() { return _nodeName; }

	virtual void accept(GraphVisitor* visitor) { visitor->visitSimpleNode(this); }

};

class FunctionSignature : public SimpleNode {
public:
	FunctionSignature() : SimpleNode("FunctionSignature"){}
};

class Instructions : public SimpleNode {
public:
	Instructions() : SimpleNode("Instructions") {}
};

class Parameters : public SimpleNode {
public:
	Parameters() : SimpleNode("Parameters") {}
};

class Locals : public SimpleNode {
public:
	Locals() : SimpleNode("Locals") {}
};

class Results : public SimpleNode {
public:
	Results() : SimpleNode("Results") {}
};

class Return : public SimpleNode {
public:
	Return() : SimpleNode("Return") {}
};

class Instruction : public Node {
	ExprType _type;
	const Expr* _expr;
public:
	Instruction(ExprType type, const Expr* expr) : _type(type), _expr(expr) {}

	inline const Expr* getExpr() { return _expr; }

	void accept(GraphVisitor* visitor) { visitor->visitInstruction(this); }
};

class IndexNode : public Node {
	Index _index;
public:
	IndexNode(Index index) : _index(index) {}

	inline Index getIndex() { return _index; }

	void accept(GraphVisitor* visitor) { visitor->visitIndexNode(this); }
};

struct Edge {
	Node* _src;
	Node* _dest;

	Edge(Node* src, Node* dest) : _src(src), _dest(dest) {
		src->addEdge(this);
	}

	inline Node* getDest() {
		return _dest;
	}

	virtual ~Edge() {}

	virtual void accept(GraphVisitor* visitor) = 0;
};

struct ASTEdge : Edge {
	ASTEdge(Node* src, Node* dest) : Edge(src, dest) {}
	void accept(GraphVisitor* visitor) { visitor->visitASTEdge(this); }
};

struct CFGEdge :  Edge {
	CFGEdge(Node* src, Node* dest) : Edge(src, dest) {}
	void accept(GraphVisitor* visitor) { visitor->visitCFGEdge(this); }
};

struct PDGEdge :  Edge {
	PDGEdge(Node* src, Node* dest) : Edge(src, dest) {}
	void accept(GraphVisitor* visitor) { visitor->visitPDGEdge(this); }
};


class Graph {
	wabt::ModuleContext* _mc;
	std::vector<Node*> _nodes;

	void getLocalsNames(Func* f, std::vector<std::string>* names);

public:
	Graph(wabt::Module* mc);
	~Graph() {
		for (auto node : _nodes) {
			delete node;
		}
		delete _mc;
	}

	inline void insertNode(Node* node) {_nodes.push_back(node);}
	inline std::vector<Node*>* getNodes() { return &_nodes; }

	void generateAST(GenerateASTOptions options);
	void visitWabtNode(wasmati::Node* parentNode, wabt::Node* node);
	std::string cellRepr(std::string content) {
		return "<TR><TD>" + content + "</TD></TR>";
	}
	std::string writeConst(const Const& const_);

};


}  // namespace wasmati

#endif /* WASMATI_GRAPH_H*/
