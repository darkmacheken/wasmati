#ifndef WABT_GRAPH_H
#define WABT_GRAPH_H
#include "src/common.h"
#include "src/ir.h"
#include "src/stream.h"

namespace wasmati {

class Node;

enum class Color {
	BLACK,
	GREEN,
	RED,
	BLUE
};

enum class NodeType {
	Module,
	Instructions,
	Function,
	Decl,
	DeclInit,
	Expr
};


struct Edge {
	Node* _src;
	Node* _dest;

	Edge(Node* src, Node* dest) : _src(src), _dest(dest) {}

	inline Node* getDest() {
		return _dest;
	}

	virtual ~Edge() {}

	virtual std::string toString();

};

struct ASTEdge : Edge {
	ASTEdge(Node* src, Node* dest) : Edge(src, dest) {}
	std::string toString();
};

struct CFGEdge :  Edge {
	CFGEdge(Node* src, Node* dest) : Edge(src, dest) {}
	std::string toString();
};

struct PDGEdge :  Edge {
	PDGEdge(Node* src, Node* dest) : Edge(src, dest) {}
	std::string toString();
};

class Node {
	static int idCount;
	std::vector<Edge*> _edges;
public:
	int _id;

	explicit Node() : _id(idCount++) {}
	virtual ~Node() {
		for (auto e : _edges) {
			delete e;
		}
	}

	inline int getId() {
		return _id;
	}

	inline std::vector<Edge*> getEdges() {
		return _edges;
	}

	inline void addEdge(Edge* e) {
		_edges.push_back(e);
	}

	virtual std::string toString() {
		return "";
	}

	std::string edgesToString();
};

class Module : public Node {
	std::string _name;
public:
	Module() {}
	Module(std::string name) : _name(name) {}

	inline std::string getName() {
		return _name;
	}

	std::string toString() override;
};

class Function : public Node{
	std::string _name;
public:
	Function() {}
	Function(std::string name) : _name(name) {}

	inline std::string getName() {
		return _name;
	}

	std::string toString() override;
};

class Graph {
	wabt::Stream* _stream;
	wabt::Module* _mc;
	std::vector<Node*> _nodes;

	inline void insertNode(Node* node) {
		_nodes.push_back(node);
	}

	void writePuts(wabt::Stream* stream, const char* s);
	void writeString(wabt::Stream* stream, const std::string& str);
public:
	Graph(wabt::Module* mc) : _mc(mc) {}

	~Graph() {
		for (auto node : _nodes) {
			delete node;
		}
	}

	void generateAST();

	wabt::Result writeGraph(wabt::Stream* stream);
};

}  // namespace wasmati

#endif /* WABT_GRAPH_H*/
