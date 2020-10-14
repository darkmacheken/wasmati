#include "src/graph.h"

namespace wasmati {

const char functionSignatureName[] = "FunctionSignature";
const char instructionsName[] = "Instructions";
const char parametersName[] = "Parameters";
const char localsName[] = "Locals";
const char resultsName[] = "Results";
const char elseName[] = "Else";
const char trapName[] = "Trap";
const char startName[] = "Start";

Index Node::idCount = 0;

Node::~Node() {
    for (auto e : _outEdges) {
        delete e;
    }
}

EdgeSet Node::inEdges(EdgeType type) {
    EdgeSet res;
    for (auto e : _inEdges) {
        if (e->type() == type) {
            res.insert(e);
        }
    }
    return res;
}

EdgeSet Node::outEdges(EdgeType type) {
    EdgeSet res;
    for (auto e : _outEdges) {
        if (e->type() == type) {
            res.insert(e);
        }
    }
    return res;
}

inline Edge* Node::getOutEdge(Index i, EdgeType type) {
    auto edges = outEdges(type);
    assert(i < edges.size());
    std::vector<Edge*> vec(edges.begin(), edges.end());
    std::sort(vec.begin(), vec.end(), [](Edge* a, Edge* b) {
        return a->dest()->getId() < b->dest()->getId();
    });
    return vec[i];
}

inline Edge* Node::getInEdge(Index i, EdgeType type) {
    auto edges = inEdges(type);
    assert(i < edges.size());
    std::vector<Edge*> vec(edges.begin(), edges.end());
    std::sort(vec.begin(), vec.end(), [](Edge* a, Edge* b) {
        return a->src()->getId() < b->src()->getId();
    });

    return vec[i];
}

Node* Node::getChild(Index n, EdgeType type) {
    return getOutEdge(n, type)->dest();
}

Node* Node::getParent(Index n, EdgeType type) {
    return getInEdge(n, type)->src();
}

bool Node::hasEdgesOf(EdgeType type) const {
    return hasInEdgesOf(type) || hasOutEdgesOf(type);
}

bool Node::hasInEdgesOf(EdgeType type) const {
    for (auto e : _inEdges) {
        if (e->type() == type) {
            return true;
        }
    }
    return false;
}

bool Node::hasOutEdgesOf(EdgeType type) const {
    for (auto e : _outEdges) {
        if (e->type() == type) {
            return true;
        }
    }
    return false;
}

void Node::accept(GraphVisitor* visitor) {
    assert(false);
}

void Node::acceptEdges(GraphVisitor* visitor) {
    for (Edge* e : _outEdges) {
        e->accept(visitor);
    }
}

Graph::Graph(wabt::Module& mc) : _mc(ModuleContext(mc)) {
    _trap = nullptr;
}

Graph::~Graph() {
    for (auto node : _nodes) {
        delete node;
    }
}
void Module::accept(GraphVisitor* visitor) {
    visitor->visitModule(this);
}
void Function::accept(GraphVisitor* visitor) {
    visitor->visitFunction(this);
}
void VarNode::accept(GraphVisitor* visitor) {
    visitor->visitVarNode(this);
}

void ConstInst::accept(GraphVisitor* visitor) {
    visitor->visitConstInst(this);
}

void IfInst::accept(GraphVisitor* visitor) {
    visitor->visitIfInst(this);
}

void ASTEdge::accept(GraphVisitor* visitor) {
    visitor->visitASTEdge(this);
}

void CFGEdge::accept(GraphVisitor* visitor) {
    visitor->visitCFGEdge(this);
}

void PDGEdge::accept(GraphVisitor* visitor) {
    visitor->visitPDGEdge(this);
}

void CGEdge::accept(GraphVisitor* visitor) {
    visitor->visitCGEdge(this);
}

void PGEdge::accept(GraphVisitor* visitor) {
    visitor->visitPGEdge(this);
}

void BeginBlockInst::accept(GraphVisitor* visitor) {
    visitor->visitBeginBlockInst(this);
}

bool Compare::operator()(Node* const& n1, Node* const& n2) const {
    return n1->getId() < n2->getId();
}
}  // namespace wasmati
