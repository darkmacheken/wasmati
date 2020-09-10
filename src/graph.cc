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

std::vector<Edge*> Node::inEdges(EdgeType type) {
    std::vector<Edge*> res;
    for (auto e : _inEdges) {
        if (e->type() == type) {
            res.push_back(e);
        }
    }
    return res;
}

std::vector<Edge*> Node::outEdges(EdgeType type) {
    std::vector<Edge*> res;
    for (auto e : _outEdges) {
        if (e->type() == type) {
            res.push_back(e);
        }
    }
    return res;
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

Graph::Graph(wabt::Module* mc) : _mc(new ModuleContext(*mc)) {
    _trap = nullptr;
}

Graph::~Graph() {
    for (auto node : _nodes) {
        delete node;
    }
    delete _mc;
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

void BeginBlockInst::accept(GraphVisitor* visitor) {
    visitor->visitBeginBlockInst(this);
}

}  // namespace wasmati
