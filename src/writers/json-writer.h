#ifndef WASMATI_JSON_H
#define WASMATI_JSON_H
#include <map>
#include "src/graph.h"
#include "src/query.h"

namespace wasmati {

class JSONWriter : public GraphWriter {
    json _graphJson;
    Index _edgeId = 0;

public:
    JSONWriter(wabt::Stream* stream, Graph* graph)
        : GraphWriter(stream, graph) {}

    void writeGraph() override {
        NodeSet loopsInsts;
        if (!cpgOptions.loopName.empty()) {
            loopsInsts = Queries::loopsInsts(cpgOptions.loopName);
        }
        auto nodes = _graph->getNodes();
        std::sort(nodes.begin(), nodes.end(), Compare());
        _graphJson["nodes"] = json::array();
        _graphJson["edges"] = json::array();

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
        for (auto const& node : _graph->getNodes()) {
            node->acceptEdges(this);
        }
        writePuts(_graphJson.dump(2).c_str());
    }

    // Edges
    void visitASTEdge(ASTEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printAST)) {
            return;
        }
        json edgeJson;
        edgeJson["src"] = e->src()->getId();
        edgeJson["dest"] = e->dest()->getId();
        edgeJson["type"] = "AST";
        _graphJson["edges"].emplace_back(edgeJson);
        insertEdge(e->src()->getId(), e->dest()->getId(), _edgeId++);
    }
    void visitCFGEdge(CFGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printCFG)) {
            return;
        }

        json edgeJson;
        edgeJson["src"] = e->src()->getId();
        edgeJson["dest"] = e->dest()->getId();
        edgeJson["type"] = "CFG";
        edgeJson["label"] = e->label();
        _graphJson["edges"].emplace_back(edgeJson);
        insertEdge(e->src()->getId(), e->dest()->getId(), _edgeId++);
    }

    void visitPDGEdge(PDGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printPDG)) {
            return;
        }
        if (e->pdgType() == PDGType::Const) {
            json edgeJson;
            edgeJson["src"] = e->src()->getId();
            edgeJson["dest"] = e->dest()->getId();
            edgeJson["type"] = "PDG";
            edgeJson["label"] = e->label();
            edgeJson["pdgType"] = e->writePdgType();
            edgeJson["value"] = Utils::jsonConst(e->value());
            _graphJson["edges"].emplace_back(edgeJson);
            insertEdge(e->src()->getId(), e->dest()->getId(), _edgeId++);
        } else {
            json edgeJson;
            edgeJson["src"] = e->src()->getId();
            edgeJson["dest"] = e->dest()->getId();
            edgeJson["type"] = "PDG";
            edgeJson["label"] = e->label();
            edgeJson["pdgType"] = e->writePdgType();
            _graphJson["edges"].emplace_back(edgeJson);
            insertEdge(e->src()->getId(), e->dest()->getId(), _edgeId++);
        }
    }
    void visitCGEdge(CGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printCG)) {
            return;
        }
        json edgeJson;
        edgeJson["src"] = e->src()->getId();
        edgeJson["dest"] = e->dest()->getId();
        edgeJson["type"] = "CG";
        _graphJson["edges"].emplace_back(edgeJson);
        insertEdge(e->src()->getId(), e->dest()->getId(), _edgeId++);
    }

private:
    // Inherited via GraphWriter
    void visitModule(Module* node) override {
        json nodeJson;
        nodeJson["inEdges"] = json::array();
        nodeJson["outEdges"] = json::array();
        nodeJson["id"] = node->getId();
        nodeJson["type"] = "Module";
        nodeJson["name"] = node->name();
        _graphJson["nodes"].emplace_back(nodeJson);
    }

    void visitFunction(Function* node) override {
        json nodeJson;
        nodeJson["inEdges"] = json::array();
        nodeJson["outEdges"] = json::array();
        nodeJson["id"] = node->getId();
        nodeJson["type"] = "Function";
        nodeJson["name"] = node->name();
        nodeJson["index"] = node->index();
        nodeJson["nargs"] = node->nargs();
        nodeJson["nlocals"] = node->nlocals();
        nodeJson["nresults"] = node->nresults();
        nodeJson["isImport"] = node->isImport();
        nodeJson["isExport"] = node->isExport();
        _graphJson["nodes"].emplace_back(nodeJson);
    }

    void visitFunctionSignature(FunctionSignature* node) override {
        visitSimpleNode(node, node->getNodeName());
    }
    void visitParameters(Parameters* node) override {
        visitSimpleNode(node, node->getNodeName());
    }
    void visitInstructions(Instructions* node) override {
        visitSimpleNode(node, node->getNodeName());
    }
    void visitLocals(Locals* node) override {
        visitSimpleNode(node, node->getNodeName());
    }
    void visitResults(Results* node) override {
        visitSimpleNode(node, node->getNodeName());
    }
    void visitElse(Else* node) override {
        visitSimpleNode(node, node->getNodeName());
    }
    void visitStart(Start* node) override {
        visitSimpleNode(node, node->getNodeName());
    }
    void visitTrap(Trap* node) override {
        visitSimpleNode(node, node->getNodeName());
    }
    void visitVarNode(VarNode* node) override {
        json nodeJson;
        nodeJson["inEdges"] = json::array();
        nodeJson["outEdges"] = json::array();
        nodeJson["id"] = node->getId();
        nodeJson["type"] = "VarNode";
        nodeJson["name"] = node->name();
        nodeJson["index"] = node->index();
        nodeJson["varType"] = node->writeVarType();
        _graphJson["nodes"].emplace_back(nodeJson);
    }
    void visitNopInst(NopInst* node) override {
        visitSimpleInstNode(node, "Nop");
    }
    void visitUnreachableInst(UnreachableInst* node) override {
        visitSimpleInstNode(node, "Unreachable");
    }
    void visitReturnInst(ReturnInst* node) override {
        visitSimpleInstNode(node, "Return");
    }
    void visitBrTableInst(BrTableInst* node) override {
        visitSimpleInstNode(node, "BrTable");
    }
    void visitDropInst(DropInst* node) override {
        visitSimpleInstNode(node, "Drop");
    }
    void visitSelectInst(SelectInst* node) override {
        visitSimpleInstNode(node, "Select");
    }
    void visitMemorySizeInst(MemorySizeInst* node) override {
        visitSimpleInstNode(node, "MemorySize");
    }
    void visitMemoryGrowInst(MemoryGrowInst* node) override {
        visitSimpleInstNode(node, "MemoryGrow");
    }
    void visitConstInst(ConstInst* node) override {
        json nodeJson;
        nodeJson["inEdges"] = json::array();
        nodeJson["outEdges"] = json::array();
        nodeJson["id"] = node->getId();
        nodeJson["type"] = "Instruction";
        nodeJson["instType"] = "Const";
        nodeJson["value"] = Utils::jsonConst(node->value());
        _graphJson["nodes"].emplace_back(nodeJson);
    }
    void visitBinaryInst(BinaryInst* node) override {
        visitOpcodeInstNode(node, "Binary");
    }
    void visitCompareInst(CompareInst* node) override {
        visitOpcodeInstNode(node, "Compare");
    }
    void visitConvertInst(ConvertInst* node) override {
        visitOpcodeInstNode(node, "Convert");
    }
    void visitUnaryInst(UnaryInst* node) override {
        visitOpcodeInstNode(node, "Unary");
    }
    void visitLoadInst(LoadInst* node) override {
        visitLoadStoreInstNode(node, "Load");
    }
    void visitStoreInst(StoreInst* node) override {
        visitLoadStoreInstNode(node, "Store");
    }
    void visitBrInst(BrInst* node) override { visitLabelInstNode(node, "Br"); }
    void visitBrIfInst(BrIfInst* node) override {
        visitLabelInstNode(node, "BrIF");
    }
    void visitGlobalGetInst(GlobalGetInst* node) override {
        visitLabelInstNode(node, "GlobalGet");
    }
    void visitGlobalSetInst(GlobalSetInst* node) override {
        visitLabelInstNode(node, "GlobalSet");
    }
    void visitLocalGetInst(LocalGetInst* node) override {
        visitLabelInstNode(node, "LocalGet");
    }
    void visitLocalSetInst(LocalSetInst* node) override {
        visitLabelInstNode(node, "LocalSet");
    }
    void visitLocalTeeInst(LocalTeeInst* node) override {
        visitLabelInstNode(node, "LocalTee");
    }
    void visitCallInst(CallInst* node) override {
        visitCallInstNode(node, "Cal");
    }
    void visitCallIndirectInst(CallIndirectInst* node) override {
        visitCallInstNode(node, "CallIndirect");
    }
    void visitBeginBlockInst(BeginBlockInst* node) override {
        visitLabelInstNode(node, "BeginBlock");
    }
    void visitBlockInst(BlockInst* node) override {
        visitBlockInstNode(node, "Block");
    }
    void visitLoopInst(LoopInst* node) override {
        visitBlockInstNode(node, "Loop");
    }
    void visitEndLoopInst(EndLoopInst* node) override {
        visitBlockInstNode(node, "EndLoop");
    }
    void visitIfInst(IfInst* node) override {
        json nodeJson;
        nodeJson["inEdges"] = json::array();
        nodeJson["outEdges"] = json::array();
        nodeJson["id"] = node->getId();
        nodeJson["type"] = "Instruction";
        nodeJson["nresults"] = node->nresults();
        nodeJson["instType"] = "If";
        nodeJson["hasElse"] = node->hasElse();
        _graphJson["nodes"].emplace_back(nodeJson);
    }

private:
    inline void visitSimpleNode(Node* node, const std::string nodeName) {
        json nodeJson;
        nodeJson["inEdges"] = json::array();
        nodeJson["outEdges"] = json::array();
        nodeJson["id"] = node->getId();
        nodeJson["type"] = nodeName;
        _graphJson["nodes"].emplace_back(nodeJson);
    }

    inline void visitSimpleInstNode(Node* node, const std::string instType) {
        json nodeJson;
        nodeJson["inEdges"] = json::array();
        nodeJson["outEdges"] = json::array();
        nodeJson["id"] = node->getId();
        nodeJson["type"] = "Instruction";
        nodeJson["instType"] = instType;
        _graphJson["nodes"].emplace_back(nodeJson);
    }

    inline void visitOpcodeInstNode(Node* node, const std::string instType) {
        json nodeJson;
        nodeJson["inEdges"] = json::array();
        nodeJson["outEdges"] = json::array();
        nodeJson["id"] = node->getId();
        nodeJson["type"] = "Instruction";
        nodeJson["instType"] = instType;
        nodeJson["opcode"] = node->opcode().GetName();
        _graphJson["nodes"].emplace_back(nodeJson);
    }

    inline void visitLoadStoreInstNode(Node* node, const std::string instType) {
        json nodeJson;
        nodeJson["inEdges"] = json::array();
        nodeJson["outEdges"] = json::array();
        nodeJson["id"] = node->getId();
        nodeJson["type"] = "Instruction";
        nodeJson["instType"] = instType;
        nodeJson["opcode"] = node->opcode().GetName();
        nodeJson["offset"] = node->offset();
        _graphJson["nodes"].emplace_back(nodeJson);
    }
    inline void visitLabelInstNode(Node* node, const std::string instType) {
        json nodeJson;
        nodeJson["inEdges"] = json::array();
        nodeJson["outEdges"] = json::array();
        nodeJson["id"] = node->getId();
        nodeJson["type"] = "Instruction";
        nodeJson["instType"] = instType;
        nodeJson["label"] = node->label();
        _graphJson["nodes"].emplace_back(nodeJson);
    }
    inline void visitCallInstNode(Node* node, const std::string instType) {
        json nodeJson;
        nodeJson["inEdges"] = json::array();
        nodeJson["outEdges"] = json::array();
        nodeJson["id"] = node->getId();
        nodeJson["type"] = "Instruction";
        nodeJson["nargs"] = node->nargs();
        nodeJson["nresults"] = node->nresults();
        nodeJson["instType"] = instType;
        nodeJson["label"] = node->label();
        _graphJson["nodes"].emplace_back(nodeJson);
    }

    inline void visitBlockInstNode(Node* node, const std::string instType) {
        json nodeJson;
        nodeJson["inEdges"] = json::array();
        nodeJson["outEdges"] = json::array();
        nodeJson["id"] = node->getId();
        nodeJson["type"] = "Instruction";
        nodeJson["nresults"] = node->nresults();
        nodeJson["instType"] = instType;
        nodeJson["label"] = node->label();
        _graphJson["nodes"].emplace_back(nodeJson);
    }

    inline void insertEdge(Index src, Index dest, Index edgeId) {
        _graphJson["nodes"][src]["outEdges"].emplace_back(edgeId);
        _graphJson["nodes"][dest]["inEdges"].emplace_back(edgeId);
    }
};

}  // namespace wasmati

#endif /* end of WASMATI_DOT_H */
