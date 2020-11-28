#ifndef WASMATI_CSV_H
#define WASMATI_CSV_H
#include <map>
#include "graph.h"
#include "query.h"

namespace wasmati {

class CSVWriter : public GraphWriter {
    std::map<PDGType, std::string> pdgTypeMap = {
        {PDGType::Const, "Const"},
        {PDGType::Control, "Control"},
        {PDGType::Function, "Function"},
        {PDGType::Global, "Global"},
        {PDGType::Local, "Local"}};

public:
    CSVWriter(wabt::Stream* stream, Graph* graph)
        : GraphWriter(stream, graph) {}

    void writeGraph() override {
        NodeSet loopsInsts;
        if (!cpgOptions.loopName.empty()) {
            loopsInsts = Queries::loopsInsts(cpgOptions.loopName);
        }
        auto nodes = _graph->getNodes();
        std::sort(nodes.begin(), nodes.end(), Compare());

        for (auto const& node : nodes) {
            if (!loopsInsts.empty() && loopsInsts.count(node) == 0) {
                continue;
            }

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
        writePutsln("-");
        for (auto const& node : _graph->getNodes()) {
            node->acceptEdges(this);
        }
    }

    // Edges
    void visitASTEdge(ASTEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printAST)) {
            return;
        }
        _stream->Writef("%u,%u,AST,,,,,\n", e->src()->getId(),
                        e->dest()->getId());
    }
    void visitCFGEdge(CFGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printCFG)) {
            return;
        }

        _stream->Writef("%u,%u,CFG,%s,,,,\n", e->src()->getId(),
                        e->dest()->getId(), e->label().c_str());
    }
    void visitPDGEdge(PDGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printPDG)) {
            return;
        }
        if (e->pdgType() == PDGType::Const) {
            _stream->Writef("%u,%u,PDG,%s,%s,%s,%s\n", e->src()->getId(),
                            e->dest()->getId(), e->label().c_str(),
                            pdgTypeMap[e->pdgType()].c_str(),
                            Utils::writeConstType(e->value()).c_str(),
                            Utils::writeConst(e->value(), false).c_str());
        } else {
            _stream->Writef("%u,%u,PDG,%s,%s,,,\n", e->src()->getId(),
                            e->dest()->getId(), e->label().c_str(),
                            pdgTypeMap[e->pdgType()].c_str());
        }
    }
    void visitCGEdge(CGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printCG)) {
            return;
        }
        _stream->Writef("%u,%u,CG,,,,,\n", e->src()->getId(),
                        e->dest()->getId());
    }
    void visitPGEdge(PGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printPG)) {
            return;
        }
        _stream->Writef("%u,%u,PG,,,,,\n", e->src()->getId(),
                        e->dest()->getId());
    }

private:
    // Inherited via GraphWriter
    void visitModule(Module* node) override {
        _stream->Writef("%u,Module,%s,,,,,,,,,,,,,,,\n", node->getId(),
                        node->name().c_str());
    }
    void visitFunction(Function* node) override {
        _stream->Writef("%u,Function,%s,%u,%u,%u,%u,%u,%u,,,,,,,,,\n",
                        node->getId(), node->name().c_str(), node->index(),
                        node->nargs(), node->nlocals(), node->nresults(),
                        node->isImport(), node->isExport());
    }
    void visitFunctionSignature(FunctionSignature* node) override {
        _stream->Writef("%u,FunctionSignature,,,,,,,,,,,,,,,,\n",
                        node->getId());
    }
    void visitParameters(Parameters* node) override {
        _stream->Writef("%u,Parameters,,,,,,,,,,,,,,,,\n", node->getId());
    }
    void visitInstructions(Instructions* node) override {
        _stream->Writef("%u,Instructions,,,,,,,,,,,,,,,,\n", node->getId());
    }
    void visitLocals(Locals* node) override {
        _stream->Writef("%u,Locals,,,,,,,,,,,,,,,,\n", node->getId());
    }
    void visitResults(Results* node) override {
        _stream->Writef("%u,Results,,,,,,,,,,,,,,,,\n", node->getId());
    }
    void visitElse(Else* node) override {
        _stream->Writef("%u,Else,,,,,,,,,,,,,,,,\n", node->getId());
    }
    void visitStart(Start* node) override {
        _stream->Writef("%u,Start,,,,,,,,,,,,,,,,\n", node->getId());
    }
    void visitTrap(Trap* node) override {
        _stream->Writef("%u,Trap,,,,,,,,,,,,,,,,\n", node->getId());
    }
    void visitVarNode(VarNode* node) override {
        _stream->Writef("%u,VarNode,%s,%u,,,,,,%s,,,,,,,,\n", node->getId(),
                        node->name().c_str(), node->index(),
                        node->writeVarType().c_str());
    }
    void visitNopInst(NopInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,Nop,,,,,,,\n", node->getId());
    }
    void visitUnreachableInst(UnreachableInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,Unreachable,,,,,,,\n",
                        node->getId());
    }
    void visitReturnInst(ReturnInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,Return,,,,,,,\n",
                        node->getId());
    }
    void visitBrTableInst(BrTableInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,BrTable,,,,,,,\n",
                        node->getId());
    }
    void visitDropInst(DropInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,Drop,,,,,,,\n", node->getId());
    }
    void visitSelectInst(SelectInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,Select,,,,,,,\n",
                        node->getId());
    }
    void visitMemorySizeInst(MemorySizeInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,MemorySize,,,,,,,\n",
                        node->getId());
    }
    void visitMemoryGrowInst(MemoryGrowInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,MemoryGrow,,,,,,,\n",
                        node->getId());
    }
    void visitConstInst(ConstInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,Const,,%s,%s,,,,\n",
                        node->getId(),
                        Utils::writeConstType(node->value()).c_str(),
                        Utils::writeConst(node->value(), false).c_str());
    }
    void visitBinaryInst(BinaryInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,Binary,%s,,,,,,\n",
                        node->getId(), node->opcode().GetName());
    }
    void visitCompareInst(CompareInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,Compare,%s,,,,,,\n",
                        node->getId(), node->opcode().GetName());
    }
    void visitConvertInst(ConvertInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,Convert,%s,,,,,,\n",
                        node->getId(), node->opcode().GetName());
    }
    void visitUnaryInst(UnaryInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,Unary,%s,,,,,,\n",
                        node->getId(), node->opcode().GetName());
    }
    void visitLoadInst(LoadInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,Load,%s,,,,%u,,\n",
                        node->getId(), node->opcode().GetName(),
                        node->offset());
    }
    void visitStoreInst(StoreInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,Store,%s,,,,%u,,\n",
                        node->getId(), node->opcode().GetName(),
                        node->offset());
    }
    void visitBrInst(BrInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,Br,,,,%s,,,\n", node->getId(),
                        node->label().c_str());
    }
    void visitBrIfInst(BrIfInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,BrIf,,,,%s,,,\n", node->getId(),
                        node->label().c_str());
    }
    void visitGlobalGetInst(GlobalGetInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,GlobalGet,,,,%s,,,\n",
                        node->getId(), node->label().c_str());
    }
    void visitGlobalSetInst(GlobalSetInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,GlobalSet,,,,%s,,,\n",
                        node->getId(), node->label().c_str());
    }
    void visitLocalGetInst(LocalGetInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,LocalGet,,,,%s,,,\n",
                        node->getId(), node->label().c_str());
    }
    void visitLocalSetInst(LocalSetInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,LocalSet,,,,%s,,,\n",
                        node->getId(), node->label().c_str());
    }
    void visitLocalTeeInst(LocalTeeInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,LocalTee,,,,%s,,,\n",
                        node->getId(), node->label().c_str());
    }
    void visitCallInst(CallInst* node) override {
        _stream->Writef("%u,Instruction,,,%u,,%u,,,,Call,,,,%s,,,\n",
                        node->getId(), node->nargs(), node->nresults(),
                        node->label().c_str());
    }
    void visitCallIndirectInst(CallIndirectInst* node) override {
        _stream->Writef("%u,Instruction,,,%u,,%u,,,,CallIndirect,,,,%s,,,\n",
                        node->getId(), node->nargs(), node->nresults(),
                        node->label().c_str());
    }
    void visitBeginBlockInst(BeginBlockInst* node) override {
        _stream->Writef("%u,Instruction,,,,,,,,,BeginBlock,,,,%s,,,\n",
                        node->getId(), node->label().c_str());
    }
    void visitBlockInst(BlockInst* node) override {
        _stream->Writef("%u,Instruction,,,,,%u,,,,Block,,,,%s,,,\n",
                        node->getId(), node->nresults(), node->label().c_str());
    }
    void visitLoopInst(LoopInst* node) override {
        _stream->Writef("%u,Instruction,,,,,%u,,,,Loop,,,,%s,,,\n",
                        node->getId(), node->nresults(), node->label().c_str());
    }
    void visitIfInst(IfInst* node) override {
        _stream->Writef("%u,Instruction,,,,,%u,,,,If,,,,,,%u\n", node->getId(),
                        node->nresults(), node->hasElse());
    }
};

}  // namespace wasmati

#endif /* end of WASMATI_DOT_H */
