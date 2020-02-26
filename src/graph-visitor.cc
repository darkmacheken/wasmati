#include "src/graph-visitor.h"


namespace wasmati {

void GraphVisitor::visitExpr(Expr* expr) {
	switch (expr->type()) {
	case ExprType::AtomicLoad:
		OnAtomicLoadExpr(cast<AtomicLoadExpr>(expr));
		break;
	case ExprType::AtomicStore:
		OnAtomicStoreExpr(cast<AtomicStoreExpr>(expr));
		break;
	case ExprType::AtomicRmw:
		OnAtomicRmwExpr(cast<AtomicRmwExpr>(expr));
		break;
	case ExprType::AtomicRmwCmpxchg:
		OnAtomicRmwCmpxchgExpr(cast<AtomicRmwCmpxchgExpr>(expr));
		break;
	case ExprType::AtomicWait:
		OnAtomicWaitExpr(cast<AtomicWaitExpr>(expr));
		break;
	case ExprType::AtomicNotify:
		OnAtomicNotifyExpr(cast<AtomicNotifyExpr>(expr));
		break;
	case ExprType::Binary:
		OnBinaryExpr(cast<BinaryExpr>(expr));
		break;
	case ExprType::Block:
		OnBlockExpr(cast<BlockExpr>(expr));
		break;
	case ExprType::Br:
		OnBrExpr(cast<BrExpr>(expr));
		break;
	case ExprType::BrIf:
		OnBrIfExpr(cast<BrIfExpr>(expr));
		break;
	case ExprType::BrOnExn:
		OnBrOnExnExpr(cast<BrOnExnExpr>(expr));
		break;
	case ExprType::BrTable:
		OnBrTableExpr(cast<BrTableExpr>(expr));
		break;
	case ExprType::Call:
		OnCallExpr(cast<CallExpr>(expr));
		break;
	case ExprType::CallIndirect:
		OnCallIndirectExpr(cast<CallIndirectExpr>(expr));
		break;
	case ExprType::Compare:
		OnCompareExpr(cast<CompareExpr>(expr));
		break;
	case ExprType::Const:
		OnConstExpr(cast<ConstExpr>(expr));
		break;
	case ExprType::Convert:
		OnConvertExpr(cast<ConvertExpr>(expr));
		break;
	case ExprType::Drop:
		OnDropExpr(cast<DropExpr>(expr));
		break;
	case ExprType::GlobalGet:
		OnGlobalGetExpr(cast<GlobalGetExpr>(expr));
		break;
	case ExprType::GlobalSet:
		OnGlobalSetExpr(cast<GlobalSetExpr>(expr));
		break;
	case ExprType::If:
		OnIfExpr(cast<IfExpr>(expr));
		break;
	case ExprType::Load:
		OnLoadExpr(cast<LoadExpr>(expr));
		break;
	case ExprType::LoadSplat:
		OnLoadSplatExpr(cast<LoadSplatExpr>(expr));
		break;
	case ExprType::LocalGet:
		OnLocalGetExpr(cast<LocalGetExpr>(expr));
		break;
	case ExprType::LocalSet:
		OnLocalSetExpr(cast<LocalSetExpr>(expr));
		break;
	case ExprType::LocalTee:
		OnLocalTeeExpr(cast<LocalTeeExpr>(expr));
		break;
	case ExprType::Loop:
		OnLoopExpr(cast<LoopExpr>(expr));
		break;
	case ExprType::MemoryCopy:
		OnMemoryCopyExpr(cast<MemoryCopyExpr>(expr));
		break;
	case ExprType::DataDrop:
		OnDataDropExpr(cast<DataDropExpr>(expr));
		break;
	case ExprType::MemoryFill:
		OnMemoryFillExpr(cast<MemoryFillExpr>(expr));
		break;
	case ExprType::MemoryGrow:
		OnMemoryGrowExpr(cast<MemoryGrowExpr>(expr));
		break;
	case ExprType::MemoryInit:
		OnMemoryInitExpr(cast<MemoryInitExpr>(expr));
		break;
	case ExprType::MemorySize:
		OnMemorySizeExpr(cast<MemorySizeExpr>(expr));
		break;
	case ExprType::TableCopy:
		OnTableCopyExpr(cast<TableCopyExpr>(expr));
		break;
	case ExprType::ElemDrop:
		OnElemDropExpr(cast<ElemDropExpr>(expr));
		break;
	case ExprType::TableInit:
		OnTableInitExpr(cast<TableInitExpr>(expr));
		break;
	case ExprType::TableGet:
		OnTableGetExpr(cast<TableGetExpr>(expr));
		break;
	case ExprType::TableSet:
		OnTableSetExpr(cast<TableSetExpr>(expr));
		break;
	case ExprType::TableGrow:
		OnTableGrowExpr(cast<TableGrowExpr>(expr));
		break;
	case ExprType::TableSize:
		OnTableSizeExpr(cast<TableSizeExpr>(expr));
		break;
	case ExprType::TableFill:
		OnTableFillExpr(cast<TableFillExpr>(expr));
		break;
	case ExprType::RefFunc:
		OnRefFuncExpr(cast<RefFuncExpr>(expr));
		break;
	case ExprType::RefNull:
		OnRefNullExpr(cast<RefNullExpr>(expr));
		break;
	case ExprType::RefIsNull:
		OnRefIsNullExpr(cast<RefIsNullExpr>(expr));
		break;
	case ExprType::Nop:
		OnNopExpr(cast<NopExpr>(expr));
		break;
	case ExprType::Rethrow:
		OnRethrowExpr(cast<RethrowExpr>(expr));
		break;
	case ExprType::Return:
		OnReturnExpr(cast<ReturnExpr>(expr));
		break;
	case ExprType::ReturnCall:
		OnReturnCallExpr(cast<ReturnCallExpr>(expr));
		break;
	case ExprType::ReturnCallIndirect:
		OnReturnCallIndirectExpr(cast<ReturnCallIndirectExpr>(expr));
		break;
	case ExprType::Select:
		OnSelectExpr(cast<SelectExpr>(expr));
		break;
	case ExprType::Store:
		OnStoreExpr(cast<StoreExpr>(expr));
		break;
	case ExprType::Throw:
		OnThrowExpr(cast<ThrowExpr>(expr));
		break;
	case ExprType::Try:
		OnTryExpr(cast<TryExpr>(expr));
		break;
	case ExprType::Unary:
		OnUnaryExpr(cast<UnaryExpr>(expr));
		break;
	case ExprType::Ternary:
		OnTernaryExpr(cast<TernaryExpr>(expr));
		break;
	case ExprType::SimdLaneOp:
		OnSimdLaneOpExpr(cast<SimdLaneOpExpr>(expr));
		break;
	case ExprType::SimdShuffleOp:
		OnSimdShuffleOpExpr(cast<SimdShuffleOpExpr>(expr));
		break;
	case ExprType::Unreachable:
		OnUnreachableExpr(cast<UnreachableExpr>(expr));
		break;
	}
}

void GraphWriter::writePuts(const char* s) {
	size_t len = strlen(s);
	_stream->WriteData(s, len);
}

void GraphWriter::writeString(const std::string& str) {
	writePuts(str.c_str());
}

void GraphWriter::writePutsln(const char* s) {
	size_t len = strlen(s);
	_stream->WriteData(s, len);
	_stream->WriteChar('\n');
}

void GraphWriter::writeStringln(const std::string& str) {
	writePutsln(str.c_str());
}

} // end namespace wasmati