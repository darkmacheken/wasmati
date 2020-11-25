#include "datalog-writer.h"

namespace wasmati {

void DatalogWriter::writeGraph() {
    NodeSet loopsInsts;
    if (!cpgOptions.loopName.empty()) {
        loopsInsts = Queries::loopsInsts(cpgOptions.loopName);
    }

    writePuts(R"(
#ifndef WASMATI_DATALOG
#define WASMATI_DATALOG

#define false 0
#define true 1
.type bool <: unsigned

.type Id <: unsigned
.type NodeType <: symbol
.type InstType <: symbol
.type EdgeType <: symbol
.type PDGType <: symbol
.type VarType <: symbol
.type Label <: symbol

.type Const = [
	type:VarType,
	i:number,
	f:float
]
.type Opcode <: symbol
.type Name = Label | symbol

// Declarations
// -- Edges
.decl edge(x:Id, y:Id, type:EdgeType, label:Label, pdgType:PDGType, value:Const)
.decl astEdge(x:Id, y:Id)
.decl cpgEdge(x:Id, y:Id, label:Label)
.decl pdgEdge(x:Id, y:Id, label:Label, type:PDGType, value:Const)
.decl cgEdge(x:Id, y:Id)
.decl pgEdge(x:Id, y:Id)

// -- Nodes
.decl module(x:Id, name:Name)
.decl function(x:Id, name:Name, index:unsigned, narg:unsigned, nlocals:unsigned, nresults:unsigned, isImport:bool, isExport:bool)
.decl varNode(x:Id, type:VarType, index:unsigned, name:Name)
.decl functionSignature(x:Id)
.decl instructions(x:Id)
.decl parameters(x:Id)
.decl locals(x:Id)
.decl results(x:Id)
.decl else(x:Id)
.decl trap(x:Id)
.decl start(x:Id)
.decl instruction(x:Id, type:InstType)
.decl nop(x:Id)
.decl unreachable(x:Id)
.decl return(x:Id)
.decl brTable(x:Id)
.decl drop(x:Id)
.decl select(x:Id)
.decl memorySize(x:Id)
.decl memoryGrow(x:Id)
.decl const(x:Id, value:Const)
.decl binary(x:Id, opcode:Opcode)
.decl compare(x:Id, opcode:Opcode)
.decl convert(x:Id, opcode:Opcode)
.decl unary(x:Id, opcode:Opcode)
.decl load(x:Id, offset:unsigned)
.decl store(x:Id, offset:unsigned)
.decl br(x:Id, label:Label)
.decl brIf(x:Id, label:Label)
.decl globalGet(x:Id, label:Label)
.decl globalSet(x:Id, label:Label)
.decl localGet(x:Id, label:Label)
.decl localSet(x:Id, label:Label)
.decl localTee(x:Id, label:Label)
.decl call(x:Id, label:Label, nargs:unsigned, nresults:unsigned)
.decl callIndirect(x:Id, label:Label, nargs:unsigned, nresults:unsigned)
.decl block(x:Id, label:Label, nresults:unsigned)
.decl loop(x:Id, label:Label, nresults:unsigned)
.decl beginBlock(x:Id, label:Label)
.decl if(x:Id, nresults:unsigned, hasElse:bool)

// -- Basic Queries
.decl reaches(x:Id, y:Id, type:EdgeType)

// Rules
// -- Edges
astEdge(x, y) :- edge(x, y, "AST", _, _, _).
cpgEdge(x, y, label) :- edge(x, y, "CFG", label, _, _).
pdgEdge(x, y, label, type, value) :- edge(x , y, "PDG", label, type,value).
cgEdge(x,y) :- edge(x, y, "CG", _, _, _).
pgEdge(x,y) :- edge(x, y, "CG", _, _, _).

// -- Nodes
instruction(x, "Nop") :- nop(x).
instruction(x, "Unreachable") :- unreachable(x).
instruction(x, "Return") :- return(x).
instruction(x, "BrTable") :- brTable(x).
instruction(x, "Drop") :- drop(x).
instruction(x, "Select") :- select(x).
instruction(x, "MemorySize") :- memorySize(x).
instruction(x, "MemoryGrow") :- memoryGrow(x).
instruction(x, "Const") :- const(x, _).
instruction(x, "Binary") :- binary(x, _).
instruction(x, "Compare") :- compare(x, _).
instruction(x, "Convert") :- convert(x, _).
instruction(x, "Unary") :- unary(x, _).
instruction(x, "Load") :- load(x, _).
instruction(x, "Store") :- store(x, _).
instruction(x, "Br") :- br(x, _).
instruction(x, "BrIf") :- brIf(x, _).
instruction(x, "GlobalGet") :- globalGet(x, _).
instruction(x, "GlobalSet") :- globalSet(x, _).
instruction(x, "LocalGet") :- localGet(x, _).
instruction(x, "LocalSet") :- localSet(x, _).
instruction(x, "LocalTee") :- localTee(x, _).
instruction(x, "Call") :- call(x, _, _, _).
instruction(x, "CallIndirect") :- callIndirect(x, _, _, _).
instruction(x, "Block") :- block(x, _, _).
instruction(x, "Loop") :- loop(x, _, _).
instruction(x, "BeginBlock") :- beginBlock(x, _).
instruction(x, "If") :- if(x, _, _).

// -- Basic Queries
reaches(X, Y, type) :- edge(X, Y, type, _, _, _).
reaches(X, Y, type) :- edge(X, Z, type, _, _, _), reaches(Z, Y, _type).

)");

    bool justOneGraph =
        cpgOptions.printNoAST || cpgOptions.printNoCFG || cpgOptions.printNoPDG;

    for (auto const& node : _graph->getNodes()) {
        if (!loopsInsts.empty() && loopsInsts.count(node) == 0) {
            continue;
        }
        node->acceptEdges(this);

        if (!justOneGraph) {
            node->accept(this);
        } else if (!cpgOptions.printNoAST &&
                   node->hasEdgesOf(EdgeType::AST)) {  // AST
            node->accept(this);
        } else if (!cpgOptions.printNoCFG &&
                   node->hasEdgesOf(EdgeType::CFG)) {  // CFG
            node->accept(this);
        } else if (!cpgOptions.printNoPDG &&
                   node->hasEdgesOf(EdgeType::PDG)) {  // PDG
            node->accept(this);
        }
    }
    writePuts("#endif\n");
}

void DatalogWriter::visitASTEdge(ASTEdge* e) {
    if (cpgOptions.printNoAST) {
        return;
    }
    _stream->Writef("edge(%u, %u, \"AST\", \"\", \"\", nil).\n",
                    e->src()->getId(), e->dest()->getId());
}

void DatalogWriter::visitCFGEdge(CFGEdge* e) {
    if (cpgOptions.printNoCFG) {
        return;
    }

    _stream->Writef("edge(%u, %u, \"CFG\", \"%s\", \"\", nil).\n",
                    e->src()->getId(), e->dest()->getId(), e->label().c_str());
}

void DatalogWriter::visitPDGEdge(PDGEdge* e) {
    if (cpgOptions.printNoPDG) {
        return;
    }
    if (e->pdgType() == PDGType::Const) {
        if (e->value().type == Type::I32 || e->value().type == Type::I64) {
            _stream->Writef(
                "edge(%u, %u, \"PDG\", \"%s\", \"%s\", [\"%s\", %s, %f]).\n",
                e->src()->getId(), e->dest()->getId(), e->label().c_str(),
                pdgTypeMap[e->pdgType()].c_str(),
                ConstInst::writeConstType(e->value()).c_str(),
                ConstInst::writeConst(e->value(), false).c_str(), 0.0);
        } else {
            _stream->Writef(
                "edge(%u, %u, \"PDG\", \"%s\", \"%s\", [\"%s\", %u, %s]).\n",
                e->src()->getId(), e->dest()->getId(), e->label().c_str(),
                pdgTypeMap[e->pdgType()].c_str(),
                ConstInst::writeConstType(e->value()).c_str(), 0,
                ConstInst::writeConst(e->value(), false).c_str());
        }
    } else {
        _stream->Writef("edge(%u, %u, \"PDG\", \"%s\", \"%s\", nil).\n",
                        e->src()->getId(), e->dest()->getId(),
                        e->label().c_str(), pdgTypeMap[e->pdgType()].c_str());
    }
}

void DatalogWriter::visitCGEdge(CGEdge* e) {
    if (cpgOptions.printNoCG) {
        return;
    }
    _stream->Writef("edge(%u, %u, \"AST\", \"\", \"\", nil).\n",
                    e->src()->getId(), e->dest()->getId());
}

void DatalogWriter::visitPGEdge(PGEdge* e) {
    if (cpgOptions.printNoPG) {
        return;
    }
    _stream->Writef("edge(%u, %u, \"AST\", \"\", \"\", nil).\n",
                    e->src()->getId(), e->dest()->getId());
}

void DatalogWriter::visitModule(Module* mod) {
    _stream->Writef("module(%u, \"%s\").\n", mod->getId(), mod->name().c_str());
}

void DatalogWriter::visitFunction(Function* func) {
    _stream->Writef("function(%u, \"%s\", %u, %u, %u, %u, %u, %u).\n",
                    func->getId(), func->name().c_str(), func->index(),
                    func->nargs(), func->nlocals(), func->nresults(),
                    func->isImport(), func->isExport());
}

void DatalogWriter::visitFunctionSignature(FunctionSignature* node) {
    _stream->Writef("functionSignature(%u).\n", node->getId());
}

void DatalogWriter::visitParameters(Parameters* node) {
    _stream->Writef("parameters(%u).\n", node->getId());
}

void DatalogWriter::visitInstructions(Instructions* node) {
    _stream->Writef("instructions(%u).\n", node->getId());
}

void DatalogWriter::visitLocals(Locals* node) {
    _stream->Writef("locals(%u).\n", node->getId());
}

void DatalogWriter::visitResults(Results* node) {
    _stream->Writef("results(%u).\n", node->getId());
}

void DatalogWriter::visitElse(Else* node) {
    _stream->Writef("else(%u).\n", node->getId());
}

void DatalogWriter::visitStart(Start* node) {
    _stream->Writef("start(%u).\n", node->getId());
}
void DatalogWriter::visitTrap(Trap* node) {
    _stream->Writef("trap(%u).\n", node->getId());
}

void DatalogWriter::visitVarNode(VarNode* node) {
    _stream->Writef("varNode(%u, \"%s\", %u, \"%s\").\n", node->getId(),
                    node->writeVarType().c_str(), node->index(),
                    node->name().c_str());
}

void DatalogWriter::visitNopInst(NopInst* node) {
    _stream->Writef("nop(%u).\n", node->getId());
}

void DatalogWriter::visitUnreachableInst(UnreachableInst* node) {
    _stream->Writef("unreachable(%u).\n", node->getId());
}

void DatalogWriter::visitReturnInst(ReturnInst* node) {
    _stream->Writef("return(%u).\n", node->getId());
}

void DatalogWriter::visitBrTableInst(BrTableInst* node) {
    _stream->Writef("brTable(%u).\n", node->getId());
}

void DatalogWriter::visitDropInst(DropInst* node) {
    _stream->Writef("drop(%u).\n", node->getId());
}

void DatalogWriter::visitSelectInst(SelectInst* node) {
    _stream->Writef("select(%u).\n", node->getId());
}

void DatalogWriter::visitMemorySizeInst(MemorySizeInst* node) {
    _stream->Writef("memorySize(%u).\n", node->getId());
}

void DatalogWriter::visitMemoryGrowInst(MemoryGrowInst* node) {
    _stream->Writef("memoryGrow(%u).\n", node->getId());
}

void DatalogWriter::visitConstInst(ConstInst* node) {
    if (node->value().type == Type::I32 || node->value().type == Type::I64) {
        _stream->Writef("const(%u, [\"%s\", %s, %f]).\n", node->getId(),
                        ConstInst::writeConstType(node->value()).c_str(),
                        ConstInst::writeConst(node->value(), false).c_str(), 0.0);
    } else {
        _stream->Writef("const(%u, [\"%s\", %d, %s]).\n", node->getId(),
                        ConstInst::writeConstType(node->value()).c_str(), 0,
                        ConstInst::writeConst(node->value(), false).c_str());
    }
}

void DatalogWriter::visitBinaryInst(BinaryInst* node) {
    _stream->Writef("binary(%u, \"%s\").\n", node->getId(),
                    node->opcode().GetName());
}

void DatalogWriter::visitCompareInst(CompareInst* node) {
    _stream->Writef("compare(%u, \"%s\").\n", node->getId(),
                    node->opcode().GetName());
}

void DatalogWriter::visitConvertInst(ConvertInst* node) {
    _stream->Writef("convert(%u, \"%s\").\n", node->getId(),
                    node->opcode().GetName());
}

void DatalogWriter::visitUnaryInst(UnaryInst* node) {
    _stream->Writef("unary(%u, \"%s\").\n", node->getId(),
                    node->opcode().GetName());
}

void DatalogWriter::visitLoadInst(LoadInst* node) {
    _stream->Writef("load(%u, %u).\n", node->getId(), node->offset());
}

void DatalogWriter::visitStoreInst(StoreInst* node) {
    _stream->Writef("store(%u, %u).\n", node->getId(), node->offset());
}

void DatalogWriter::visitBrInst(BrInst* node) {
    _stream->Writef("br(%u, \"%s\").\n", node->getId(), node->label().c_str());
}

void DatalogWriter::visitBrIfInst(BrIfInst* node) {
    _stream->Writef("brIf(%u, \"%s\").\n", node->getId(),
                    node->label().c_str());
}

void DatalogWriter::visitGlobalGetInst(GlobalGetInst* node) {
    _stream->Writef("globalGet(%u, \"%s\").\n", node->getId(),
                    node->label().c_str());
}

void DatalogWriter::visitGlobalSetInst(GlobalSetInst* node) {
    _stream->Writef("globalSet(%u, \"%s\").\n", node->getId(),
                    node->label().c_str());
}

void DatalogWriter::visitLocalGetInst(LocalGetInst* node) {
    _stream->Writef("localGet(%u, \"%s\").\n", node->getId(),
                    node->label().c_str());
}

void DatalogWriter::visitLocalSetInst(LocalSetInst* node) {
    _stream->Writef("localSet(%u, \"%s\").\n", node->getId(),
                    node->label().c_str());
}

void DatalogWriter::visitLocalTeeInst(LocalTeeInst* node) {
    _stream->Writef("localTee(%u, \"%s\").\n", node->getId(),
                    node->label().c_str());
}

void DatalogWriter::visitCallInst(CallInst* node) {
    _stream->Writef("call(%u, \"%s\", %u, %u).\n", node->getId(),
                    node->label().c_str(), node->nargs(), node->nresults());
}

void DatalogWriter::visitCallIndirectInst(CallIndirectInst* node) {
    _stream->Writef("callIndirect(%u, \"%s\", %u, %u).\n", node->getId(),
                    node->label().c_str(), node->nargs(), node->nresults());
}

void DatalogWriter::visitBlockInst(BlockInst* node) {
    _stream->Writef("block(%u, \"%s\", %u).\n", node->getId(),
                    node->label().c_str(), node->nresults());
}

void DatalogWriter::visitLoopInst(LoopInst* node) {
    _stream->Writef("loop(%u, \"%s\", %u).\n", node->getId(),
                    node->label().c_str(), node->nresults());
}

void DatalogWriter::visitBeginBlockInst(BeginBlockInst* node) {
    _stream->Writef("beginBlock(%u, \"%s\").\n", node->getId(),
                    node->label().c_str());
}

void DatalogWriter::visitIfInst(IfInst* node) {
    _stream->Writef("if(%u, %u, %u).\n", node->getId(), node->nresults(),
                    node->hasElse());
}

}  // namespace wasmati
