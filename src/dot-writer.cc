#include "dot-writer.h"

namespace wasmati {

void DotWriter::writeGraph() {
    writePuts("digraph G {");
    writePuts("graph [rankdir=TD];");
    writePuts("node [shape=none];");
    bool justOneGraph =
        _options.printNoAST || _options.printNoCFG || _options.printNoPDG;

    for (auto const& node : _graph->getNodes()) {
        node->acceptEdges(this);

        if (!justOneGraph) {
            node->accept(this);
        } else if (!_options.printNoAST &&
                   node->hasEdgesOf(EdgeType::AST)) {  // AST
            node->accept(this);
        } else if (!_options.printNoCFG &&
                   node->hasEdgesOf(EdgeType::CFG)) {  // CFG
            node->accept(this);
        } else if (!_options.printNoPDG &&
                   node->hasEdgesOf(EdgeType::PDG)) {  // PDG
            node->accept(this);
        }
    }

    if (!justOneGraph) {
        setSameRank();
    }

    writePuts("}");
}

void DotWriter::visitASTEdge(ASTEdge* e) {
    if (_options.printNoAST) {
        return;
    }
    writeStringln(std::to_string(e->src()->getId()) + " -> " +
                  std::to_string(e->dest()->getId()) + " [color=forestgreen]");
}

void DotWriter::visitCFGEdge(CFGEdge* e) {
    if (_options.printNoCFG) {
        return;
    }
    if (e->_label.empty()) {
        writeStringln(std::to_string(e->src()->getId()) + " -> " +
                      std::to_string(e->dest()->getId()) + " [color=red]");
    } else {
        writeStringln(std::to_string(e->src()->getId()) + " -> " +
                      std::to_string(e->dest()->getId()) +
                      " [fontcolor=red label=\"" + e->_label + "\" color=red]");
    }
}

void DotWriter::visitPDGEdge(PDGEdge* e) {
    if (_options.printNoPDG) {
        return;
    }
    if (e->_label.empty()) {
        writeStringln(std::to_string(e->src()->getId()) + " -> " +
                      std::to_string(e->dest()->getId()) + " [color=blue]");
    } else {
        writeStringln(std::to_string(e->src()->getId()) + " -> " +
                      std::to_string(e->dest()->getId()) +
                      " [fontcolor=blue label=\"" + e->_label +
                      "\" color=blue]");
    }
}

void DotWriter::visitModule(Module* mod) {
    std::string s;
    if (mod->name().empty()) {
        s += std::to_string(mod->getId()) +
             " [label=<<TABLE><TR><TD>Module</TD></TR></TABLE>>];";
    } else {
        s += std::to_string(mod->getId()) +
             " [label=<<TABLE><TR><TD>module</TD></TR>";
        s += "<TR><TD>";
        s += "name = " + mod->name() + "</TD></TR></TABLE>>];";
    }
    writeStringln(s);
}

void DotWriter::visitFunction(Function* func) {
    std::string s = std::to_string(func->getId()) +
                    " [label=<<TABLE><TR><TD>Function</TD></TR>";
    s += "<TR><TD>";
    s += "name = " + func->name() + "</TD></TR></TABLE>> ];";
    writeStringln(s);
}

void DotWriter::visitSimpleNode(int nodeId, const std::string& nodeName) {
    writeStringln(std::to_string(nodeId) + " [label=<<TABLE><TR><TD>" +
                  nodeName + "</TD></TR></TABLE>>];");
}

void DotWriter::visitFunctionSignature(FunctionSignature* node) {
    visitSimpleNode(node->getId(), node->getNodeName());
}

void DotWriter::visitParameters(Parameters* node) {
    visitSimpleNode(node->getId(), node->getNodeName());
}

void DotWriter::visitInstructions(Instructions* node) {
    visitSimpleNode(node->getId(), node->getNodeName());
}

void DotWriter::visitLocals(Locals* node) {
    visitSimpleNode(node->getId(), node->getNodeName());
}

void DotWriter::visitResults(Results* node) {
    visitSimpleNode(node->getId(), node->getNodeName());
}

void DotWriter::visitElse(Else* node) {
    visitSimpleNode(node->getId(), node->getNodeName());
}

void DotWriter::visitStart(Start* node) {
    visitSimpleNode(node->getId(), node->getNodeName());
}
void DotWriter::visitTrap(Trap* node) {
    visitSimpleNode(node->getId(), node->getNodeName());
}

void DotWriter::visitVarNode(VarNode* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE>");
    writeString("<TR><TD>type</TD><TD>" + typeToString(node->varType()) +
                "</TD></TR>");
    if (!node->name().empty()) {
        writeString("<TR><TD>name</TD><TD>" + node->name() + "</TD></TR>");
    }
    writeStringln("</TABLE>>];");
}

void DotWriter::visitNopInst(NopInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::Nop_Opcode.GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitUnreachableInst(UnreachableInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::Unreachable_Opcode.GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitReturnInst(ReturnInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::Return_Opcode.GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitBrTableInst(BrTableInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::BrTable_Opcode.GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitCallIndirectInst(CallIndirectInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::CallIndirect_Opcode.GetName());
    writeString(" " + node->label());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitDropInst(DropInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::Drop_Opcode.GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitSelectInst(SelectInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::Select_Opcode.GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitMemorySizeInst(MemorySizeInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::MemorySize_Opcode.GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitMemoryGrowInst(MemoryGrowInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::MemoryGrow_Opcode.GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitConstInst(ConstInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(ConstInst::writeConst(node->value()));
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitBinaryInst(BinaryInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(node->opcode().GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitCompareInst(CompareInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(node->opcode().GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitConvertInst(ConvertInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(node->opcode().GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitUnaryInst(UnaryInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(node->opcode().GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitLoadInst(LoadInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(node->opcode().GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitStoreInst(StoreInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(node->opcode().GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitBrInst(BrInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::Br_Opcode.GetName());
    writeString(" " + node->label());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitBrIfInst(BrIfInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::BrIf_Opcode.GetName());
    writeString(" " + node->label());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitCallInst(CallInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::Call_Opcode.GetName());
    writeString(" " + node->label());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitGlobalGetInst(GlobalGetInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::GlobalGet_Opcode.GetName());
    writeString(" " + node->label());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitGlobalSetInst(GlobalSetInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::GlobalSet_Opcode.GetName());
    writeString(" " + node->label());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitLocalGetInst(LocalGetInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::LocalGet_Opcode.GetName());
    writeString(" " + node->label());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitLocalSetInst(LocalSetInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::LocalSet_Opcode.GetName());
    writeString(" " + node->label());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitLocalTeeInst(LocalTeeInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::LocalTee_Opcode.GetName());
    writeString(" " + node->label());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitBeginBlockInst(BeginBlockInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString("BeginBlock");
    writeString(" " + node->label());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitBlockInst(BlockInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::Block_Opcode.GetName());
    writeString(" " + node->label());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitLoopInst(LoopInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::Loop_Opcode.GetName());
    writeString(" " + node->label());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::visitIfInst(IfInst* node) {
    writeString(std::to_string(node->getId()) + " [label=<<TABLE><TR><TD>");
    writeString(Opcode::If_Opcode.GetName());
    writeStringln("</TD></TR></TABLE>>];");
}

void DotWriter::setSameRank() {
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

void DotWriter::setDepth(const Node* node, Index depth) {
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

std::string DotWriter::typeToString(Type type) {
    switch (type) {
    case wabt::Type::I32:
        return "i32";
    case wabt::Type::I64:
        return "i64";
    case wabt::Type::F32:
        return "f32";
    case wabt::Type::F64:
        return "f64";
    default:
        return "unknown";
    }
}

}  // namespace wasmati
