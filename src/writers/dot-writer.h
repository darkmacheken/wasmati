#ifndef WASMATI_DOT_H
#define WASMATI_DOT_H
#include "src/graph.h"
#include "src/query.h"

namespace wasmati {

class DotWriter : public GraphWriter {
    std::vector<std::vector<int>> _depth;

public:
    DotWriter(wabt::Stream* stream, Graph* graph)
        : GraphWriter(stream, graph) {}

    void writeGraph() override {
        NodeSet loopsInsts;
        if (!cpgOptions.loopName.empty()) {
            loopsInsts = Queries::loopsInsts(cpgOptions.loopName);
        }

        writePuts("digraph G {");
        writePuts("graph [rankdir=TD];");
        writePuts("node [shape=none];");

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
            }
        }

        if (cpgOptions.printAll) {
            setSameRank();
        }

        writePuts("}");
    }

    // Edges
    void visitASTEdge(ASTEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printAST)) {
            return;
        }
        _stream->Writef("%u -> %u [color=forestgreen]\n", e->src()->getId(),
                        e->dest()->getId());
    }
    void visitCFGEdge(CFGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printCFG)) {
            return;
        }
        if (e->_label.empty()) {
            _stream->Writef("%u -> %u [color=red]\n", e->src()->getId(),
                            e->dest()->getId());
        } else {
            _stream->Writef("%u -> %u [color=red fontcolor=red label=\"%s\"]\n",
                            e->src()->getId(), e->dest()->getId(),
                            e->label().c_str());
        }
    }
    void visitPDGEdge(PDGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printPDG)) {
            return;
        }
        if (e->_label.empty()) {
            _stream->Writef("%u -> %u [color=blue]\n", e->src()->getId(),
                            e->dest()->getId());
        } else {
            _stream->Writef(
                "%u -> %u [color=blue fontcolor=blue label=\"%s\"]\n",
                e->src()->getId(), e->dest()->getId(), e->label().c_str());
        }
    }
    void visitCGEdge(CGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printCG)) {
            return;
        }
        _stream->Writef("%u -> %u [color=mediumpurple3]\n", e->src()->getId(),
                        e->dest()->getId());
    }

private:
    // Inherited via GraphWriter
    void visitModule(Module* node) override {
        if (node->name().empty()) {
            _stream->Writef(
                "%u [label="
                "<<TABLE>"
                "<TR><TD>Module</TD></TR>"
                "</TABLE>>];\n",
                node->getId());
        } else {
            _stream->Writef(
                "%u [label="
                "<<TABLE>"
                "<TR><TD>module</TD></TR>"
                "<TR><TD>name = %s</TD></TR>"
                "</TABLE>>];\n",
                node->getId(), node->name().c_str());
        }
    }
    void visitFunction(Function* node) override {
        _stream->Writef(
            "%u [label="
            "<<TABLE>"
            "<TR><TD>Function</TD></TR>"
            "<TR><TD>name = %s</TD></TR>"
            "</TABLE>>];\n",
            node->getId(), node->name().c_str());
    }
    void visitFunctionSignature(FunctionSignature* node) override {
        visitSimpleNode(node->getId(), node->getNodeName());
    }
    void visitParameters(Parameters* node) override {
        visitSimpleNode(node->getId(), node->getNodeName());
    }
    void visitInstructions(Instructions* node) override {
        visitSimpleNode(node->getId(), node->getNodeName());
    }
    void visitLocals(Locals* node) override {
        visitSimpleNode(node->getId(), node->getNodeName());
    }
    void visitResults(Results* node) override {
        visitSimpleNode(node->getId(), node->getNodeName());
    }
    void visitElse(Else* node) override {
        visitSimpleNode(node->getId(), node->getNodeName());
    }
    void visitStart(Start* node) override {
        visitSimpleNode(node->getId(), node->getNodeName());
    }
    void visitTrap(Trap* node) override {
        visitSimpleNode(node->getId(), node->getNodeName());
    }
    void visitVarNode(VarNode* node) override {
        _stream->Writef(
            "%u [label="
            "<<TABLE>"
            "<TR><TD>type</TD><TD>%s</TD></TR>"
            "<TR><TD>name</TD><TD>%s</TD></TR>"
            "</TABLE>>];\n",
            node->getId(), node->writeVarType().c_str(), node->name().c_str());
    }
    void visitNopInst(NopInst* node) override {
        visitSimpleNode(node->getId(), Opcode::Nop_Opcode.GetName());
    }
    void visitUnreachableInst(UnreachableInst* node) override {
        visitSimpleNode(node->getId(), Opcode::Unreachable_Opcode.GetName());
    }
    void visitReturnInst(ReturnInst* node) override {
        visitSimpleNode(node->getId(), Opcode::Return_Opcode.GetName());
    }
    void visitBrTableInst(BrTableInst* node) override {
        visitSimpleNode(node->getId(), Opcode::BrTable_Opcode.GetName());
    }
    void visitDropInst(DropInst* node) override {
        visitSimpleNode(node->getId(), Opcode::Drop_Opcode.GetName());
    }
    void visitSelectInst(SelectInst* node) override {
        visitSimpleNode(node->getId(), Opcode::Select_Opcode.GetName());
    }
    void visitMemorySizeInst(MemorySizeInst* node) override {
        visitSimpleNode(node->getId(), Opcode::MemorySize_Opcode.GetName());
    }
    void visitMemoryGrowInst(MemoryGrowInst* node) override {
        visitSimpleNode(node->getId(), Opcode::MemoryGrow_Opcode.GetName());
    }
    void visitConstInst(ConstInst* node) override {
        visitSimpleNode(node->getId(), Utils::writeConst(node->value()));
    }
    void visitBinaryInst(BinaryInst* node) override {
        visitSimpleNode(node->getId(), node->opcode().GetName());
    }
    void visitCompareInst(CompareInst* node) override {
        visitSimpleNode(node->getId(), node->opcode().GetName());
    }
    void visitConvertInst(ConvertInst* node) override {
        visitSimpleNode(node->getId(), node->opcode().GetName());
    }
    void visitUnaryInst(UnaryInst* node) override {
        visitSimpleNode(node->getId(), node->opcode().GetName());
    }
    void visitLoadInst(LoadInst* node) override {
        visitOffsetNode(node->getId(), node->opcode().GetName(),
                        node->offset());
    }
    void visitStoreInst(StoreInst* node) override {
        visitOffsetNode(node->getId(), node->opcode().GetName(),
                        node->offset());
    }
    void visitBrInst(BrInst* node) override {
        visitLabelNode(node->getId(), Opcode::Br_Opcode.GetName(),
                       node->label());
    }
    void visitBrIfInst(BrIfInst* node) override {
        visitLabelNode(node->getId(), Opcode::BrIf_Opcode.GetName(),
                       node->label());
    }
    void visitGlobalGetInst(GlobalGetInst* node) override {
        visitLabelNode(node->getId(), Opcode::GlobalGet_Opcode.GetName(),
                       node->label());
    }
    void visitGlobalSetInst(GlobalSetInst* node) override {
        visitLabelNode(node->getId(), Opcode::GlobalSet_Opcode.GetName(),
                       node->label());
    }
    void visitLocalGetInst(LocalGetInst* node) override {
        visitLabelNode(node->getId(), Opcode::LocalGet_Opcode.GetName(),
                       node->label());
    }
    void visitLocalSetInst(LocalSetInst* node) override {
        visitLabelNode(node->getId(), Opcode::LocalSet_Opcode.GetName(),
                       node->label());
    }
    void visitLocalTeeInst(LocalTeeInst* node) override {
        visitLabelNode(node->getId(), Opcode::LocalTee_Opcode.GetName(),
                       node->label());
    }
    void visitCallInst(CallInst* node) override {
        visitLabelNode(node->getId(), Opcode::Call_Opcode.GetName(),
                       node->label());
    }
    void visitCallIndirectInst(CallIndirectInst* node) override {
        visitLabelNode(node->getId(), Opcode::CallIndirect_Opcode.GetName(),
                       node->label());
    }
    void visitBeginBlockInst(BeginBlockInst* node) override {
        visitLabelNode(node->getId(), "BeginBlock", node->label());
    }
    void visitBlockInst(BlockInst* node) override {
        visitLabelNode(node->getId(), Opcode::Block_Opcode.GetName(),
                       node->label());
    }
    void visitLoopInst(LoopInst* node) override {
        visitLabelNode(node->getId(), Opcode::Loop_Opcode.GetName(),
                       node->label());
    }
    void visitIfInst(IfInst* node) override {
        visitSimpleNode(node->getId(), Opcode::If_Opcode.GetName());
    }

    // Auxiliaries
    inline void visitSimpleNode(Index nodeId, const std::string& nodeName) {
        visitSimpleNode(nodeId, nodeName.c_str());
    }
    inline void visitSimpleNode(Index nodeId, const char* nodeName) {
        _stream->Writef(
            "%u [label="
            "<<TABLE>"
            "<TR><TD>%s</TD></TR>"
            "</TABLE>>];\n",
            nodeId, nodeName);
    }
    inline void visitOffsetNode(Index nodeId,
                                const char* nodeName,
                                Index offset) {
        _stream->Writef(
            "%u [label="
            "<<TABLE>"
            "<TR><TD>%s offset=%u</TD></TR>"
            "</TABLE>>];\n",
            nodeId, nodeName, offset);
    }
    inline void visitLabelNode(Index nodeId,
                               const char* nodeName,
                               std::string label) {
        _stream->Writef(
            "%u [label="
            "<<TABLE>"
            "<TR><TD>%s %s</TD></TR>"
            "</TABLE>>];\n",
            nodeId, nodeName, label.c_str());
    }
    void setSameRank() {
        setDepth(_graph->getNodes().front(), 0);

        for (Index i = 0; i < _depth.size(); i++) {
            std::string rank = "{rank =" + std::to_string(i) + "; ";
            writePuts(rank.c_str());
            for (auto id : _depth[i]) {
                writeString(std::to_string(id) + "; ");
            }
            writePutsln("}");
        }
    }
    void setDepth(const Node* node, Index depth) {
        if (_depth.size() <= depth) {
            _depth.emplace_back();
        }
        _depth[depth].push_back(node->getId());
        for (auto e : node->outEdges()) {
            if (e->type() == EdgeType::AST) {
                setDepth(e->dest(), depth + 1);
            }
        }
    }
};

}  // namespace wasmati

#endif /* end of WASMATI_DOT_H */
