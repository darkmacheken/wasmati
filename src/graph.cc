#include "src/graph.h"

namespace wasmati {

int Node::idCount = 0;

Node::~Node() {
	for (auto e : _edges) {
		delete e;
	}
}
std::string Edge::toString() {
	return std::to_string(_src->getId()) + " -> " + std::to_string(_dest->getId()) + " [color=black]\n";
}

std::string ASTEdge::toString() {
	return std::to_string(_src->getId()) + " -> " + std::to_string(_dest->getId()) + " [color=forestgreen]\n";
}

std::string CFGEdge::toString() {
	return std::to_string(_src->getId()) + " -> " + std::to_string(_dest->getId()) + " [color=red]\n";
}

std::string PDGEdge::toString() {
	return std::to_string(_src->getId()) + " -> " + std::to_string(_dest->getId()) + " [color=blue]\n";
}

std::string Node::edgesToString() {
	std::string s;
	
	for (Edge* e : _edges) {
		s  += e->toString();
	}
	return s;
}

std::string Module::toString() {
	std::string s = this->edgesToString();
	if (_name.empty()) {
		s += std::to_string(_id) + " [label=<<TABLE><TR><TD>Module</TD></TR></TABLE>>];";
	} else {
		s += std::to_string(_id) + " [label=<<TABLE><TR><TD>module</TD></TR>";
		s += "<TR><TD>";
		s += "name = " + _name + "</TD></TR></TABLE>>];";
	}
	return s;
}

std::string Function::toString() {
	std::string s = this->edgesToString();
	s += std::to_string(_id) + " [label=<<TABLE><TR><TD>Function</TD></TR>";
	s += "<TR><TD>";
	s += "name = " + _name + "</TD></TR></TABLE>> ];";
	return s;
}

std::string SimpleNode::toString() {
	return this->edgesToString() + std::to_string(_id) + " [label=<<TABLE><TR><TD>" + _nodeName + "</TD></TR></TABLE>>];";
}

std::string IndexNode::toString() {
	std::string s = this->edgesToString();
	return s + std::to_string(_id) + " [label=<<TABLE><TR><TD>Index</TD></TR>" +
		"<TR><TD>" + std::to_string(_index) + "</TD></TR></TABLE>>];";
}

std::string NamedNode::toString() {
	return this->edgesToString() + std::to_string(_id) + " [label=<<TABLE><TR><TD>name</TD></TR><TR><TD>" + 
		_name + "</TD></TR></TABLE>>];";
}

std::string TypeNode::toString() {
	std::string s = this->edgesToString();
	s += std::to_string(_id) + " [label=<<TABLE><TR><TD>Type</TD></TR>";
	s += "<TR><TD>";
	switch (_type) {
	case wabt::Type::I32:
		s += "i32";
		break;
	case wabt::Type::I64:
		s += "i64";
		break;
	case wabt::Type::F32:
		s += "f32";
		break;
	case wabt::Type::F64:
		s += "i64";
		break;
	default:
		s += "unknown";
		break;
	}
	s += "</TD></TR></TABLE>>];";
	return s;
}

template <ExprType T>
std::string Instruction<T>::toString() {
	return this->edgesToString() + std::to_string(_id) + " [label=<<TABLE>" + _representation + "</TABLE>>];";
}

Graph::Graph(wabt::Module* mc) : _mc(new ModuleContext(*mc)),
		_visitor(this){}

void Graph::writePuts(wabt::Stream* stream, const char* s) {
	size_t len = strlen(s);
	stream->WriteData(s, len);
	stream->WriteChar('\n');
}

void Graph::writeString(wabt::Stream* stream, const std::string& str) {
	writePuts(stream, str.c_str());
}

void Graph::generateAST() {
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
			//// Parameters
			Index numParameters = f->GetNumParams();
			if (numParameters > 0) {
				Parameters* parameters = new Parameters();
				this->insertNode(parameters);
				new ASTEdge(fsign, parameters);

				for (Index i = 0; i < numParameters; i++) {
					NamedNode* node = new NamedNode("p" + std::to_string(i));
					this->insertNode(node);
					new ASTEdge(parameters, node);
					TypeNode* tnode = new TypeNode(f->GetParamType(i));
					this->insertNode(tnode);
					new ASTEdge(node, tnode);
				}
			}
			//// Locals
			Index numLocals = f->GetNumLocals();
			if (numLocals > 0) {
				Locals* locals = new Locals();
				this->insertNode(locals);
				new ASTEdge(fsign, locals);

				for (Index i = numParameters; i < f->GetNumParamsAndLocals(); i++) {
					NamedNode* node = new NamedNode("l" + std::to_string(i));
					this->insertNode(node);
					new ASTEdge(locals, node);
					TypeNode* tnode = new TypeNode(f->GetLocalType(i));
					this->insertNode(tnode);
					new ASTEdge(node, tnode);
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
		_visitor.visitExpr(const_cast<Expr*>(node->e));
		new ASTEdge(parentNode, _nodes[nextId]);
		for (wabt::Node& n : node->children) {
			visitWabtNode(_nodes[nextId], &n);
		}
		break;
	}
	case wabt::NodeType::DeclInit: {
		std::string s = Opcode::LocalSet_Opcode.GetName() + node->u.var->name();
		Instruction<ExprType::LocalSet>* inst = 
			new Instruction<ExprType::LocalSet>(cellRepr(s));
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

wabt::Result Graph::writeGraph(wabt::Stream* stream) {
	writePuts(stream, "digraph G {");
	writePuts(stream, "graph [rankdir=TD];");
	writePuts(stream, "node [shape=none];");
	
	for (auto const& node : _nodes) {
		writeString(stream, node->toString());
	}
	writePuts(stream, "}");
	return wabt::Result::Ok;
}

Node* Graph::ExprVisitor::visitExpr(Expr* expr) {
	Node* ret;
	switch (expr->type()) {
	case ExprType::AtomicLoad:
		ret = OnAtomicLoadExpr(cast<AtomicLoadExpr>(expr));
		break;
	case ExprType::AtomicStore:
		ret = OnAtomicStoreExpr(cast<AtomicStoreExpr>(expr));
		break;
	case ExprType::AtomicRmw:
		ret = OnAtomicRmwExpr(cast<AtomicRmwExpr>(expr));
		break;
	case ExprType::AtomicRmwCmpxchg:
		ret = OnAtomicRmwCmpxchgExpr(cast<AtomicRmwCmpxchgExpr>(expr));
		break;
	case ExprType::AtomicWait:
		ret = OnAtomicWaitExpr(cast<AtomicWaitExpr>(expr));
		break;
	case ExprType::AtomicNotify:
		ret = OnAtomicNotifyExpr(cast<AtomicNotifyExpr>(expr));
		break;
	case ExprType::Binary:
		ret = OnBinaryExpr(cast<BinaryExpr>(expr));
		break;
	case ExprType::Block:
		ret = OnBlockExpr(cast<BlockExpr>(expr));
		break;
	case ExprType::Br:
		ret = OnBrExpr(cast<BrExpr>(expr));
		break;
	case ExprType::BrIf:
		ret = OnBrIfExpr(cast<BrIfExpr>(expr));
		break;
	case ExprType::BrOnExn:
		ret = OnBrOnExnExpr(cast<BrOnExnExpr>(expr));
		break;
	case ExprType::BrTable:
		ret = OnBrTableExpr(cast<BrTableExpr>(expr));
		break;
	case ExprType::Call:
		ret = OnCallExpr(cast<CallExpr>(expr));
		break;
	case ExprType::CallIndirect:
		ret = OnCallIndirectExpr(cast<CallIndirectExpr>(expr));
		break;
	case ExprType::Compare:
		ret = OnCompareExpr(cast<CompareExpr>(expr));
		break;
	case ExprType::Const:
		ret = OnConstExpr(cast<ConstExpr>(expr));
		break;
	case ExprType::Convert:
		ret = OnConvertExpr(cast<ConvertExpr>(expr));
		break;
	case ExprType::Drop:
		ret = OnDropExpr(cast<DropExpr>(expr));
		break;
	case ExprType::GlobalGet:
		ret = OnGlobalGetExpr(cast<GlobalGetExpr>(expr));
		break;
	case ExprType::GlobalSet:
		ret = OnGlobalSetExpr(cast<GlobalSetExpr>(expr));
		break;
	case ExprType::If: 
		ret = OnIfExpr(cast<IfExpr>(expr));
		break;
	case ExprType::Load:
		ret = OnLoadExpr(cast<LoadExpr>(expr));
		break;
	case ExprType::LoadSplat:
		ret = OnLoadSplatExpr(cast<LoadSplatExpr>(expr));
		break;
	case ExprType::LocalGet:
		ret = OnLocalGetExpr(cast<LocalGetExpr>(expr));
		break;
	case ExprType::LocalSet:
		ret = OnLocalSetExpr(cast<LocalSetExpr>(expr));
		break;
	case ExprType::LocalTee:
		ret = OnLocalTeeExpr(cast<LocalTeeExpr>(expr));
		break;
	case ExprType::Loop: 
		ret = OnLoopExpr(cast<LoopExpr>(expr));
		break;
	case ExprType::MemoryCopy:
		ret = OnMemoryCopyExpr(cast<MemoryCopyExpr>(expr));
		break;
	case ExprType::DataDrop:
		ret = OnDataDropExpr(cast<DataDropExpr>(expr));
		break;
	case ExprType::MemoryFill:
		ret = OnMemoryFillExpr(cast<MemoryFillExpr>(expr));
		break;
	case ExprType::MemoryGrow:
		ret = OnMemoryGrowExpr(cast<MemoryGrowExpr>(expr));
		break;
	case ExprType::MemoryInit:
		ret = OnMemoryInitExpr(cast<MemoryInitExpr>(expr));
		break;
	case ExprType::MemorySize:
		ret = OnMemorySizeExpr(cast<MemorySizeExpr>(expr));
		break;
	case ExprType::TableCopy:
		ret = OnTableCopyExpr(cast<TableCopyExpr>(expr));
		break;
	case ExprType::ElemDrop:
		ret = OnElemDropExpr(cast<ElemDropExpr>(expr));
		break;
	case ExprType::TableInit:
		ret = OnTableInitExpr(cast<TableInitExpr>(expr));
		break;
	case ExprType::TableGet:
		ret = OnTableGetExpr(cast<TableGetExpr>(expr));
		break;
	case ExprType::TableSet:
		ret = OnTableSetExpr(cast<TableSetExpr>(expr));
		break;
	case ExprType::TableGrow:
		ret = OnTableGrowExpr(cast<TableGrowExpr>(expr));
		break;
	case ExprType::TableSize:
		ret = OnTableSizeExpr(cast<TableSizeExpr>(expr));
		break;
	case ExprType::TableFill:
		ret = OnTableFillExpr(cast<TableFillExpr>(expr));
		break;
	case ExprType::RefFunc:
		ret = OnRefFuncExpr(cast<RefFuncExpr>(expr));
		break;
	case ExprType::RefNull:
		ret = OnRefNullExpr(cast<RefNullExpr>(expr));
		break;
	case ExprType::RefIsNull:
		ret = OnRefIsNullExpr(cast<RefIsNullExpr>(expr));
		break;
	case ExprType::Nop:
		ret = OnNopExpr(cast<NopExpr>(expr));
		break;
	case ExprType::Rethrow:
		ret = OnRethrowExpr(cast<RethrowExpr>(expr));
		break;
	case ExprType::Return:
		ret = OnReturnExpr(cast<ReturnExpr>(expr));
		break;
	case ExprType::ReturnCall:
		ret = OnReturnCallExpr(cast<ReturnCallExpr>(expr));
		break;
	case ExprType::ReturnCallIndirect:
		ret = OnReturnCallIndirectExpr(cast<ReturnCallIndirectExpr>(expr));
		break;
	case ExprType::Select:
		ret = OnSelectExpr(cast<SelectExpr>(expr));
		break;
	case ExprType::Store:
		ret = OnStoreExpr(cast<StoreExpr>(expr));
		break;
	case ExprType::Throw:
		ret = OnThrowExpr(cast<ThrowExpr>(expr));
		break;
	case ExprType::Try: 
		ret = OnTryExpr(cast<TryExpr>(expr));
		break;
	case ExprType::Unary:
		ret = OnUnaryExpr(cast<UnaryExpr>(expr));
		break;
	case ExprType::Ternary:
		ret = OnTernaryExpr(cast<TernaryExpr>(expr));
		break;
	case ExprType::SimdLaneOp: 
		ret = OnSimdLaneOpExpr(cast<SimdLaneOpExpr>(expr));
		break;
	case ExprType::SimdShuffleOp: 
		ret = OnSimdShuffleOpExpr(cast<SimdShuffleOpExpr>(expr));
		break;
	case ExprType::Unreachable:
		ret = OnUnreachableExpr(cast<UnreachableExpr>(expr));
		break;
	}
	return ret;
}

Node* Graph::ExprVisitor::OnBinaryExpr(BinaryExpr* expr) {
	Instruction<ExprType::Binary>* inst = 
		new Instruction<ExprType::Binary>(_graph->cellRepr(expr->opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnBlockExpr(BlockExpr* expr) {
	Instruction<ExprType::Block>* inst =
		new Instruction<ExprType::Block>(_graph->cellRepr(Opcode::Block_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;

}

Node* Graph::ExprVisitor::OnBrExpr(BrExpr* expr) {
	std::string s = Opcode::Br_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::Br>* inst = new Instruction<ExprType::Br>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnBrIfExpr(BrIfExpr* expr) {
	std::string s = Opcode::BrIf_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::BrIf>* inst = new Instruction<ExprType::BrIf>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnBrOnExnExpr(BrOnExnExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnBrTableExpr(BrTableExpr* expr) {
	Instruction<ExprType::BrTable>* inst =
		new Instruction<ExprType::BrTable>(_graph->cellRepr(Opcode::BrTable_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnCallExpr(CallExpr* expr) {
	std::string s = Opcode::Call_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::Call>* inst = new Instruction<ExprType::Call>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnCallIndirectExpr(CallIndirectExpr* expr) {
	std::string s = Opcode::CallIndirect_Opcode.GetName();
	s += " " + expr->table.name();
	Instruction<ExprType::CallIndirect>* inst = new Instruction<ExprType::CallIndirect>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnCompareExpr(CompareExpr* expr) {
	Instruction<ExprType::Compare>* inst =
		new Instruction<ExprType::Compare>(_graph->cellRepr(expr->opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnConstExpr(ConstExpr* expr) {
	Instruction<ExprType::Const>* inst =
		new Instruction<ExprType::Const>(_graph->cellRepr(_graph->writeConst(expr->const_)));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnConvertExpr(ConvertExpr* expr) {
	Instruction<ExprType::Convert>* inst =
		new Instruction<ExprType::Convert>(_graph->cellRepr(expr->opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnDropExpr(DropExpr* expr) {
	Instruction<ExprType::Drop>* inst = 
		new Instruction<ExprType::Drop>(_graph->cellRepr(Opcode::Drop_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnGlobalGetExpr(GlobalGetExpr* expr) {
	std::string s = Opcode::GlobalGet_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::GlobalGet>* inst = new Instruction<ExprType::GlobalGet>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnGlobalSetExpr(GlobalSetExpr* expr) {
	std::string s = Opcode::GlobalSet_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::GlobalSet>* inst = new Instruction<ExprType::GlobalSet>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnIfExpr(IfExpr* expr) {
	Instruction<ExprType::If>* inst =
		new Instruction<ExprType::If>(_graph->cellRepr(Opcode::If_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnLoadExpr(LoadExpr* expr) {
	Instruction<ExprType::Load>* inst =
		new Instruction<ExprType::Load>(_graph->cellRepr(expr->opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnLocalGetExpr(LocalGetExpr* expr) {
	std::string s = Opcode::LocalGet_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::LocalGet>* inst = new Instruction<ExprType::LocalGet>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnLocalSetExpr(LocalSetExpr* expr) {
	std::string s = Opcode::LocalSet_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::LocalSet>* inst = new Instruction<ExprType::LocalSet>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnLocalTeeExpr(LocalTeeExpr* expr) {
	std::string s = Opcode::LocalTee_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::LocalTee>* inst = new Instruction<ExprType::LocalTee>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnLoopExpr(LoopExpr* expr) {
	Instruction<ExprType::Loop>* inst =
		new Instruction<ExprType::Loop>(_graph->cellRepr(Opcode::Loop_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;

}

Node* Graph::ExprVisitor::OnMemoryCopyExpr(MemoryCopyExpr* expr) {
	Instruction<ExprType::MemoryCopy>* inst =
		new Instruction<ExprType::MemoryCopy>(_graph->cellRepr(Opcode::Drop_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnDataDropExpr(DataDropExpr* expr) {
	std::string s = Opcode::DataDrop_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::DataDrop>* inst = new Instruction<ExprType::DataDrop>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnMemoryFillExpr(MemoryFillExpr* expr) {
	Instruction<ExprType::MemoryFill>* inst =
		new Instruction<ExprType::MemoryFill>(_graph->cellRepr(Opcode::MemoryFill_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnMemoryGrowExpr(MemoryGrowExpr* expr) {
	Instruction<ExprType::MemoryGrow>* inst =
		new Instruction<ExprType::MemoryGrow>(_graph->cellRepr(Opcode::MemoryGrow_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnMemorySizeExpr(MemorySizeExpr* expr) {
	Instruction<ExprType::MemorySize>* inst =
		new Instruction<ExprType::MemorySize>(_graph->cellRepr(Opcode::MemorySize_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnMemoryInitExpr(MemoryInitExpr* expr) {
	std::string s = Opcode::MemoryInit_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::MemoryInit>* inst = new Instruction<ExprType::MemoryInit>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnTableCopyExpr(TableCopyExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnElemDropExpr(ElemDropExpr* expr) {
	std::string s = Opcode::ElemDrop_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::ElemDrop>* inst = new Instruction<ExprType::ElemDrop>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnTableInitExpr(TableInitExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnTableGetExpr(TableGetExpr* expr) {
	std::string s = Opcode::TableGet_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::TableGet>* inst = new Instruction<ExprType::TableGet>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnTableSetExpr(TableSetExpr* expr) {
	std::string s = Opcode::TableSet_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::TableSet>* inst = new Instruction<ExprType::TableSet>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnTableGrowExpr(TableGrowExpr* expr) {
	std::string s = Opcode::TableGrow_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::TableGrow>* inst = new Instruction<ExprType::TableGrow>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnTableSizeExpr(TableSizeExpr* expr) {
	std::string s = Opcode::TableSize_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::TableSize>* inst = new Instruction<ExprType::TableSize>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnTableFillExpr(TableFillExpr* expr) {
	std::string s = Opcode::TableFill_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::TableFill>* inst = new Instruction<ExprType::TableFill>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnRefFuncExpr(RefFuncExpr* expr) {
	std::string s = Opcode::RefFunc_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::RefFunc>* inst = new Instruction<ExprType::RefFunc>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnRefNullExpr(RefNullExpr* expr) {
	Instruction<ExprType::RefNull>* inst =
		new Instruction<ExprType::RefNull>(_graph->cellRepr(Opcode::RefNull_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnRefIsNullExpr(RefIsNullExpr* expr) {
	Instruction<ExprType::RefIsNull>* inst =
		new Instruction<ExprType::RefIsNull>(_graph->cellRepr(Opcode::RefIsNull_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnNopExpr(NopExpr* expr) {
	Instruction<ExprType::Nop>* inst =
		new Instruction<ExprType::Nop>(_graph->cellRepr(Opcode::Nop_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnReturnExpr(ReturnExpr* expr) {
	Instruction<ExprType::Return>* inst =
		new Instruction<ExprType::Return>(_graph->cellRepr(Opcode::Return_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnReturnCallExpr(ReturnCallExpr* expr) {
	std::string s = Opcode::ReturnCall_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::ReturnCallIndirect>* inst = new Instruction<ExprType::ReturnCallIndirect>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnReturnCallIndirectExpr(
    ReturnCallIndirectExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnSelectExpr(SelectExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnStoreExpr(StoreExpr* expr) {
	Instruction<ExprType::Store>* inst =
		new Instruction<ExprType::Store>(_graph->cellRepr(expr->opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnUnaryExpr(UnaryExpr* expr) {
	Instruction<ExprType::Unary>* inst =
		new Instruction<ExprType::Unary>(_graph->cellRepr(expr->opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnUnreachableExpr(
    UnreachableExpr* expr) {
	Instruction<ExprType::Unreachable>* inst =
		new Instruction<ExprType::Unreachable>(_graph->cellRepr(Opcode::Unreachable_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnTryExpr(TryExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnThrowExpr(ThrowExpr* expr) {
	std::string s = Opcode::Throw_Opcode.GetName();
	s += " " + expr->var.name();
	Instruction<ExprType::Throw>* inst = new Instruction<ExprType::Throw>(_graph->cellRepr(s));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnRethrowExpr(RethrowExpr* expr) {
	Instruction<ExprType::Rethrow>* inst =
		new Instruction<ExprType::Rethrow>(_graph->cellRepr(Opcode::Rethrow_Opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnAtomicWaitExpr(AtomicWaitExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnAtomicNotifyExpr(AtomicNotifyExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnAtomicLoadExpr(AtomicLoadExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnAtomicStoreExpr(AtomicStoreExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnAtomicRmwExpr(AtomicRmwExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnAtomicRmwCmpxchgExpr(AtomicRmwCmpxchgExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnTernaryExpr(TernaryExpr* expr) {
	Instruction<ExprType::Ternary>* inst =
		new Instruction<ExprType::Ternary>(_graph->cellRepr(expr->opcode.GetName()));
	_graph->insertNode(inst);
	return inst;
}

Node* Graph::ExprVisitor::OnSimdLaneOpExpr(SimdLaneOpExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnSimdShuffleOpExpr( SimdShuffleOpExpr* expr) {
	assert(false);
	return nullptr;
}

Node* Graph::ExprVisitor::OnLoadSplatExpr(LoadSplatExpr* expr) {
	assert(false);
	return nullptr;
}

std::string Graph::writeConst(const Const& const_) {
	std::string s;
	switch (const_.type) {
	case Type::I32:
		s += Opcode::I32Const_Opcode.GetName();
		s += " " + std::to_string(static_cast<int32_t>(const_.u32));
		break;

	case Type::I64:
		s += Opcode::I64Const_Opcode.GetName();
		s += " " + std::to_string(static_cast<int64_t>(const_.u64));
		break;

	case Type::F32: {
		s += Opcode::F32Const_Opcode.GetName();
		float f32;
		memcpy(&f32, &const_.f32_bits, sizeof(f32));
		s += " " + std::to_string(f32);
		break;
	}

	case Type::F64: {
		s += Opcode::F64Const_Opcode.GetName();
		double f64;
		memcpy(&f64, &const_.f64_bits, sizeof(f64));
		s += " " + std::to_string(f64);
		break;
	}

	case Type::V128: {
		assert(false);
		break;
	}

	default:
		assert(0);
		break;
	}
	return s;
}


}  // namespace wasmati

