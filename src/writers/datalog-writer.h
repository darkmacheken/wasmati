#ifndef WASMATI_DATALOG_H
#define WASMATI_DATALOG_H
#include <map>
#include <sstream>
#include "src/graph.h"
#include "src/query.h"

constexpr auto BASE_DL = R"(#ifndef WASMATI_DATALOG 
#define WASMATI_DATALOG

#define false 0
#define true 1
.type bool <: unsigned

.type Const = [
	type:symbol,
	i:number,
	f:float
]

// Declarations
// -- Edges
.decl edge(x:unsigned, y:unsigned, type:symbol, label:symbol, pdgType:symbol, value:Const)
.decl astEdge(x:unsigned, y:unsigned)
.decl cpgEdge(x:unsigned, y:unsigned, label:symbol)
.decl pdgEdge(x:unsigned, y:unsigned, label:symbol, type:symbol, value:Const)
.decl cgEdge(x:unsigned, y:unsigned)
.decl pgEdge(x:unsigned, y:unsigned)
// -- Nodes
.decl node(x:unsigned, type:symbol, name:symbol, index:unsigned, nargs:unsigned, nlocals:unsigned, \
			nresults:unsigned, isImport:bool, isExport:bool, varType:symbol, instType:symbol, \
			opcode:symbol, value:Const, label:symbol, offset:unsigned, hasElse:bool) 
.decl module(x:unsigned, name:symbol)
.decl function(x:unsigned, name:symbol, index:unsigned, narg:unsigned, nlocals:unsigned, nresults:unsigned, \
				isImport:bool, isExport:bool)
.decl varNode(x:unsigned, type:symbol, index:unsigned, name:symbol)
.decl functionSignature(x:unsigned)
.decl instructions(x:unsigned)
.decl parameters(x:unsigned)
.decl locals(x:unsigned)
.decl results(x:unsigned)
.decl else(x:unsigned)
.decl trap(x:unsigned)
.decl start(x:unsigned)
.decl instruction(x:unsigned, type:symbol)
.decl nop(x:unsigned)
.decl unreachable(x:unsigned)
.decl return(x:unsigned)
.decl brTable(x:unsigned)
.decl drop(x:unsigned)
.decl select(x:unsigned)
.decl memorySize(x:unsigned)
.decl memoryGrow(x:unsigned)
.decl const(x:unsigned, value:Const)
.decl binary(x:unsigned, opcode:symbol)
.decl compare(x:unsigned, opcode:symbol)
.decl convert(x:unsigned, opcode:symbol)
.decl unary(x:unsigned, opcode:symbol)
.decl load(x:unsigned, offset:unsigned)
.decl store(x:unsigned, offset:unsigned)
.decl br(x:unsigned, label:symbol)
.decl brIf(x:unsigned, label:symbol)
.decl globalGet(x:unsigned, label:symbol)
.decl globalSet(x:unsigned, label:symbol)
.decl localGet(x:unsigned, label:symbol)
.decl localSet(x:unsigned, label:symbol)
.decl localTee(x:unsigned, label:symbol)
.decl call(x:unsigned, label:symbol, nargs:unsigned, nresults:unsigned)
.decl callIndirect(x:unsigned, label:symbol, nargs:unsigned, nresults:unsigned)
.decl block(x:unsigned, label:symbol, nresults:unsigned)
.decl loop(x:unsigned, label:symbol, nresults:unsigned)
.decl beginBlock(x:unsigned, label:symbol)
.decl if(x:unsigned, nresults:unsigned, hasElse:bool)

// Rules
// -- Edges
astEdge(x, y) :- edge(x, y, "AST", _, _, _).
cpgEdge(x, y, label) :- edge(x, y, "CFG", label, _, _).
pdgEdge(x, y, label, type, value) :- edge(x , y, "PDG", label, type,value).
cgEdge(x,y) :- edge(x, y, "CG", _, _, _).
pgEdge(x,y) :- edge(x, y, "CG", _, _, _).

// -- Nodes
// ---- Other Nodes
module(x, name) :- node(x, "Module", name, _, _, _, _, _, _, _, _, _, _, _, _, _).
function(x, name, index, nargs, nlocals, nresults, isImport, isExport) :- node(x, "Function", name, index , nargs, nlocals, nresults, isImport, isExport, _, _, _, _, _, _, _).
varNode(x, type, index, name) :- node(x, "VarNode", name, index , _, _, _, _, _, type, _, _, _, _, _, _).
functionSignature(x) :- node(x, "FunctionSignature", _, _, _, _, _, _, _, _, _, _, _, _, _, _).
instructions(x) :- node(x, "Instructions", _, _, _, _, _, _, _, _, _, _, _, _, _, _).
parameters(x) :- node(x, "Parameters", _, _, _, _, _, _, _, _, _, _, _, _, _, _).
locals(x) :- node(x, "Locals", _, _, _, _, _, _, _, _, _, _, _, _, _, _).
results(x) :- node(x, "Results", _, _, _, _, _, _, _, _, _, _, _, _, _, _).
else(x) :- node(x, "Else", _, _, _, _, _, _, _, _, _, _, _, _, _, _).
trap(x) :- node(x, "Trap", _, _, _, _, _, _, _, _, _, _, _, _, _, _).
start(x) :- node(x, "Start", _, _, _, _, _, _, _, _, _, _, _, _, _, _).
// ---- Instructions
instruction(x, instType) :- node(x, "Instruction", _, _, _, _, _, _, _, _, instType, _, _, _, _, _).
nop(x) :- node(x, _, _, _, _, _, _, _, _, _, "Nop", _, _, _, _, _).
unreachable(x) :- node(x, _, _, _, _, _, _, _, _, _, "Unreachable", _, _, _, _, _).
return(x) :- node(x, _, _, _, _, _, _, _, _, _, "Return", _, _, _, _, _).
brTable(x) :- node(x, _, _, _, _, _, _, _, _, _, "BrTable", _, _, _, _, _).
drop(x) :- node(x, _, _, _, _, _, _, _, _, _, "Drop", _, _, _, _, _).
select(x) :- node(x, _, _, _, _, _, _, _, _, _, "Select", _, _, _, _, _).
memorySize(x) :- node(x, _, _, _, _, _, _, _, _, _, "MemorySize", _, _, _, _, _).
memoryGrow(x) :- node(x, _, _, _, _, _, _, _, _, _, "MemoryGrow", _, _, _, _, _).
const(x, value) :- node(x, _, _, _, _, _, _, _, _, _, "Const", _, value, _, _, _).
binary(x, opcode) :- node(x, _, _, _, _, _, _, _, _, _, "Binary", opcode, _, _, _, _).
compare(x, opcode) :- node(x, _, _, _, _, _, _, _, _, _, "Compare", opcode, _, _, _, _).
convert(x, opcode) :- node(x, _, _, _, _, _, _, _, _, _, "Convert", opcode, _, _, _, _).
unary(x, opcode) :- node(x, _, _, _, _, _, _, _, _, _, "Unary", opcode, _, _, _, _).
load(x, offset) :- node(x, _, _, _, _, _, _, _, _, _, "Load", _, _, _, offset, _).
store(x, offset) :- node(x, _, _, _, _, _, _, _, _, _, "Store", _, _, _, offset, _).
br(x, label) :- node(x, _, _, _, _, _, _, _, _, _, "Br", _, _, label, _, _).
brIf(x, label) :- node(x, _, _, _, _, _, _, _, _, _, "BrIf", _, _, label, _, _).
globalGet(x, label) :- node(x, _, _, _, _, _, _, _, _, _, "GlobalGet", _, _, label, _, _).
globalSet(x, label) :- node(x, _, _, _, _, _, _, _, _, _, "GlobalSet", _, _, label, _, _).
localGet(x, label) :- node(x, _, _, _, _, _, _, _, _, _, "LocalGet", _, _, label, _, _).
localSet(x, label) :- node(x, _, _, _, _, _, _, _, _, _, "LocalSet", _, _, label, _, _).
localTee(x, label) :- node(x, _, _, _, _, _, _, _, _, _, "LocalTee", _, _, label, _, _).
call(x, label, nargs, nresults) :- node(x, _, _, _, nargs, _, nresults, _, _, _, "Call", _, _, label, _, _).
callIndirect(x, label, nargs, nresults) :- node(x, _, _, _, nargs, _, nresults, _, _, _, "CallIndirect", _, _, label, _, _).
block(x, label, nresults) :- node(x, _, _, _, _, _, nresults, _, _, _, "Block", _, _, label, _, _).
loop(x, label, nresults) :- node(x, _, _, _, _, _, nresults, _, _, _, "Loop", _, _, label, _, _).
beginBlock(x, label) :- node(x, _, _, _, _, _, _, _, _, _, "BeginBlock", _, _, label, _, _).
if(x, nresults,hasElse) :- node(x, _, _, _, _, _, nresults, _, _, _, "BeginBlock", _, _, _, _, hasElse).

.input edge(IO=file, delimiter=",")
.input node(IO=file, delimiter=",")

// -- Basic Queries
.decl reaches(x:unsigned, y:unsigned, type:symbol)
reaches(X, Y, type) :- edge(X, Y, type, _, _, _).
reaches(X, Y, type) :- edge(X, Z, type, _, _, _), reaches(Z, Y, type).

#endif)";

namespace wasmati {

class DatalogWriter : public GraphWriter {
    wabt::Stream* _edges;
    wabt::Stream* _nodes;

public:
    DatalogWriter(wabt::Stream* stream,
                  wabt::Stream* edges,
                  wabt::Stream* nodes,
                  Graph* graph)
        : GraphWriter(stream, graph), _edges(edges), _nodes(nodes) {}

    void writeGraph() override {
        NodeSet loopsInsts;
        if (!cpgOptions.loopName.empty()) {
            loopsInsts = Queries::loopsInsts(cpgOptions.loopName);
        }

        writePutsln(BASE_DL);

        for (auto const& node : _graph->getNodes()) {
            if (!loopsInsts.empty() && loopsInsts.count(node) == 0) {
                continue;
            }
            node->acceptEdges(this);

            if (cpgOptions.printAll) {
                node->accept(this);
            } else if (cpgOptions.printAST &&
                       node->hasEdgesOf(EdgeType::AST)) {  // AST
                node->accept(this);
            } else if (cpgOptions.printCFG &&
                       node->hasEdgesOf(EdgeType::CFG)) {  // CFG
                node->accept(this);
            } else if (cpgOptions.printPDG &&
                       node->hasEdgesOf(EdgeType::PDG)) {  // PDG
                node->accept(this);
            } else if (cpgOptions.printCG &&
                       node->hasEdgesOf(EdgeType::CG)) {  // CG
                node->accept(this);
            } else if (cpgOptions.printPG &&
                       node->hasEdgesOf(EdgeType::PG)) {  // PDG
                node->accept(this);
            }
        }
    }

    // Edges
    void visitASTEdge(ASTEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printAST)) {
            return;
        }
        _edges->Writef("%u,%u,AST,,,nil\n", e->src()->getId(),
                       e->dest()->getId());
    }
    void visitCFGEdge(CFGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printCFG)) {
            return;
        }

        _edges->Writef("%u,%u,CFG,%s,,nil\n", e->src()->getId(),
                       e->dest()->getId(), e->label().c_str());
    }
    void visitPDGEdge(PDGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printPDG)) {
            return;
        }
        if (e->pdgType() == PDGType::Const) {
            if (e->value().type == Type::I32 || e->value().type == Type::I64) {
                _edges->Writef("%u,%u,PDG,%s,%s,[%s,%s,%f]\n",
                               e->src()->getId(), e->dest()->getId(),
                               e->label().c_str(), e->writePdgType().c_str(),
                               Utils::writeConstType(e->value()).c_str(),
                               Utils::writeConst(e->value(), false).c_str(),
                               0.0);
            } else {
                _edges->Writef("%u,%u,PDG,%s,%s,[%s,%u,%s]\n",
                               e->src()->getId(), e->dest()->getId(),
                               e->label().c_str(), e->writePdgType().c_str(),
                               Utils::writeConstType(e->value()).c_str(), 0,
                               Utils::writeConst(e->value(), false).c_str());
            }
        } else {
            _edges->Writef("%u,%u,PDG,%s,%s,nil\n", e->src()->getId(),
                           e->dest()->getId(), e->label().c_str(),
                           e->writePdgType().c_str());
        }
    }
    void visitCGEdge(CGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printCG)) {
            return;
        }
        _edges->Writef("%u,%u,CG,,,nil\n", e->src()->getId(),
                       e->dest()->getId());
    }
    void visitPGEdge(PGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printPG)) {
            return;
        }
        _edges->Writef("%u,%u,PG,,,nil\n", e->src()->getId(),
                       e->dest()->getId());
    }

private:
    // Inherited via GraphWriter
    void visitModule(Module* node) override {
        _nodes->Writef("%u,Module,%s,0,0,0,0,0,0,,,,[,0,0],,0,0\n",
                       node->getId(), node->name().c_str());
    }
    void visitFunction(Function* node) override {
        _nodes->Writef("%u,Function,%s,%u,%u,%u,%u,%u,%u,,,,[,0,0],,0,0\n",
                       node->getId(), node->name().c_str(), node->index(),
                       node->nargs(), node->nlocals(), node->nresults(),
                       node->isImport(), node->isExport());
    }
    void visitFunctionSignature(FunctionSignature* node) override {
        _nodes->Writef("%u,FunctionSignature,,0,0,0,0,0,0,,,,[,0,0],,0,0\n",
                       node->getId());
    }
    void visitParameters(Parameters* node) override {
        _nodes->Writef("%u,Parameters,,0,0,0,0,0,0,,,,[,0,0],,0,0\n",
                       node->getId());
    }
    void visitInstructions(Instructions* node) override {
        _nodes->Writef("%u,Instructions,,0,0,0,0,0,0,,,,[,0,0],,0,0\n",
                       node->getId());
    }
    void visitLocals(Locals* node) override {
        _nodes->Writef("%u,Locals,,0,0,0,0,0,0,,,,[,0,0],,0,0\n",
                       node->getId());
    }
    void visitResults(Results* node) override {
        _nodes->Writef("%u,Results,,0,0,0,0,0,0,,,,[,0,0],,0,0\n",
                       node->getId());
    }
    void visitElse(Else* node) override {
        _nodes->Writef("%u,Else,,0,0,0,0,0,0,,,,[,0,0],,0,0\n", node->getId());
    }
    void visitStart(Start* node) override {
        _nodes->Writef("%u,Start,,0,0,0,0,0,0,,,,[,0,0],,0,0\n", node->getId());
    }
    void visitTrap(Trap* node) override {
        _nodes->Writef("%u,Trap,,0,0,0,0,0,0,,,,[,0,0],,0,0\n", node->getId());
    }
    void visitVarNode(VarNode* node) override {
        _nodes->Writef("%u,VarNode,%s,%u,0,0,0,0,0,%s,,,[,0,0],,0,0\n",
                       node->getId(), node->name().c_str(), node->index(),
                       node->writeVarType().c_str());
    }
    void visitNopInst(NopInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,Nop,,[,0,0],,0,0\n",
                       node->getId());
    }
    void visitUnreachableInst(UnreachableInst* node) override {
        _nodes->Writef(
            "%u,Instruction,,0,0,0,0,0,0,,Unreachable,,[,0,0],,0,0\n",
            node->getId());
    }
    void visitReturnInst(ReturnInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,Return,,[,0,0],,0,0\n",
                       node->getId());
    }
    void visitBrTableInst(BrTableInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,BrTable,,[,0,0],,0,0\n",
                       node->getId());
    }
    void visitDropInst(DropInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,Drop,,[,0,0],,0,0\n",
                       node->getId());
    }
    void visitSelectInst(SelectInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,Select,,[,0,0],,0,0\n",
                       node->getId());
    }
    void visitMemorySizeInst(MemorySizeInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,MemorySize,,[,0,0],,0,0\n",
                       node->getId());
    }
    void visitMemoryGrowInst(MemoryGrowInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,MemoryGrow,,[,0,0],,0,0\n",
                       node->getId());
    }
    void visitConstInst(ConstInst* node) override {
        if (node->value().type == Type::I32 ||
            node->value().type == Type::I64) {
            _nodes->Writef(
                "%u,Instruction,,0,0,0,0,0,0,,Const,,[%s,%s,%f],,0,0\n",
                node->getId(), Utils::writeConstType(node->value()).c_str(),
                Utils::writeConst(node->value(), false).c_str(), 0.0);
        } else {
            _nodes->Writef(
                "%u,Instruction,,0,0,0,0,0,0,,Const,,[%s,%d,%s],,0,0\n",
                node->getId(), Utils::writeConstType(node->value()).c_str(), 0,
                Utils::writeConst(node->value(), false).c_str());
        }
    }
    void visitBinaryInst(BinaryInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,Binary,%s,[,0,0],,0,0\n",
                       node->getId(), node->opcode().GetName());
    }
    void visitCompareInst(CompareInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,Compare,%s,[,0,0],,0,0\n",
                       node->getId(), node->opcode().GetName());
    }
    void visitConvertInst(ConvertInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,Convert,%s,[,0,0],,0,0\n",
                       node->getId(), node->opcode().GetName());
    }
    void visitUnaryInst(UnaryInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,Unary,%s,[,0,0],,0,0\n",
                       node->getId(), node->opcode().GetName());
    }
    void visitLoadInst(LoadInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,Load,,[,0,0],,%u,0\n",
                       node->getId(), node->offset());
    }
    void visitStoreInst(StoreInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,Store,,[,0,0],,%u,0\n",
                       node->getId(), node->offset());
    }
    void visitBrInst(BrInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,Br,,[,0,0],%s,0,0\n",
                       node->getId(), node->label().c_str());
    }
    void visitBrIfInst(BrIfInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,BrIf,,[,0,0],%s,0,0\n",
                       node->getId(), node->label().c_str());
    }
    void visitGlobalGetInst(GlobalGetInst* node) override {
        _nodes->Writef(
            "%u,Instruction,,0,0,0,0,0,0,,GlobalGet,,[,0,0],%s,0,0\n",
            node->getId(), node->label().c_str());
    }
    void visitGlobalSetInst(GlobalSetInst* node) override {
        _nodes->Writef(
            "%u,Instruction,,0,0,0,0,0,0,,GlobalSet,,[,0,0],%s,0,0\n",
            node->getId(), node->label().c_str());
    }
    void visitLocalGetInst(LocalGetInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,LocalGet,,[,0,0],%s,0,0\n",
                       node->getId(), node->label().c_str());
    }
    void visitLocalSetInst(LocalSetInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,LocalSet,,[,0,0],%s,0,0\n",
                       node->getId(), node->label().c_str());
    }
    void visitLocalTeeInst(LocalTeeInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,0,0,0,,LocalTee,,[,0,0],%s,0,0\n",
                       node->getId(), node->label().c_str());
    }
    void visitCallInst(CallInst* node) override {
        _nodes->Writef("%u,Instruction,,0,%u,0,%u,0,0,,Call,,[,0,0],%s,0,0\n",
                       node->getId(), node->nargs(), node->nresults(),
                       node->label().c_str());
    }
    void visitCallIndirectInst(CallIndirectInst* node) override {
        _nodes->Writef(
            "%u,Instruction,,0,%u,0,%u,0,0,,CallIndirect,,[,0,0],%s,0,0\n",
            node->getId(), node->nargs(), node->nresults(),
            node->label().c_str());
    }
    void visitBeginBlockInst(BeginBlockInst* node) override {
        _nodes->Writef(
            "%u,Instruction,,0,0,0,0,0,0,,BeginBlock,,[,0,0],%s,0,0\n",
            node->getId(), node->label().c_str());
    }
    void visitBlockInst(BlockInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,%u,0,0,,Block,,[,0,0],%s,0,0\n",
                       node->getId(), node->nresults(), node->label().c_str());
    }
    void visitLoopInst(LoopInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,%u,0,0,,Loop,,[,0,0],%s,0,0\n",
                       node->getId(), node->nresults(), node->label().c_str());
    }
    void visitIfInst(IfInst* node) override {
        _nodes->Writef("%u,Instruction,,0,0,0,%u,0,0,,If,,[,0,0],,0,%u\n",
                       node->getId(), node->nresults(), node->hasElse());
    }
};

}  // namespace wasmati

#endif /* end of WASMATI_DOT_H */
