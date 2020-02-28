#include "src/graph.h"

namespace wasmati {

int Node::idCount = 0;

Node::~Node() {
	for (auto e : _edges) {
		delete e;
	}
}

void Node::acceptEdges(GraphVisitor* visitor) {
	for (Edge* e : _edges) {
		e->accept(visitor);
	}
}

Graph::Graph(wabt::Module* mc) : _mc(new ModuleContext(*mc)){}


void Graph::generateAST(GenerateASTOptions options) {
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
		if (!options.funcName.empty() && options.funcName.compare(f->name) != 0) {
			func_index++; 
			continue;
		}
		auto isImport = _mc->module.IsImport(ExternalKind::Func, Var(func_index));
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
					TypeNode* tnode = new TypeNode(f->GetParamType(i), localsNames[i]);
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

				for (Index i = numParameters; i < f->GetNumParamsAndLocals(); i++) {
					TypeNode* tnode = new TypeNode(f->GetLocalType(i), localsNames[i]);
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

			//Instructions
			Instructions* inst = new Instructions();
			this->insertNode(inst);
			new ASTEdge(func, inst);

			for (wabt::Node& n : ast.exp_stack) {
				visitWabtNode(inst, &n);
			}

		}
		func_index++;
	}

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
		Instruction* inst = new Instruction(node->etype, node->e);
		this->insertNode(inst);
		new ASTEdge(parentNode, _nodes[nextId]);
		for (wabt::Node& n : node->children) {
			visitWabtNode(_nodes[nextId], &n);
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
	case wabt::NodeType::FlushToVars:
	case wabt::NodeType::Uninitialized:
	default:
		break;
	}
	return;
}

}  // namespace wasmati

