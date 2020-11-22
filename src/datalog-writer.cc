#include "datalog-writer.h"

namespace wasmati {

void DatalogWriter::writeGraph() {
    NodeSet loopsInsts;
    if (!cpgOptions.loopName.empty()) {
        loopsInsts = Queries::loopsInsts(cpgOptions.loopName);
    }

    writePuts(R"(
node(X) :- module(X, NAME).
node(X) :- function(X, NAME, INDEX, N_ARGS, N_LOCALS, N_RESULTS, IS_IMPORT, IS_EXPORT).
node(X) :- varNode(X, TYPE, INDEX, NAME).
node(X) :- functionSignature(X).
node(X) :- instructions(X).     
node(X) :- parameters(X).     
node(X) :- locals(X).     
node(X) :- results(X).     
node(X) :- else(X).     
node(X) :- trap(X).     
node(X) :- start(X). 
node(X) :- instruction(X, INST_TYPE).
instruction(X, "Nop") :- nop(X).
instruction(X, "Unreachable") :- unreachable(X).
instruction(X, "Return") :- return(X).
instruction(X, "BrTable") :- brTable(X).
instruction(X, "Drop") :- drop(X).
instruction(X, "Select") :- select(X).
instruction(X, "MemorySize") :- memorySize(X).
instruction(X, "MemoryGrow") :- memoryGrow(X).
instruction(X, "Const") :- const(X, CONST_TYPE, VALUE).
instruction(X, "Binary") :- binary(X, OPCODE).
instruction(X, "Compare") :- compare(X, OPCODE).
instruction(X, "Convert") :- convert(X, OPCODE).
instruction(X, "Unary") :- unary(X, OPCODE).
instruction(X, "Load") :- load(X, OFFSET).
instruction(X, "Store") :- store(X, OFFSET).
instruction(X, "Br") :- br(X, LABEL).
instruction(X, "BrIf") :- brIf(X, LABEL).
instruction(X, "GlobalGet") :- globalGet(X, LABEL).
instruction(X, "GlobalSet") :- globalSet(X, LABEL).
instruction(X, "LocalGet") :- localGet(X, LABEL).
instruction(X, "LocalSet") :- localSet(X, LABEL).
instruction(X, "LocalTee") :- localTee(X, LABEL).
instruction(X, "Call") :- call(X, LABEL, N_ARGS, N_RESULTS).
instruction(X, "CallIndirect") :- callIndirect(X, LABEL, N_ARGS, N_RESULTS).
instruction(X, "Block") :- block(X, LABEL, N_RESULTS).
instruction(X, "Loop") :- loop(X, LABEL, N_RESULTS).
instruction(X, "BeginBlock") :- loop(X, LABEL).
instruction(X, "If") :- if(X, N_RESULTS, HAS_ELSE).
edge(X, Y, "AST") :- astEdge(X, Y).
edge(X, Y, "CFG") :- cfgEdge(X, Y).
cfgEdge(X, Y, "CFG") :- cfgEdge(X, Y, LABEL).
edge(X, Y, "PDG") :- pdgEdge(X, Y, LABEL, PDG_TYPE).
edge(X, Y, "CG") :- cgEdge(X, Y).
edge(X, Y, "PG") :- pgEdge(X, Y).
pdgEdge(X, Y, LABEL, PDG_TYPE) :- pdgConstEdge(X, Y, LABEL, PDG_TYPE, VALUE).

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
}

void DatalogWriter::visitASTEdge(ASTEdge* e) {
    if (cpgOptions.printNoAST) {
        return;
    }
    std::stringstream res;
    res << "astEdge(" << e->src()->getId() << ", " << e->dest()->getId()
        << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitCFGEdge(CFGEdge* e) {
    if (cpgOptions.printNoCFG) {
        return;
    }
    if (e->_label.empty()) {
        std::stringstream res;
        res << "cfgEdge(" << e->src()->getId() << ", " << e->dest()->getId()
            << ").";
        writeStringln(res.str());
    } else {
        std::stringstream res;
        res << "cfgEdge(" << e->src()->getId() << ", " << e->dest()->getId()
            << ", " << '"' << e->label() << '"' << ").";
        writeStringln(res.str());
    }
}

void DatalogWriter::visitPDGEdge(PDGEdge* e) {
    if (cpgOptions.printNoPDG) {
        return;
    }
    if (e->pdgType() == PDGType::Const) {
        std::stringstream res;
        res << "pdgConstEdge(" << e->src()->getId() << ", "
            << e->dest()->getId() << ", " << '"' << e->label() << '"' << ", "
            << '"' << pdgTypeMap[e->pdgType()] << '"' << ", "
            << ConstInst::writeConst(e->value(), false) << ").";
        writeStringln(res.str());
    } else {
        std::stringstream res;
        res << "pdgEdge(" << e->src()->getId() << ", " << e->dest()->getId()
            << ", " << '"' << e->label() << '"' << ", " << '"'
            << pdgTypeMap[e->pdgType()] << '"' << ").";
        writeStringln(res.str());
    }
}

void DatalogWriter::visitCGEdge(CGEdge* e) {
    if (cpgOptions.printNoCG) {
        return;
    }
    std::stringstream res;
    res << "cgEdge(" << e->src()->getId() << ", " << e->dest()->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitPGEdge(PGEdge* e) {
    if (cpgOptions.printNoPG) {
        return;
    }
    std::stringstream res;
    res << "pgEdge(" << e->src()->getId() << ", " << e->dest()->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitModule(Module* mod) {
    std::stringstream res;
    res << "module(" << mod->getId() << ", " << '"' << mod->name() << '"'
        << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitFunction(Function* func) {
    std::stringstream res;
    res << "function(" << func->getId() << ", " << '"' << func->name() << '"'
        << ", " << func->index() << "," << func->nargs() << ", "
        << func->nlocals() << ", " << func->nresults() << ", "
        << func->isImport() << ", " << func->isExport() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitFunctionSignature(FunctionSignature* node) {
    std::stringstream res;
    res << "functionSignature(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitParameters(Parameters* node) {
    std::stringstream res;
    res << "parameters(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitInstructions(Instructions* node) {
    std::stringstream res;
    res << "instructions(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitLocals(Locals* node) {
    std::stringstream res;
    res << "locals(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitResults(Results* node) {
    std::stringstream res;
    res << "results(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitElse(Else* node) {
    std::stringstream res;
    res << "else(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitStart(Start* node) {
    std::stringstream res;
    res << "start(" << node->getId() << ").";
    writeStringln(res.str());
}
void DatalogWriter::visitTrap(Trap* node) {
    std::stringstream res;
    res << "trap(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitVarNode(VarNode* node) {
    std::stringstream res;
    res << "varNode(" << node->getId() << ", " << '"'
        << typeToString(node->varType()) << '"' << ", " << node->index() << ", "
        << '"' << node->name() << '"' << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitNopInst(NopInst* node) {
    std::stringstream res;
    res << "nop(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitUnreachableInst(UnreachableInst* node) {
    std::stringstream res;
    res << "unreachable(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitReturnInst(ReturnInst* node) {
    std::stringstream res;
    res << "return(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitBrTableInst(BrTableInst* node) {
    std::stringstream res;
    res << "brTable(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitDropInst(DropInst* node) {
    std::stringstream res;
    res << "drop(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitSelectInst(SelectInst* node) {
    std::stringstream res;
    res << "select(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitMemorySizeInst(MemorySizeInst* node) {
    std::stringstream res;
    res << "memorySize(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitMemoryGrowInst(MemoryGrowInst* node) {
    std::stringstream res;
    res << "memoryGrow(" << node->getId() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitConstInst(ConstInst* node) {
    std::stringstream res;
    res << "const(" << node->getId() << ", " << '"'
        << typeToString(node->value().type) << '"' << ", "
        << ConstInst::writeConst(node->value(), false) << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitBinaryInst(BinaryInst* node) {
    std::stringstream res;
    res << "binary(" << node->getId() << ", " << '"' << node->opcode().GetName()
        << '"' << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitCompareInst(CompareInst* node) {
    std::stringstream res;
    res << "compare(" << node->getId() << ", " << '"'
        << node->opcode().GetName() << '"' << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitConvertInst(ConvertInst* node) {
    std::stringstream res;
    res << "convert(" << node->getId() << ", " << '"'
        << node->opcode().GetName() << '"' << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitUnaryInst(UnaryInst* node) {
    std::stringstream res;
    res << "unary(" << node->getId() << ", " << '"' << node->opcode().GetName()
        << '"' << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitLoadInst(LoadInst* node) {
    std::stringstream res;
    res << "load(" << node->getId() << ", " << node->offset() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitStoreInst(StoreInst* node) {
    std::stringstream res;
    res << "store(" << node->getId() << ", " << node->offset() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitBrInst(BrInst* node) {
    std::stringstream res;
    res << "br(" << node->getId() << ", " << '"' << node->label() << '"'
        << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitBrIfInst(BrIfInst* node) {
    std::stringstream res;
    res << "brIf(" << node->getId() << ", " << '"' << node->label() << '"'
        << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitGlobalGetInst(GlobalGetInst* node) {
    std::stringstream res;
    res << "globalGet(" << node->getId() << ", " << '"' << node->label() << '"'
        << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitGlobalSetInst(GlobalSetInst* node) {
    std::stringstream res;
    res << "globalSet(" << node->getId() << ", " << '"' << node->label() << '"'
        << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitLocalGetInst(LocalGetInst* node) {
    std::stringstream res;
    res << "localGet(" << node->getId() << ", " << '"' << node->label() << '"'
        << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitLocalSetInst(LocalSetInst* node) {
    std::stringstream res;
    res << "localSet(" << node->getId() << ", " << '"' << node->label() << '"'
        << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitLocalTeeInst(LocalTeeInst* node) {
    std::stringstream res;
    res << "localTee(" << node->getId() << ", " << '"' << node->label() << '"'
        << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitCallInst(CallInst* node) {
    std::stringstream res;
    res << "call(" << node->getId() << ", " << '"' << node->label() << '"'
        << ", " << node->nargs() << ", " << node->nresults() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitCallIndirectInst(CallIndirectInst* node) {
    std::stringstream res;
    res << "callIndirect(" << node->getId() << ", " << '"' << node->label()
        << '"' << ", " << node->nargs() << ", " << node->nresults() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitBlockInst(BlockInst* node) {
    std::stringstream res;
    res << "block(" << node->getId() << ", " << '"' << node->label() << '"'
        << ", " << node->nresults() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitLoopInst(LoopInst* node) {
    std::stringstream res;
    res << "loop(" << node->getId() << ", " << '"' << node->label() << '"'
        << ", " << node->nresults() << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitBeginBlockInst(BeginBlockInst* node) {
    std::stringstream res;
    res << "beginBlock(" << node->getId() << ", " << '"' << node->label() << '"'
        << ").";
    writeStringln(res.str());
}

void DatalogWriter::visitIfInst(IfInst* node) {
    std::stringstream res;
    res << "if(" << node->getId() << ", " << node->nresults() << ", "
        << node->hasElse() << ").";
    writeStringln(res.str());
}

std::string DatalogWriter::typeToString(Type type) {
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
