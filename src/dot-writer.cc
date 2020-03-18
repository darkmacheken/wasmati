#include "src/dot-writer.h"

namespace wasmati {

void DotWriter::writeGraph() {
	writePuts("digraph G {");
	writePuts("graph [rankdir=TD];");
	writePuts("node [shape=none];");
	bool justOneGraph = _options.printJustAST || _options.printJustCFG || _options.printJustPDG;

	for (auto const& node : *_graph->getNodes()) {
		node->acceptEdges(this);

		if (!justOneGraph) {
			node->accept(this);
		} else if (_options.printJustAST && node->hasASTEdges()){ // AST
			node->accept(this);
		} else if (_options.printJustCFG && node->hasCFGEdges()) { // CFG
			node->accept(this);
		} else if (_options.printJustPDG && node->hasPDGEdges()) { // PDG
			node->accept(this);
		}
	}

	if (!justOneGraph) {
		setSameRank();
	}

	writePuts("}");
}

void DotWriter::visitASTEdge(ASTEdge* e) {
	if (_options.printJustCFG || _options.printJustPDG) {
		return;
	}
	writeStringln(std::to_string(e->_src->getId()) + " -> " + std::to_string(e->_dest->getId()) + " [color=forestgreen]");
}

void DotWriter::visitCFGEdge(CFGEdge* e) {
	if (_options.printJustAST || _options.printJustPDG) {
		return;
	}
	if (e->_label.empty()) {
		writeStringln(std::to_string(e->_src->getId()) + " -> " + std::to_string(e->_dest->getId()) + " [color=red]");
	} else {
		writeStringln(std::to_string(e->_src->getId()) + " -> " + std::to_string(e->_dest->getId()) + 
			" [label=\"" + e->_label + "\"color=red]");
	}
}

void DotWriter::visitPDGEdge(PDGEdge* e) {
	if (_options.printJustAST || _options.printJustCFG) {
		return;
	}
	writeStringln(std::to_string(e->_src->getId()) + " -> " + std::to_string(e->_dest->getId()) + " [color=blue]");
}

void DotWriter::visitModule(Module* mod) {
	std::string s;
	if (mod->getName().empty()) {
		s += std::to_string(mod->getId()) + " [label=<<TABLE><TR><TD>Module</TD></TR></TABLE>>];";
	}
	else {
		s += std::to_string(mod->getId()) + " [label=<<TABLE><TR><TD>module</TD></TR>";
		s += "<TR><TD>";
		s += "name = " + mod->getName() + "</TD></TR></TABLE>>];";
	}
	writeStringln(s);
}

void DotWriter::visitFunction(Function* func) {
	std::string s = std::to_string(func->getId()) + " [label=<<TABLE><TR><TD>Function</TD></TR>";
	s += "<TR><TD>";
	s += "name = " + func->getName() + "</TD></TR></TABLE>> ];";
	writeStringln(s);
}

void DotWriter::visitTypeNode(TypeNode* node) {
	std::string s = std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>Type</TD>";
	s += "<TD>";
	switch (node->getType()) {
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
		s += "f64";
		break;
	default:
		s += "unknown";
		break;
	}
	s += "</TD></TR>";

	if (!node->getName().empty()) {
		s += "<TR><TD>name</TD><TD>" + node->getName() + "</TD></TR>";
	}
	s += "</TABLE>>];";
	writeStringln(s);
		
}

void DotWriter::visitSimpleNode(SimpleNode* node) {
	writeStringln(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>" + node->getNodeName() + 
		"</TD></TR></TABLE>>];");
}

void DotWriter::visitInstructions(Instructions* node) {
	visitSimpleNode(node);
}

void DotWriter::visitIndexNode(IndexNode* node) {
	writeStringln(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>Index</TD></TR>" +
		"<TR><TD>" + std::to_string(node->getIndex()) + "</TD></TR></TABLE>>];");
}

void DotWriter::visitInstruction(Instruction* inst) {
	writeString(std::to_string(inst->getId()) + " [label=<<TABLE><TR><TD>");
	visitExpr(const_cast<Expr*>(inst->getExpr()));
	writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitReturn(Return* expr) {
	visitSimpleNode(expr);
}

void DotWriter::visitElse(Else* expr) {
	visitSimpleNode(expr);
}

void DotWriter::visitStart(Start* start) {
	visitSimpleNode(start);
}

void DotWriter::visitTrap(Trap* trap) {
	visitSimpleNode(trap);
}

void DotWriter::OnBinaryExpr(BinaryExpr* expr) {
	writeString(expr->opcode.GetName());
}

void DotWriter::OnBlockExpr(BlockExpr* expr) {
	writeString(Opcode::Block_Opcode.GetName());
	writeString(" " + expr->block.label);
}

void DotWriter::OnBrExpr(BrExpr* expr) {
	writeString(Opcode::Br_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnBrIfExpr(BrIfExpr* expr) {
	writeString(Opcode::BrIf_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnBrOnExnExpr(BrOnExnExpr*) {
	assert(false);
}

void DotWriter::OnBrTableExpr(BrTableExpr*) {
	writeString(Opcode::BrTable_Opcode.GetName());
}

void DotWriter::OnCallExpr(CallExpr* expr) {
	writeString(Opcode::Call_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnCallIndirectExpr(CallIndirectExpr* expr) {
	writeString(Opcode::CallIndirect_Opcode.GetName());
	writeString(" " + expr->table.name());
}

void DotWriter::OnCompareExpr(CompareExpr* expr) {
	writeString(expr->opcode.GetName());
}

void DotWriter::OnConstExpr(ConstExpr* expr) {
	writeString(writeConst(expr->const_));
}

void DotWriter::OnConvertExpr(ConvertExpr* expr) {
	writeString(expr->opcode.GetName());
}

void DotWriter::OnDropExpr(DropExpr*) {
	writeString(Opcode::Drop_Opcode.GetName());
}

void DotWriter::OnGlobalGetExpr(GlobalGetExpr* expr) {
	writeString(Opcode::GlobalGet_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnGlobalSetExpr(GlobalSetExpr* expr) {
	writeString(Opcode::GlobalSet_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnIfExpr(IfExpr*) {
	writeString(Opcode::If_Opcode.GetName());
}

void DotWriter::OnLoadExpr(LoadExpr* expr) {
	writeString(expr->opcode.GetName());
}

void DotWriter::OnLocalGetExpr(LocalGetExpr* expr) {
	writeString(Opcode::LocalGet_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnLocalSetExpr(LocalSetExpr* expr) {
	writeString(Opcode::LocalSet_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnLocalTeeExpr(LocalTeeExpr* expr) {
	writeString(Opcode::LocalTee_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnLoopExpr(LoopExpr*) {
	writeString(Opcode::Loop_Opcode.GetName());
}

void DotWriter::OnMemoryCopyExpr(MemoryCopyExpr*) {
	writeString(Opcode::MemoryCopy_Opcode.GetName());
}

void DotWriter::OnDataDropExpr(DataDropExpr* expr) {
	writeString(Opcode::DataDrop_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnMemoryFillExpr(MemoryFillExpr*) {
	writeString(Opcode::MemoryFill_Opcode.GetName());
}

void DotWriter::OnMemoryGrowExpr(MemoryGrowExpr*) {
	writeString(Opcode::MemoryGrow_Opcode.GetName());
}

void DotWriter::OnMemoryInitExpr(MemoryInitExpr* expr) {
	writeString(Opcode::MemoryInit_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnMemorySizeExpr(MemorySizeExpr*) {
	writeString(Opcode::MemorySize_Opcode.GetName());
}

void DotWriter::OnTableCopyExpr(TableCopyExpr*) {
	assert(false);
}

void DotWriter::OnElemDropExpr(ElemDropExpr* expr) {
	writeString(Opcode::ElemDrop_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnTableInitExpr(TableInitExpr*) {
	assert(false);
}

void DotWriter::OnTableGetExpr(TableGetExpr* expr) {
	writeString(Opcode::TableGet_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnTableSetExpr(TableSetExpr* expr) { 
	writeString(Opcode::TableSet_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnTableGrowExpr(TableGrowExpr* expr) {
	writeString(Opcode::TableGrow_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnTableSizeExpr(TableSizeExpr* expr) {
	writeString(Opcode::TableSize_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnTableFillExpr(TableFillExpr* expr) {
	writeString(Opcode::TableFill_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnRefFuncExpr(RefFuncExpr* expr) {
	writeString(Opcode::RefFunc_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnRefNullExpr(RefNullExpr*) {
	writeString(Opcode::RefNull_Opcode.GetName());
}

void DotWriter::OnRefIsNullExpr(RefIsNullExpr*) {
	writeString(Opcode::RefIsNull_Opcode.GetName());
}

void DotWriter::OnNopExpr(NopExpr*) {
	writeString(Opcode::Nop_Opcode.GetName());
}

void DotWriter::OnReturnExpr(ReturnExpr*) {
	writeString(Opcode::Return_Opcode.GetName());
}

void DotWriter::OnReturnCallExpr(ReturnCallExpr* expr) {
	writeString(Opcode::ReturnCall_Opcode.GetName());
	writeString(" " + expr->var.name());
}

void DotWriter::OnReturnCallIndirectExpr(ReturnCallIndirectExpr*) {
	assert(false);
}

void DotWriter::OnSelectExpr(SelectExpr*) {
	writeString(Opcode::Select_Opcode.GetName());
}

void DotWriter::OnStoreExpr(StoreExpr* expr) {
	writeString(expr->opcode.GetName());
}

void DotWriter::OnUnaryExpr(UnaryExpr* expr) {
	writeString(expr->opcode.GetName());
}

void DotWriter::OnUnreachableExpr(UnreachableExpr*) {
	writeString(Opcode::Unreachable_Opcode.GetName());
}

void DotWriter::OnTryExpr(TryExpr*) {
	assert(false);
}

void DotWriter::OnThrowExpr(ThrowExpr* expr) {
	assert(false);
}

void DotWriter::OnRethrowExpr(RethrowExpr*) {
	assert(false);
}

void DotWriter::OnAtomicWaitExpr(AtomicWaitExpr*) {
	assert(false);
}

void DotWriter::OnAtomicNotifyExpr(AtomicNotifyExpr*) {
	assert(false);
}

void DotWriter::OnAtomicLoadExpr(AtomicLoadExpr*) {
	assert(false);
}

void DotWriter::OnAtomicStoreExpr(AtomicStoreExpr*) {
	assert(false);
}

void DotWriter::OnAtomicRmwExpr(AtomicRmwExpr*) {
	assert(false);
}

void DotWriter::OnAtomicRmwCmpxchgExpr(AtomicRmwCmpxchgExpr*) {
	assert(false);
}

void DotWriter::OnTernaryExpr(TernaryExpr* expr) {
	writeString(expr->opcode.GetName());
}

void DotWriter::OnSimdLaneOpExpr(SimdLaneOpExpr*) {
	assert(false);
}

void DotWriter::OnSimdShuffleOpExpr(SimdShuffleOpExpr*) {
	assert(false);
}

void DotWriter::OnLoadSplatExpr(LoadSplatExpr*) {
	assert(false);
}

std::string DotWriter::writeConst(const Const& const_) {
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

void DotWriter::setSameRank() {
	setDepth(_graph->getNodes()->front(), 0);

	for (auto v : _depth) {
		writePuts("{rank = same; ");
		for (auto id : *v) {
			writeString(std::to_string(id) + "; ");
		}
		writePutsln("}");
		delete v;
	}
}

void DotWriter::setDepth(Node* node, Index depth) {
	if (_depth.size() <= depth) {
		_depth.push_back(new std::vector<int>());
	}
	_depth[depth]->push_back(node->getId());
	for (auto e : node->getEdges()) {
		if (e->getType() == EdgeType::AST) {
			setDepth(e->getDest(), depth + 1);
		}
	}
}


} // namespace wasmati