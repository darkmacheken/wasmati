#ifndef WASMATI_GRAPH_H
#define WASMATI_GRAPH_H
#include "src/common.h"
#include "src/cast.h"
#include "src/ir.h"
#include "src/ir-util.h"
#include "src/decompiler-ast.h"
#include "src/graph-visitor.h"

using namespace wabt;
namespace wasmati {

class Edge;

enum class EdgeType {
	AST,
	CFG,
	PDG
};

class Node {
	static int idCount;
	std::vector<Edge*> _edges;
	bool _hasASTEdges;
	bool _hasCFGEdges;
	bool _hasPDGEdges;
public:
	int _id;

	explicit Node() : _id(idCount++) { _hasCFGEdges = false; }
	virtual ~Node();

	inline int getId() {
		return _id;
	}

	inline std::vector<Edge*> getEdges() {
		return _edges;
	}

	inline Edge* getEdge(Index i) {
		if (i >= _edges.size()) {
			assert(false);
		}
		return _edges[i];
	}

	inline int getNumEdges() {
		return _edges.size();
	}

	void addEdge(Edge* e);

	inline bool hasASTEdges() { return _hasASTEdges; }
	inline bool hasCFGEdges() { return _hasCFGEdges; }
	inline bool hasPDGEdges() { return _hasPDGEdges; }

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

	virtual void accept(GraphVisitor* visitor) { visitor->visitInstructions(this); }
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

	virtual void accept(GraphVisitor* visitor) { visitor->visitReturn(this); }
};

class Else : public SimpleNode {
public:
	Else() : SimpleNode("Else") {}

	virtual void accept(GraphVisitor* visitor) { visitor->visitElse(this); }
};

class Trap : public SimpleNode {
public:
	Trap() : SimpleNode("Trap") {}

	virtual void accept(GraphVisitor* visitor) { visitor->visitTrap(this); }
};

class Start : public SimpleNode {
public:
	Start() : SimpleNode("Start") {}

	virtual void accept(GraphVisitor* visitor) { visitor->visitStart(this); }
};

class Instruction : public Node {
	ExprType _type;
	Expr* _expr;
public:
	Instruction(ExprType type, Expr* expr) : _type(type), _expr(expr) {}

	inline Expr* getExpr() { return _expr; }

	inline ExprType getType() { return _type; }

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
	EdgeType _type;

	Edge(Node* src, Node* dest, EdgeType type) : _src(src), _dest(dest), _type(type) {
		src->addEdge(this);
	}

	inline Node* getDest() {
		return _dest;
	}

	inline EdgeType getType() {
		return _type;
	}

	virtual ~Edge() {}

	virtual void accept(GraphVisitor* visitor) = 0;
};

struct ASTEdge : Edge {
	ASTEdge(Node* src, Node* dest) : Edge(src, dest, EdgeType::AST) {}
	void accept(GraphVisitor* visitor) { visitor->visitASTEdge(this); }
};

struct CFGEdge :  Edge {
	std::string _label;

	CFGEdge(Node* src, Node* dest) : Edge(src, dest, EdgeType::CFG) {}
	CFGEdge(Node* src, Node* dest, std::string label) : Edge(src, dest, EdgeType::CFG), _label(label) {}
	void accept(GraphVisitor* visitor) { visitor->visitCFGEdge(this); }
};

struct PDGEdge :  Edge {
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

	inline void insertNode(Node* node) {_nodes.push_back(node);}
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
