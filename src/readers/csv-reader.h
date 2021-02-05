#ifndef WASMATI_CSV_R_H
#define WASMATI_CSV_R_H
#include "src/graph.h"
#include "zip.h"

#define BUFF_SIZE 8192

namespace wasmati {

struct zip_file_reader_t {
    zip_file_t* file = nullptr;
    char buffer[BUFF_SIZE] = {0};
    size_t current = 0;
    size_t last = 0;
    bool eof = false;
};

class CSVReader {
    Graph* _graph;
    zip_t* _zipArchive;

public:
    CSVReader(std::string zipFileName, Graph* graph) : _graph(graph) {
        int error;
        _zipArchive = zip_open(zipFileName.c_str(), ZIP_RDONLY, &error);
    }

    std::pair<size_t, size_t> readGraph() {
        size_t nodes = 0, edges = 0;
        json infoFile = json::parse(readFile(_zipArchive, "info.json"));
        auto nodesFile = fopen(_zipArchive, "nodes.csv");
        while (!nodesFile->eof) {
            std::string row = readLine(nodesFile);
            if (row.empty()) {
                continue;
            }
            Node* node = parseNode(row);
            _graph->insertNode(node);
            if (node->type() == NodeType::Module) {
                _graph->setModule(dynamic_cast<Module*>(node));
            } else if (node->type() == NodeType::Trap) {
                _graph->setTrap(dynamic_cast<Trap*>(node));
            } else if (node->type() == NodeType::Start) {
                _graph->setStart(dynamic_cast<Start*>(node));
            }
            nodes++;
        }
        fclose(nodesFile);
        auto edgesFile = fopen(_zipArchive, "edges.csv");
        while (!edgesFile->eof) {
            std::string row = readLine(edgesFile);
            if (row.empty()) {
                continue;
            }
            parseEdge(row);
            edges++;
        }
        fclose(edgesFile);
        assert(nodes == infoFile["nodes"]);
        assert(edges == infoFile["edges"]);
        deleteConsts();

        return std::make_pair(nodes, edges);
    }

private:
    zip_file_reader_t* fopen(zip_t* zipFile, std::string fname) {
        zip_file_reader_t* result = new zip_file_reader_t();

        result->file = zip_fopen(zipFile, fname.c_str(), ZIP_FL_UNCHANGED);
        assert(result->file != nullptr);

        return result;
    }

    void fclose(zip_file_reader_t* file) {
        if (file == nullptr) {
            return;
        }
        delete file;
    }

    std::string readLine(zip_file_reader_t* file, std::string prefix = "") {
        if (file->eof) {
            return prefix + "";
        }
        if (file->current == BUFF_SIZE) {
            file->current = file->last = 0;
        }

        if (file->last == 0) {
            file->last = zip_fread(file->file, file->buffer, BUFF_SIZE);
            assert(file->last > 0);
        }

        size_t previous = file->current;
        while (file->current < BUFF_SIZE &&
               file->buffer[file->current] != '\n') {
            if (file->current == file->last) {
                file->eof = true;
                if (previous == file->current) {
                    return prefix + "";
                }
                std::string result(&file->buffer[previous],
                                   file->current - previous - 1);
                return prefix + result;
            }
            file->current++;
        }

        if (file->current == BUFF_SIZE) {
            file->current = file->last = 0;
            if (file->buffer[BUFF_SIZE - 1] == '\n') {
                if (previous == BUFF_SIZE - 1) {
                    return prefix + "";
                }
                std::string result(&file->buffer[previous],
                                   (BUFF_SIZE - 1) - previous);
                return prefix + result;
            } else {
                std::string result;
                if (previous == BUFF_SIZE - 1) {
                    result.assign(&file->buffer[previous], 1);
                } else {
                    result.assign(&file->buffer[previous],
                                  BUFF_SIZE - previous);
                }
                return prefix + readLine(file, result);
            }
        } else if (file->buffer[file->current] == '\n') {
            if (previous == file->current) {
                file->current++;
                return prefix + "";
            }
            std::string result(&file->buffer[previous],
                               file->current - previous);
            file->current++;
            return prefix + result;
        }
        assert(false);
        return prefix + "";
    }

    std::string readFile(zip_t* zipFile, std::string fname) {
        zip_file_t* file = zip_fopen(zipFile, fname.c_str(), ZIP_FL_UNCHANGED);
        zip_stat_t stat;
        assert(zip_stat(zipFile, fname.c_str(), 0, &stat) == 0);
        assert(stat.valid | ZIP_STAT_SIZE);
        char* buffer = new char[stat.size + 1];
        size_t nbytes = zip_fread(file, buffer, stat.size);
        assert(nbytes == stat.size);
        std::string result(buffer, nbytes);
        delete[] buffer;
        return result;
    }

private:
    // To delete in the end
    std::vector<Const*> consts;

    struct NodeCol {
        enum {
            id = 0,
            NodeType,
            Name,
            Index,
            Nargs,
            Nlocals,
            Nresults,
            IsImport,
            IsExport,
            VarType,
            InstType,
            Opcode,
            ConstType,
            ConstValueI,
            ConstValueF,
            Label,
            Offset,
            HasElse
        };
    };

    struct EdgeCol {
        enum {
            Src = 0,
            Dest,
            Type,
            Label,
            PdgType,
            ConstType,
            ConstValueI,
            ConstValueF
        };
    };

    Const* createConst(std::string& type,
                       std::string valueI,
                       std::string& valueF) {
        Const* res = nullptr;
        if (type == "i32") {
            uint32_t val = std::stoi(valueI);
            res = new Const(Const::I32(val));
        } else if (type == "i64") {
            uint64_t val = std::stol(valueI);
            res = new Const(Const::I64(val));
        } else if (type == "f32") {
            float valF = std::stof(valueF);
            uint32_t val;
            memcpy(&val, &valF, sizeof(valF));
            res = new Const(Const::F32(val));
        } else if (type == "f64") {
            double valF = std::stod(valueF);
            uint64_t val;
            memcpy(&val, &valF, sizeof(valF));
            res = new Const(Const::F64(val));
        } else {
            assert(false);
        }

        if (res != nullptr) {
            insertConst(res);
        }

        return res;
    }

    Node* parseNode(std::string& str) {
        auto row = Utils::split(str, ',');
        assert(row.size() == 18);
        Index id = std::stoi(row[NodeCol::id]);
        NodeType nodeType = NODE_TYPE_MAP_R.at(row[NodeCol::NodeType]);
        switch (nodeType) {
        // Module
        case NodeType::Module:
            return new Module(id, row[NodeCol::Name]);
        // Function
        case NodeType::Function:
            return new Function(id, row[NodeCol::Name],
                                std::stoi(row[NodeCol::Index]),
                                std::stoi(row[NodeCol::Nargs]),
                                std::stoi(row[NodeCol::Nlocals]),
                                std::stoi(row[NodeCol::Nresults]),
                                std::stoi(row[NodeCol::IsImport]),
                                std::stoi(row[NodeCol::IsImport]));
        // VarNode
        case NodeType::VarNode:
            return new VarNode(id, row[NodeCol::VarType],
                               std::stoi(row[NodeCol::Index]),
                               row[NodeCol::Name]);
        // FunctionSignature
        case NodeType::FunctionSignature:
            return new FunctionSignature(id);
        // Instructions
        case NodeType::Instructions:
            return new Instructions(id);
        // Parameters
        case NodeType::Parameters:
            return new Parameters(id);
        // Locals
        case NodeType::Locals:
            return new Locals(id);
        // Results
        case NodeType::Results:
            return new Results(id);
        // Else
        case NodeType::Else:
            return new Else(id);
        // Trap
        case NodeType::Trap:
            return new Trap(id);
        // Start
        case NodeType::Start:
            return new Start(id);
        // Instruction
        case NodeType::Instruction: {
            auto instType = INST_TYPE_MAP_R.at(row[NodeCol::InstType]);
            switch (instType) {
            // Nop
            case InstType::Nop:
                return new NopInst(id);
            // Unreachable
            case InstType::Unreachable:
                return new UnreachableInst(id);
            // Return
            case InstType::Return:
                return new ReturnInst(id);
            // BrTable
            case InstType::BrTable:
                return new BrTableInst(id);
            // Drop
            case InstType::Drop:
                return new DropInst(id);
            // Select
            case InstType::Select:
                return new SelectInst(id);
            // MemorySize
            case InstType::MemorySize:
                return new MemorySizeInst(id);
            // MemoryGrow
            case InstType::MemoryGrow:
                return new MemoryGrowInst(id);
            // Const
            case InstType::Const:
                return new ConstInst(id,
                                     *createConst(row[NodeCol::ConstType],
                                                  row[NodeCol::ConstValueI],
                                                  row[NodeCol::ConstValueF]));
            // Binary
            case InstType::Binary:
                return new BinaryInst(id, row[NodeCol::Opcode]);
            // Compare
            case InstType::Compare:
                return new CompareInst(id, row[NodeCol::Opcode]);
            // Convert
            case InstType::Convert:
                return new ConvertInst(id, row[NodeCol::Opcode]);
            // Unary
            case InstType::Unary:
                return new UnaryInst(id, row[NodeCol::Opcode]);
            // Load
            case InstType::Load:
                return new LoadInst(id, row[NodeCol::Opcode],
                                    std::stoi(row[NodeCol::Offset]));
            // Store
            case InstType::Store:
                return new StoreInst(id, row[NodeCol::Opcode],
                                     std::stoi(row[NodeCol::Offset]));
            // Br
            case InstType::Br:
                return new BrInst(id, row[NodeCol::Label]);
            // BrIf
            case InstType::BrIf:
                return new BrIfInst(id, row[NodeCol::Label]);
            // GlobalGet
            case InstType::GlobalGet:
                return new GlobalGetInst(id, row[NodeCol::Label]);
            // GlobalSet
            case InstType::GlobalSet:
                return new GlobalSetInst(id, row[NodeCol::Label]);
            // LocalGet
            case InstType::LocalGet:
                return new LocalGetInst(id, row[NodeCol::Label]);
            // LocalSet
            case InstType::LocalSet:
                return new LocalSetInst(id, row[NodeCol::Label]);
            // LocalTee
            case InstType::LocalTee:
                return new LocalTeeInst(id, row[NodeCol::Label]);
            // Call
            case InstType::Call:
                return new CallInst(id, std::stoi(row[NodeCol::Nargs]),
                                    std::stoi(row[NodeCol::Nresults]),
                                    row[NodeCol::Label]);
            // CallIndirect
            case InstType::CallIndirect:
                return new CallIndirectInst(id, std::stoi(row[NodeCol::Nargs]),
                                            std::stoi(row[NodeCol::Nresults]),
                                            row[NodeCol::Label]);
            // BeginBlock
            case InstType::BeginBlock:
                return new BeginBlockInst(id, std::stoi(row[NodeCol::Nresults]),
                                          row[NodeCol::Label]);
            // Block
            case InstType::Block:
                return new BlockInst(id, std::stoi(row[NodeCol::Nresults]),
                                     row[NodeCol::Label]);
            // Loop
            case InstType::Loop:
                return new LoopInst(id, std::stoi(row[NodeCol::Nresults]),
                                    row[NodeCol::Label]);
            // LoopEnd
            case InstType::EndLoop:
                return new EndLoopInst(id, std::stoi(row[NodeCol::Nresults]),
                                       row[NodeCol::Label]);
            // If
            case InstType::If:
                return new IfInst(id, std::stoi(row[NodeCol::Nresults]),
                                  std::stoi(row[NodeCol::HasElse]));
            default:
                break;
            }
        }
        default:
            break;
        }
        assert(false);
        return nullptr;
    }

    Edge* parseEdge(std::string& str) {
        auto row = Utils::split(str, ',');
        assert(row.size() >= 8);
        Index src = std::stoi(row[EdgeCol::Src]);
        Index dest = std::stoi(row[EdgeCol::Dest]);
        auto& nodes = _graph->getNodes();

        assert(src < nodes.size() && nodes[src]->id() == src);
        assert(dest < nodes.size() && nodes[dest]->id() == dest);

        std::string type = row[EdgeCol::Type];
        switch (Edge::type(type)) {
        case EdgeType::AST:
            return new ASTEdge(nodes[src], nodes[dest]);
        case EdgeType::CFG: {
            std::string label = row[EdgeCol::Label];
            return new CFGEdge(nodes[src], nodes[dest], label);
        }
        case EdgeType::PDG: {
            std::string label = row[EdgeCol::Label];
            auto pdgType = PDG_TYPE_MAP_R.at(row[EdgeCol::PdgType]);
            if (pdgType == PDGType::Const) {
                return new PDGEdgeConst(
                    nodes[src], nodes[dest],
                    *createConst(row[EdgeCol::ConstType],
                                 row[EdgeCol::ConstValueI],
                                 row[EdgeCol::ConstValueF]));
            } else {
                return new PDGEdge(nodes[src], nodes[dest], label, pdgType);
            }
        }
        case EdgeType::CG:
            return new CGEdge(nodes[src], nodes[dest]);
        default:
            assert(false);
            break;
        }
        return nullptr;
    }

    inline void insertConst(Const* expr) { consts.push_back(expr); }

    inline void deleteConsts() {
        for (auto expr : consts) {
            delete expr;
        }
    }
};

}  // namespace wasmati

#endif /* end of WASMATI_DOT_H */
