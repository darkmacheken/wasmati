#ifndef WASMATI_CSV_H
#define WASMATI_CSV_H
#include <map>
#include "src/graph.h"
#include "src/query.h"

namespace wasmati {

class CSVWriter : public GraphWriter {
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
        _stream->Writef("%u,%u,%s,,,,,\n", e->src()->getId(),
                        e->dest()->getId(),
                        EDGE_TYPES_MAP.at(EdgeType::AST).c_str());
    }
    void visitCFGEdge(CFGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printCFG)) {
            return;
        }

        _stream->Writef(
            "%u,%u,%s,%s,,,,\n", e->src()->getId(), e->dest()->getId(),
            EDGE_TYPES_MAP.at(EdgeType::CFG).c_str(), e->label().c_str());
    }
    void visitPDGEdge(PDGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printPDG)) {
            return;
        }
        if (e->pdgType() == PDGType::Const) {
            _stream->Writef(
                "%u,%u,%s,%s,%s,%s,%s\n", e->src()->getId(), e->dest()->getId(),
                EDGE_TYPES_MAP.at(EdgeType::PDG).c_str(), e->label().c_str(),
                PDG_TYPE_MAP.at(e->pdgType()).c_str(),
                Utils::writeConstType(e->value()).c_str(),
                Utils::writeConst(e->value(), false).c_str());
        } else {
            _stream->Writef(
                "%u,%u,%s,%s,%s,,,\n", e->src()->getId(), e->dest()->getId(),
                EDGE_TYPES_MAP.at(EdgeType::PDG).c_str(), e->label().c_str(),
                PDG_TYPE_MAP.at(e->pdgType()).c_str());
        }
    }
    void visitCGEdge(CGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printCG)) {
            return;
        }
        _stream->Writef("%u,%u,%s,,,,,\n", e->src()->getId(),
                        e->dest()->getId(),
                        EDGE_TYPES_MAP.at(EdgeType::CG).c_str());
    }

private:
    // Inherited via GraphWriter
    void visitModule(Module* node) override {
        _stream->Writef("%u,%s,%s,,,,,,,,,,,,,,,\n", node->getId(),
                        NODE_TYPE_MAP.at(node->type()).c_str(),
                        node->name().c_str());
    }
    void visitFunction(Function* node) override {
        _stream->Writef("%u,%s,%s,%u,%u,%u,%u,%u,%u,,,,,,,,,\n", node->getId(),
                        NODE_TYPE_MAP.at(node->type()).c_str(),
                        node->name().c_str(), node->index(), node->nargs(),
                        node->nlocals(), node->nresults(), node->isImport(),
                        node->isExport());
    }
    void visitFunctionSignature(FunctionSignature* node) override {
        visitSimpleNode(node);
    }
    void visitParameters(Parameters* node) override { visitSimpleNode(node); }
    void visitInstructions(Instructions* node) override {
        visitSimpleNode(node);
    }
    void visitLocals(Locals* node) override { visitSimpleNode(node); }
    void visitResults(Results* node) override { visitSimpleNode(node); }
    void visitElse(Else* node) override { visitSimpleNode(node); }
    void visitStart(Start* node) override { visitSimpleNode(node); }
    void visitTrap(Trap* node) override { visitSimpleNode(node); }
    void visitVarNode(VarNode* node) override {
        _stream->Writef("%u,%s,%s,%u,,,,,,%s,,,,,,,,\n", node->getId(),
                        NODE_TYPE_MAP.at(node->type()).c_str(),
                        node->name().c_str(), node->index(),
                        node->writeVarType().c_str());
    }
    void visitNopInst(NopInst* node) override { visitSimpleInstNode(node); }
    void visitUnreachableInst(UnreachableInst* node) override {
        visitSimpleInstNode(node);
    }
    void visitReturnInst(ReturnInst* node) override {
        visitSimpleInstNode(node);
    }
    void visitBrTableInst(BrTableInst* node) override {
        visitSimpleInstNode(node);
    }
    void visitDropInst(DropInst* node) override { visitSimpleInstNode(node); }
    void visitSelectInst(SelectInst* node) override {
        visitSimpleInstNode(node);
    }
    void visitMemorySizeInst(MemorySizeInst* node) override {
        visitSimpleInstNode(node);
    }
    void visitMemoryGrowInst(MemoryGrowInst* node) override {
        visitSimpleInstNode(node);
    }
    void visitConstInst(ConstInst* node) override {
        _stream->Writef("%u,%s,,,,,,,,,%s,,%s,%s,,,,\n", node->getId(),
                        NODE_TYPE_MAP.at(node->type()).c_str(),
                        INST_TYPE_MAP.at(node->instType()).c_str(),
                        Utils::writeConstType(node->value()).c_str(),
                        Utils::writeConst(node->value(), false).c_str());
    }
    void visitBinaryInst(BinaryInst* node) override {
        visitOpcodeInstNode(node);
    }
    void visitCompareInst(CompareInst* node) override {
        visitOpcodeInstNode(node);
    }
    void visitConvertInst(ConvertInst* node) override {
        visitOpcodeInstNode(node);
    }
    void visitUnaryInst(UnaryInst* node) override { visitOpcodeInstNode(node); }
    void visitLoadInst(LoadInst* node) override {
        visitLoadStoreInstNode(node);
    }
    void visitStoreInst(StoreInst* node) override {
        visitLoadStoreInstNode(node);
    }
    void visitBrInst(BrInst* node) override { visitLabelInstNode(node); }
    void visitBrIfInst(BrIfInst* node) override { visitLabelInstNode(node); }
    void visitGlobalGetInst(GlobalGetInst* node) override {
        visitLabelInstNode(node);
    }
    void visitGlobalSetInst(GlobalSetInst* node) override {
        visitLabelInstNode(node);
    }
    void visitLocalGetInst(LocalGetInst* node) override {
        visitLabelInstNode(node);
    }
    void visitLocalSetInst(LocalSetInst* node) override {
        visitLabelInstNode(node);
    }
    void visitLocalTeeInst(LocalTeeInst* node) override {
        visitLabelInstNode(node);
    }
    void visitCallInst(CallInst* node) override { visitCallInstNode(node); }
    void visitCallIndirectInst(CallIndirectInst* node) override {
        visitCallInstNode(node);
    }
    void visitBeginBlockInst(BeginBlockInst* node) override {
        _stream->Writef("%u,%s,,,,,,,,,%s,,,,%s,,,\n", node->getId(),
                        NODE_TYPE_MAP.at(node->type()).c_str(),
                        INST_TYPE_MAP.at(ExprType::First).c_str(),
                        node->label().c_str());
    }
    void visitBlockInst(BlockInst* node) override { visitBlockInstNode(node); }
    void visitLoopInst(LoopInst* node) override { visitBlockInstNode(node); }
    void visitIfInst(IfInst* node) override {
        _stream->Writef(
            "%u,%s,,,,,%u,,,,%s,,,,,,%u\n", node->getId(),
            NODE_TYPE_MAP.at(node->type()).c_str(), node->nresults(),
            INST_TYPE_MAP.at(node->instType()).c_str(), node->hasElse());
    }

private:
    inline void visitSimpleNode(Node* node) {
        _stream->Writef("%u,%s,,,,,,,,,,,,,,,,\n", node->getId(),
                        NODE_TYPE_MAP.at(node->type()).c_str());
    }

    inline void visitSimpleInstNode(Node* node) {
        _stream->Writef("%u,%s,,,,,,,,,%s,,,,,,,\n", node->getId(),
                        NODE_TYPE_MAP.at(node->type()).c_str(),
                        INST_TYPE_MAP.at(node->instType()).c_str());
    }

    inline void visitOpcodeInstNode(Node* node) {
        _stream->Writef("%u,%s,,,,,,,,,%s,%s,,,,,,\n", node->getId(),
                        NODE_TYPE_MAP.at(node->type()).c_str(),
                        INST_TYPE_MAP.at(node->instType()).c_str(),
                        node->opcode().GetName());
    }

    inline void visitLoadStoreInstNode(Node* node) {
        _stream->Writef("%u,%s,,,,,,,,,%s,%s,,,,%u,,\n", node->getId(),
                        NODE_TYPE_MAP.at(node->type()).c_str(),
                        INST_TYPE_MAP.at(node->instType()).c_str(),
                        node->opcode().GetName(), node->offset());
    }
    inline void visitLabelInstNode(Node* node) {
        _stream->Writef("%u,%s,,,,,,,,,%s,,,,%s,,,\n", node->getId(),
                        NODE_TYPE_MAP.at(node->type()).c_str(),
                        INST_TYPE_MAP.at(node->instType()).c_str(),
                        node->label().c_str());
    }
    inline void visitCallInstNode(Node* node) {
        _stream->Writef("%u,%s,,,%u,,%u,,,,%s,,,,%s,,,\n", node->getId(),
                        NODE_TYPE_MAP.at(node->type()).c_str(), node->nargs(),
                        node->nresults(),
                        INST_TYPE_MAP.at(node->instType()).c_str(),
                        node->label().c_str());
    }

    inline void visitBlockInstNode(Node* node) {
        _stream->Writef(
            "%u,%s,,,,,%u,,,,%s,,,,%s,,,\n", node->getId(),
            NODE_TYPE_MAP.at(node->type()).c_str(), node->nresults(),
            INST_TYPE_MAP.at(node->instType()).c_str(), node->label().c_str());
    }
};

}  // namespace wasmati

#endif /* end of WASMATI_DOT_H */
