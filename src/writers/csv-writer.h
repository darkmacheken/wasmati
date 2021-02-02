#ifndef WASMATI_CSV_H
#define WASMATI_CSV_H
#include <cstdio>
#include <map>
#include "config.h"
#include "src/graph.h"
#include "src/query.h"
#include "zip.h"

namespace wasmati {

class CSVWriter : public GraphWriter {
    bool _datalog;
    std::string _NODES_FILE_NAME;
    std::string _EDGES_FILE_NAME;

private:
    std::string _edgesFileName;
    std::string _nodesFileName;
    std::string _infoFileName;
    std::shared_ptr<wabt::Stream> _edges;
    std::shared_ptr<wabt::Stream> _nodes;
    std::shared_ptr<wabt::Stream> _info;
    zip_t* _zipArchive;

private:
    size_t _numNodes = 0;
    size_t _numEdges = 0;

    size_t _astOrder = 0;

private:
public:
    CSVWriter(std::string zipFileName, Graph* graph, bool datalog = false)
        : GraphWriter(nullptr, graph), _datalog(datalog) {
        auto path = Path(zipFileName);
        auto dir = path.directory();
        _edgesFileName = std::tmpnam(nullptr);
        _nodesFileName = std::tmpnam(nullptr);
        _infoFileName = std::tmpnam(nullptr);
        _edges = std::make_shared<wabt::FileStream>(_edgesFileName);
        _nodes = std::make_shared<wabt::FileStream>(_nodesFileName);
        _info = std::make_shared<wabt::FileStream>(_infoFileName);
        int error;
        _zipArchive =
            zip_open(zipFileName.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
        if (datalog) {
            _NODES_FILE_NAME = "node.facts";
            _EDGES_FILE_NAME = "edge.facts";
        } else {
            _NODES_FILE_NAME = "nodes.csv";
            _EDGES_FILE_NAME = "edges.csv";
        }
    }

    void writeGraph() override {
        NodeSet loopsInsts;
        if (!cpgOptions.loopName.empty()) {
            loopsInsts = Queries::loopsInsts(cpgOptions.loopName);
        }
        auto nodes = _graph->getNodes();
        std::sort(nodes.begin(), nodes.end(), CompareNode());

        for (auto const& node : nodes) {
            if (!loopsInsts.empty() && loopsInsts.count(node) == 0) {
                continue;
            }
            _astOrder = 0;
            for (auto edge : node->outEdges()) {
                edge->accept(this);
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
        printInfo();
        _edges->Flush();
        _nodes->Flush();
        _info->Flush();

        zip_source_t* nodesSource;
        zip_source_t* edgesSource;
        zip_source_t* infoSource;
        if ((nodesSource = zip_source_file(_zipArchive, _nodesFileName.c_str(),
                                           0, -1)) == NULL ||
            zip_file_add(_zipArchive, _NODES_FILE_NAME.c_str(), nodesSource,
                         ZIP_FL_ENC_UTF_8 | ZIP_FL_OVERWRITE) < 0) {
            zip_source_free(nodesSource);
            printf("error adding file: %s\n", zip_strerror(_zipArchive));
        }
        if ((edgesSource = zip_source_file(_zipArchive, _edgesFileName.c_str(),
                                           0, -1)) == NULL ||
            zip_file_add(_zipArchive, _EDGES_FILE_NAME.c_str(), edgesSource,
                         ZIP_FL_ENC_UTF_8 | ZIP_FL_OVERWRITE) < 0) {
            zip_source_free(edgesSource);
            printf("error adding file: %s\n", zip_strerror(_zipArchive));
        }
        if ((infoSource = zip_source_file(_zipArchive, _infoFileName.c_str(), 0,
                                          -1)) == NULL ||
            zip_file_add(_zipArchive, "info.json", infoSource,
                         ZIP_FL_ENC_UTF_8 | ZIP_FL_OVERWRITE) < 0) {
            zip_source_free(infoSource);
            printf("error adding file: %s\n", zip_strerror(_zipArchive));
        }
        zip_close(_zipArchive);
        std::remove(_edgesFileName.c_str());
        std::remove(_nodesFileName.c_str());
        std::remove(_infoFileName.c_str());
    }

    // Edges
    void visitASTEdge(ASTEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printAST)) {
            return;
        }
        auto edges = getEdgesMap();
        edges[SRC] = std::to_string(e->src()->id());
        edges[DEST] = std::to_string(e->dest()->id());
        edges[EDGE_TYPE] = EDGE_TYPES_MAP.at(EdgeType::AST);
        edges[AST_ORDER] = std::to_string(_astOrder);
        writeEdge(edges);
        _astOrder++;
    }
    void visitCFGEdge(CFGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printCFG)) {
            return;
        }
        auto edges = getEdgesMap();
        edges[SRC] = std::to_string(e->src()->id());
        edges[DEST] = std::to_string(e->dest()->id());
        edges[EDGE_TYPE] = EDGE_TYPES_MAP.at(EdgeType::CFG);
        edges[LABEL] = e->label();
        writeEdge(edges);
    }
    void visitPDGEdge(PDGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printPDG)) {
            return;
        }
        auto edges = getEdgesMap();
        edges[SRC] = std::to_string(e->src()->id());
        edges[DEST] = std::to_string(e->dest()->id());
        edges[EDGE_TYPE] = EDGE_TYPES_MAP.at(EdgeType::PDG);
        edges[LABEL] = e->label();
        edges[PDG_TYPE] = PDG_TYPE_MAP.at(e->pdgType());
        if (e->pdgType() == PDGType::Const) {
            edges[CONST_TYPE] = Utils::writeConstType(e->value());
            if (e->value().type == Type::I32 || e->value().type == Type::I64) {
                edges[CONST_VALUE_I] = Utils::writeConst(e->value(), false);
            } else {
                edges[CONST_VALUE_F] = Utils::writeConst(e->value(), false);
            }
        }
        writeEdge(edges);
    }
    void visitCGEdge(CGEdge* e) override {
        if (!(cpgOptions.printAll || cpgOptions.printCG)) {
            return;
        }
        auto edges = getEdgesMap();
        edges[SRC] = std::to_string(e->src()->id());
        edges[DEST] = std::to_string(e->dest()->id());
        edges[EDGE_TYPE] = EDGE_TYPES_MAP.at(EdgeType::CG);
        writeEdge(edges);
    }

private:
    // Inherited via GraphWriter
    void visitModule(Module* node) override {
        auto nodes = getNodesMap();
        nodes[ID] = std::to_string(node->id());
        nodes[NODE_TYPE] = NODE_TYPE_MAP.at(node->type());
        nodes[NAME] = node->name();
        writeNode(nodes);
    }
    void visitFunction(Function* node) override {
        auto nodes = getNodesMap();
        nodes[ID] = std::to_string(node->id());
        nodes[NODE_TYPE] = NODE_TYPE_MAP.at(node->type());
        nodes[NAME] = node->name();
        nodes[INDEX] = std::to_string(node->index());
        nodes[NARGS] = std::to_string(node->nargs());
        nodes[NLOCALS] = std::to_string(node->nlocals());
        nodes[NRESULTS] = std::to_string(node->nresults());
        nodes[IS_IMPORT] = std::to_string(node->isImport());
        nodes[IS_EXPORT] = std::to_string(node->isExport());
        writeNode(nodes);
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
        auto nodes = getNodesMap();
        nodes[ID] = std::to_string(node->id());
        nodes[NODE_TYPE] = NODE_TYPE_MAP.at(node->type());
        nodes[NAME] = node->name();
        nodes[INDEX] = std::to_string(node->index());
        nodes[VAR_TYPE] = node->writeVarType();
        writeNode(nodes);
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
        auto nodes = getNodesMap();
        nodes[ID] = std::to_string(node->id());
        nodes[NODE_TYPE] = NODE_TYPE_MAP.at(node->type());
        nodes[INST_TYPE] = INST_TYPE_MAP.at(node->instType());
        nodes[CONST_TYPE] = Utils::writeConstType(node->value());
        if (node->value().type == Type::I32 ||
            node->value().type == Type::I64) {
            nodes[CONST_VALUE_I] = Utils::writeConst(node->value(), false);
        } else {
            nodes[CONST_VALUE_F] = Utils::writeConst(node->value(), false);
        }
        writeNode(nodes);
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
        visitBlockInstNode(node);
    }
    void visitBlockInst(BlockInst* node) override { visitBlockInstNode(node); }
    void visitLoopInst(LoopInst* node) override { visitBlockInstNode(node); }
    void visitEndLoopInst(EndLoopInst* node) override {
        visitBlockInstNode(node);
    }
    void visitIfInst(IfInst* node) override {
        auto nodes = getNodesMap();
        nodes[ID] = std::to_string(node->id());
        nodes[NODE_TYPE] = NODE_TYPE_MAP.at(node->type());
        nodes[NRESULTS] = std::to_string(node->nresults());
        nodes[INST_TYPE] = INST_TYPE_MAP.at(node->instType());
        nodes[HAS_ELSE] = std::to_string(node->hasElse());
        writeNode(nodes);
    }

private:
    inline void visitSimpleNode(Node* node) {
        auto nodes = getNodesMap();
        nodes[ID] = std::to_string(node->id());
        nodes[NODE_TYPE] = NODE_TYPE_MAP.at(node->type());
        writeNode(nodes);
    }

    inline void visitSimpleInstNode(Node* node) {
        auto nodes = getNodesMap();
        nodes[ID] = std::to_string(node->id());
        nodes[NODE_TYPE] = NODE_TYPE_MAP.at(node->type());
        nodes[INST_TYPE] = INST_TYPE_MAP.at(node->instType());
        writeNode(nodes);
    }

    inline void visitOpcodeInstNode(Node* node) {
        auto nodes = getNodesMap();
        nodes[ID] = std::to_string(node->id());
        nodes[NODE_TYPE] = NODE_TYPE_MAP.at(node->type());
        nodes[INST_TYPE] = INST_TYPE_MAP.at(node->instType());
        nodes[OPCODE] = node->opcode().GetName();
        writeNode(nodes);
    }

    inline void visitLoadStoreInstNode(Node* node) {
        auto nodes = getNodesMap();
        nodes[ID] = std::to_string(node->id());
        nodes[NODE_TYPE] = NODE_TYPE_MAP.at(node->type());
        nodes[INST_TYPE] = INST_TYPE_MAP.at(node->instType());
        nodes[OPCODE] = node->opcode().GetName();
        nodes[OFFSET] = std::to_string(node->offset());
        writeNode(nodes);
    }
    inline void visitLabelInstNode(Node* node) {
        auto nodes = getNodesMap();
        nodes[ID] = std::to_string(node->id());
        nodes[NODE_TYPE] = NODE_TYPE_MAP.at(node->type());
        nodes[INST_TYPE] = INST_TYPE_MAP.at(node->instType());
        nodes[LABEL] = node->label();
        writeNode(nodes);
    }
    inline void visitCallInstNode(Node* node) {
        auto nodes = getNodesMap();
        nodes[ID] = std::to_string(node->id());
        nodes[NODE_TYPE] = NODE_TYPE_MAP.at(node->type());
        nodes[NARGS] = std::to_string(node->nargs());
        nodes[NRESULTS] = std::to_string(node->nresults());
        nodes[INST_TYPE] = INST_TYPE_MAP.at(node->instType());
        nodes[LABEL] = node->label();
        writeNode(nodes);
    }

    inline void visitBlockInstNode(Node* node) {
        auto nodes = getNodesMap();
        nodes[ID] = std::to_string(node->id());
        nodes[NODE_TYPE] = NODE_TYPE_MAP.at(node->type());
        nodes[NRESULTS] = std::to_string(node->nresults());
        nodes[INST_TYPE] = INST_TYPE_MAP.at(node->instType());
        nodes[LABEL] = node->label();
        writeNode(nodes);
    }

private:
    inline std::map<std::string, std::string> getEdgesMap() {
        std::map<std::string, std::string> res;
        if (_datalog) {
            res[SRC] = "0";
            res[DEST] = "0";
            res[EDGE_TYPE] = "None";
            res[LABEL] = "None";
            res[PDG_TYPE] = "None";
            res[CONST_TYPE] = "None";
            res[CONST_VALUE_I] = "0";
            res[CONST_VALUE_F] = "0";
            res[AST_ORDER] = "0";
        }
        return res;
    }

    inline std::map<std::string, std::string> getNodesMap() {
        std::map<std::string, std::string> res;
        if (_datalog) {
            res[ID] = "0";
            res[NODE_TYPE] = "None";
            res[NAME] = "None";
            res[INDEX] = "0";
            res[NARGS] = "0";
            res[NLOCALS] = "0";
            res[NRESULTS] = "0";
            res[IS_IMPORT] = "0";
            res[IS_EXPORT] = "0";
            res[VAR_TYPE] = "None";
            res[INST_TYPE] = "None";
            res[OPCODE] = "None";
            res[CONST_TYPE] = "None";
            res[CONST_VALUE_I] = "0";
            res[CONST_VALUE_F] = "0";
            res[LABEL] = "None";
            res[OFFSET] = "0";
            res[HAS_ELSE] = "0";
        }
        return res;
    }

private:
    inline void writeNode(std::map<std::string, std::string>& node) {
        // id,nodeType,name,index,nargs,nlocals,nresults,isImport,isExport,varType,instType,opcode,constType,constValueI,constValueF,label,offset,hasElse
        std::replace(node[NAME].begin(), node[NAME].end(), ',', '_');
        std::replace(node[LABEL].begin(), node[LABEL].end(), ',', '_');
        std::replace(node[CONST_VALUE_F].begin(), node[CONST_VALUE_F].end(),
                     ',', '.');
        _nodes->Writef(
            "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
            node[ID].c_str(), node[NODE_TYPE].c_str(), node[NAME].c_str(),
            node[INDEX].c_str(), node[NARGS].c_str(), node[NLOCALS].c_str(),
            node[NRESULTS].c_str(), node[IS_IMPORT].c_str(),
            node[IS_EXPORT].c_str(), node[VAR_TYPE].c_str(),
            node[INST_TYPE].c_str(), node[OPCODE].c_str(),
            node[CONST_TYPE].c_str(), node[CONST_VALUE_I].c_str(),
            node[CONST_VALUE_F].c_str(), node[LABEL].c_str(),
            node[OFFSET].c_str(), node[HAS_ELSE].c_str());
        _numNodes++;
    }

    inline void writeEdge(std::map<std::string, std::string>& edge) {
        // src,dest,edgeType,label,pdgType,constType,constValueI,constValueF,astOrder
        std::replace(edge[LABEL].begin(), edge[LABEL].end(), ',', '_');
        std::replace(edge[CONST_VALUE_F].begin(), edge[CONST_VALUE_F].end(),
                     ',', '.');
        _edges->Writef("%s,%s,%s,%s,%s,%s,%s,%s,%s\n", edge[SRC].c_str(),
                       edge[DEST].c_str(), edge[EDGE_TYPE].c_str(),
                       edge[LABEL].c_str(), edge[PDG_TYPE].c_str(),
                       edge[CONST_TYPE].c_str(), edge[CONST_VALUE_I].c_str(),
                       edge[CONST_VALUE_F].c_str(), edge[AST_ORDER].c_str());
        _numEdges++;
    }

    inline void printInfo() {
        json result;
        result["version"] = CMAKE_PROJECT_VERSION;
        result["date"] = Utils::currentDate();
        result["nodes"] = _numNodes;
        result["edges"] = _numEdges;
        result["nodeHeader"] = {
            ID,        NODE_TYPE, NAME,       INDEX,         NARGS,
            NLOCALS,   NRESULTS,  IS_IMPORT,  IS_EXPORT,     VAR_TYPE,
            INST_TYPE, OPCODE,    CONST_TYPE, CONST_VALUE_I, CONST_VALUE_F,
            LABEL,     OFFSET,    HAS_ELSE};
        result["edgeHeader"] = {SRC,           DEST,          EDGE_TYPE,
                                LABEL,         PDG_TYPE,      CONST_TYPE,
                                CONST_VALUE_I, CONST_VALUE_F, AST_ORDER};

        _info->Writef("%s", result.dump(2).c_str());
    }
};

}  // namespace wasmati

#endif /* end of WASMATI_DOT_H */
