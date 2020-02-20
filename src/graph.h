#ifndef WABT_GRAPH_H
#define WABT_GRAPH_H
#include "src/common.h"
#include "src/ir.h"
#include "src/ir-util.h"
#include "src/stream.h"
#include "src/expr-visitor.h"
#include "src/decompiler-ast.h"

using namespace wabt;
namespace wasmati {

class Edge;

enum class NodeType {
	Module,
	Instructions,
	Function,
	IndexNode,
	FunctionSignature,
	Decl,
	DeclInit,
	Expr
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

class Function : public Node {
	std::string _name;
public:
	Function() {}
	Function(std::string name) : _name(name) {}

	inline std::string getName() {
		return _name;
	}

	std::string toString() override;
};

class NamedNode : public Node {
	std::string _name;
public:
	NamedNode(std::string name) : _name(name) {}

	inline std::string getName() {
		return _name;
	}

	std::string toString() override;
};

class TypeNode : public Node {
	Type _type;
public:
	TypeNode(Type type) : _type(type) {}

	std::string toString() override;
};

class SimpleNode : public Node {
	std::string _nodeName;
public:
	SimpleNode(std::string nodeName) : _nodeName(nodeName) {}

	virtual std::string toString() override;
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

template <ExprType T>
class Instruction : public Node {
	ExprType _type;
	std::string _representation;

public:
	Instruction(std::string representation) : _type(T), _representation(representation) {}
	std::string toString() override;
};

class IndexNode : public Node {
	Index _index;
public:
	IndexNode(Index index) : _index(index) {}

	std::string toString() override;
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


class Graph {
public:
	class ExprVisitor {
	public:
		explicit ExprVisitor(Graph* graph) : _graph(graph) {}

		Node* visitExpr(Expr* expr);

		Node* OnBinaryExpr(BinaryExpr*);
		Node* OnBlockExpr(BlockExpr*);
		Node* OnBrExpr(BrExpr*);
		Node* OnBrIfExpr(BrIfExpr*);
		Node* OnBrOnExnExpr(BrOnExnExpr*);
		Node* OnBrTableExpr(BrTableExpr*);
		Node* OnCallExpr(CallExpr*);
		Node* OnCallIndirectExpr(CallIndirectExpr*);
		Node* OnCompareExpr(CompareExpr*);
		Node* OnConstExpr(ConstExpr*);
		Node* OnConvertExpr(ConvertExpr*);
		Node* OnDropExpr(DropExpr*);
		Node* OnGlobalGetExpr(GlobalGetExpr*);
		Node* OnGlobalSetExpr(GlobalSetExpr*);
		Node* OnIfExpr(IfExpr*);
		Node* OnLoadExpr(LoadExpr*);
		Node* OnLocalGetExpr(LocalGetExpr*);
		Node* OnLocalSetExpr(LocalSetExpr*);
		Node* OnLocalTeeExpr(LocalTeeExpr*);
		Node* OnLoopExpr(LoopExpr*);
		Node* OnMemoryCopyExpr(MemoryCopyExpr*);
		Node* OnDataDropExpr(DataDropExpr*);
		Node* OnMemoryFillExpr(MemoryFillExpr*);
		Node* OnMemoryGrowExpr(MemoryGrowExpr*);
		Node* OnMemoryInitExpr(MemoryInitExpr*);
		Node* OnMemorySizeExpr(MemorySizeExpr*);
		Node* OnTableCopyExpr(TableCopyExpr*);
		Node* OnElemDropExpr(ElemDropExpr*);
		Node* OnTableInitExpr(TableInitExpr*);
		Node* OnTableGetExpr(TableGetExpr*);
		Node* OnTableSetExpr(TableSetExpr*);
		Node* OnTableGrowExpr(TableGrowExpr*);
		Node* OnTableSizeExpr(TableSizeExpr*);
		Node* OnTableFillExpr(TableFillExpr*);
		Node* OnRefFuncExpr(RefFuncExpr*);
		Node* OnRefNullExpr(RefNullExpr*);
		Node* OnRefIsNullExpr(RefIsNullExpr*);
		Node* OnNopExpr(NopExpr*);
		Node* OnReturnExpr(ReturnExpr*);
		Node* OnReturnCallExpr(ReturnCallExpr*);
		Node* OnReturnCallIndirectExpr(ReturnCallIndirectExpr*);
		Node* OnSelectExpr(SelectExpr*);
		Node* OnStoreExpr(StoreExpr*);
		Node* OnUnaryExpr(UnaryExpr*);
		Node* OnUnreachableExpr(UnreachableExpr*);
		Node* OnTryExpr(TryExpr*);
		Node* OnThrowExpr(ThrowExpr*);
		Node* OnRethrowExpr(RethrowExpr*);
		Node* OnAtomicWaitExpr(AtomicWaitExpr*);
		Node* OnAtomicNotifyExpr(AtomicNotifyExpr*);
		Node* OnAtomicLoadExpr(AtomicLoadExpr*);
		Node* OnAtomicStoreExpr(AtomicStoreExpr*);
		Node* OnAtomicRmwExpr(AtomicRmwExpr*);
		Node* OnAtomicRmwCmpxchgExpr(AtomicRmwCmpxchgExpr*);
		Node* OnTernaryExpr(TernaryExpr*);
		Node* OnSimdLaneOpExpr(SimdLaneOpExpr*);
		Node* OnSimdShuffleOpExpr(SimdShuffleOpExpr*);
		Node* OnLoadSplatExpr(LoadSplatExpr*);

	private:
		Graph* _graph;
	};

private:
	wabt::Stream* _stream;
	wabt::ModuleContext* _mc;
	std::vector<Node*> _nodes;
	ExprVisitor _visitor;

	void writePuts(wabt::Stream* stream, const char* s);
	void writeString(wabt::Stream* stream, const std::string& str);

public:
	Graph(wabt::Module* mc);
	~Graph() {
		for (auto node : _nodes) {
			delete node;
		}
		delete _mc;
	}

	inline void insertNode(Node* node) {_nodes.push_back(node);}
	void generateAST();
	void visitWabtNode(wasmati::Node* parentNode, wabt::Node* node);
	wabt::Result writeGraph(wabt::Stream* stream);
	std::string cellRepr(std::string content) {
		return "<TR><TD>" + content + "</TD></TR>";
	}
	std::string writeConst(const Const& const_);

};


}  // namespace wasmati

#endif /* WABT_GRAPH_H*/
