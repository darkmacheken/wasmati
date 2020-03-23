#include "src/graph.h"
#include "src/cfg-visitor.h"

namespace wasmati {

int Node::idCount = 0;

Node::~Node() {
    for (auto e : _edges) {
        delete e;
    }
}

void Node::addEdge(Edge* e) {
    switch (e->getType()) {
    case EdgeType::AST:
        _hasASTEdges = e->getDest()->_hasASTEdges = true;
        break;
    case EdgeType::CFG:
        _hasCFGEdges = e->getDest()->_hasCFGEdges = true;
        break;
    case EdgeType::PDG:
        _hasPDGEdges = e->getDest()->_hasPDGEdges = true;
        break;
    default:
        assert(false);
        break;
    }
    _edges.push_back(e);
}

void Node::acceptEdges(GraphVisitor* visitor) {
    for (Edge* e : _edges) {
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

void Graph::generateCPG(GenerateCPGOptions options) {
    generateAST(options);

    if (options.printJustAST) {
        return;
    }

    generateCFG(options);
}

void Graph::generateAST(GenerateCPGOptions options) {
    Module* m;
    if (_mc->module.name.empty()) {
        m = new Module();
    } else {
        m = new Module(_mc->module.name);
    }
    this->insertNode(m);

    // Code
    Index func_index = 0;
    for (auto f : _mc->module.funcs) {
        if (!options.funcName.empty() &&
            options.funcName.compare(f->name) != 0) {
            func_index++;
            continue;
        }
        auto isImport =
            _mc->module.IsImport(ExternalKind::Func, Var(func_index));
        if (!isImport) {
            AST ast(*_mc, f);
            ast.Construct(f->exprs, f->GetNumResults(), true);
            // Function
            Function* func = new Function(f->name);
            this->insertNode(func);
            new ASTEdge(m, func);
            // Index
            IndexNode* index = new IndexNode(func_index);
            this->insertNode(index);
            new ASTEdge(func, index);
            // Function Signature
            FunctionSignature* fsign = new FunctionSignature();
            this->insertNode(fsign);
            new ASTEdge(func, fsign);

            std::vector<std::string> localsNames;
            getLocalsNames(f, &localsNames);
            //// Parameters
            Index numParameters = f->GetNumParams();
            if (numParameters > 0) {
                Parameters* parameters = new Parameters();
                this->insertNode(parameters);
                new ASTEdge(fsign, parameters);

                for (Index i = 0; i < numParameters; i++) {
                    TypeNode* tnode =
                        new TypeNode(f->GetParamType(i), localsNames[i]);
                    this->insertNode(tnode);
                    new ASTEdge(parameters, tnode);
                }
            }
            //// Locals
            Index numLocals = f->GetNumLocals();
            if (numLocals > 0) {
                Locals* locals = new Locals();
                this->insertNode(locals);
                new ASTEdge(fsign, locals);

                for (Index i = numParameters; i < f->GetNumParamsAndLocals();
                     i++) {
                    TypeNode* tnode =
                        new TypeNode(f->GetLocalType(i), localsNames[i]);
                    this->insertNode(tnode);
                    new ASTEdge(locals, tnode);
                }
            }
            //// Results
            Index numResults = f->GetNumResults();
            if (numResults > 0) {
                Results* results = new Results();
                this->insertNode(results);
                new ASTEdge(fsign, results);

                for (Index i = 0; i < numResults; i++) {
                    TypeNode* tnode = new TypeNode(f->GetResultType(i));
                    this->insertNode(tnode);
                    new ASTEdge(results, tnode);
                }
            }

            // Instructions
            Instructions* inst = new Instructions();
            this->insertNode(inst);
            new ASTEdge(func, inst);

            for (wabt::Node& n : ast.exp_stack) {
                visitWabtNode(inst, &n);
            }

            // Add return node if none
            if (f->GetNumResults() == 0) {
                Return* ret = new Return();
                this->insertNode(ret);
                new ASTEdge(inst, ret);
            }
        }
        func_index++;
    }
}

void Graph::generateCFG(GenerateCPGOptions options) {
    CFGvisitor visitor(this);
    _nodes[0]->accept(&visitor);
}

void Graph::getLocalsNames(Func* f, std::vector<std::string>* names) {
    Index size = f->GetNumParamsAndLocals();
    names->reserve(size);
    for (Index i = 0; i < size; i++) {
        names->push_back("");
    }
    for (auto local : f->bindings) {
        (*names)[local.second.index] = local.first;
    }
}

void Graph::visitWabtNode(wasmati::Node* parentNode, wabt::Node* node) {
    switch (node->ntype) {
    case wabt::NodeType::FlushToVars:
    case wabt::NodeType::Statements:
        for (wabt::Node& n : node->children) {
            visitWabtNode(parentNode, &n);
        }
        break;
    case wabt::NodeType::EndReturn: {
        Return* ret = new Return();
        this->insertNode(ret);
        new ASTEdge(parentNode, ret);
        for (wabt::Node& n : node->children) {
            visitWabtNode(ret, &n);
        }
        break;
    }
    case wabt::NodeType::Expr: {
        size_t nextId = _nodes.size();
        Instruction* inst =
            new Instruction(node->etype, const_cast<Expr*>(node->e));
        this->insertNode(inst);
        new ASTEdge(parentNode, _nodes[nextId]);
        // CASE OF IF EXPR
        if (node->etype == ExprType::If) {
            const IfExpr* ifExpr = cast<IfExpr>(node->e);
            if (node->children.size() < 2 && node->children.size() > 3) {
                assert(false);
            }
            // visit condition
            visitWabtNode(inst, &node->children[0]);

            // Add true block
            const Block* trueBlock = &ifExpr->true_;
            BlockExpr* blockExpr = new BlockExpr();
            //// copy block
            blockExpr->block.decl = trueBlock->decl;
            blockExpr->block.label = trueBlock->label;

            Instruction* block =
                new Instruction(ExprType::Block, static_cast<Expr*>(blockExpr));
            this->insertNode(block);
            new ASTEdge(inst, block);
            visitWabtNode(block, &node->children[1]);

            // in case there is an else (false)
            // there is no block in this case, so i add an Else node
            if (node->children.size() == 3) {
                Else* elseNode = new Else();
                this->insertNode(elseNode);
                new ASTEdge(inst, elseNode);
                visitWabtNode(elseNode, &node->children[2]);
            }
        } else {
            for (wabt::Node& n : node->children) {
                visitWabtNode(inst, &n);
            }
        }
        break;
    }
    case wabt::NodeType::DeclInit: {
        LocalSetExpr* setExpr = new LocalSetExpr(*node->u.var);
        Instruction* inst = new Instruction(node->etype, setExpr);
        insertNode(inst);
        new ASTEdge(parentNode, inst);
        for (wabt::Node& n : node->children) {
            visitWabtNode(inst, &n);
        }
        break;
    }
    case wabt::NodeType::Decl:
    case wabt::NodeType::FlushedVar:
    case wabt::NodeType::Uninitialized:
    default:
        break;
    }
    return;
}

}  // namespace wasmati
