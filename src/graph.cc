#include "src/graph.h"

namespace wasmati {

const char functionSignatureName[] = "FunctionSignature";
const char instructionsName[] = "Instructions";
const char parametersName[] = "Parameters";
const char localsName[] = "Locals";
const char resultsName[] = "Results";
const char elseName[] = "Else";
const char trapName[] = "Trap";
const char startName[] = "Start";

Index Node::idCount = 0;

const std::map<NodeType, std::string> NODE_TYPE_MAP = {
    {NodeType::Module, "0"},       {NodeType::Function, "1"},
    {NodeType::VarNode, "2"},      {NodeType::FunctionSignature, "3"},
    {NodeType::Instructions, "4"}, {NodeType::Instruction, "5"},
    {NodeType::Parameters, "6"},   {NodeType::Locals, "7"},
    {NodeType::Results, "8"},      {NodeType::Return, "9"},
    {NodeType::Else, "10"},        {NodeType::Trap, "11"},
    {NodeType::Start, "12"},
};

const std::map<std::string, NodeType> NODE_TYPE_MAP_R = {
    {"0", NodeType::Module},       {"1", NodeType::Function},
    {"2", NodeType::VarNode},      {"3", NodeType::FunctionSignature},
    {"4", NodeType::Instructions}, {"5", NodeType::Instruction},
    {"6", NodeType::Parameters},   {"7", NodeType::Locals},
    {"8", NodeType::Results},      {"9", NodeType::Return},
    {"10", NodeType::Else},        {"11", NodeType::Trap},
    {"12", NodeType::Start},
};

const std::map<ExprType, std::string> INST_TYPE_MAP = {
    {ExprType::Nop, "0"},        {ExprType::Unreachable, "1"},
    {ExprType::Return, "2"},     {ExprType::BrTable, "3"},
    {ExprType::Drop, "4"},       {ExprType::Select, "5"},
    {ExprType::MemorySize, "6"}, {ExprType::MemoryGrow, "7"},
    {ExprType::Const, "8"},      {ExprType::Binary, "9"},
    {ExprType::Compare, "10"},   {ExprType::Convert, "11"},
    {ExprType::Unary, "12"},     {ExprType::Load, "13"},
    {ExprType::Store, "14"},     {ExprType::Br, "15"},
    {ExprType::BrIf, "16"},      {ExprType::GlobalGet, "17"},
    {ExprType::GlobalSet, "18"}, {ExprType::LocalGet, "19"},
    {ExprType::LocalSet, "20"},  {ExprType::LocalTee, "21"},
    {ExprType::Call, "22"},      {ExprType::CallIndirect, "23"},
    {ExprType::First, "24"},     {ExprType::Block, "25"},
    {ExprType::Loop, "26"},      {ExprType::If, "27"},
};

const std::map<std::string, ExprType> INST_TYPE_MAP_R = {
    {"0", ExprType::Nop},        {"1", ExprType::Unreachable},
    {"2", ExprType::Return},     {"3", ExprType::BrTable},
    {"4", ExprType::Drop},       {"5", ExprType::Select},
    {"6", ExprType::MemorySize}, {"7", ExprType::MemoryGrow},
    {"8", ExprType::Const},      {"9", ExprType::Binary},
    {"10", ExprType::Compare},   {"11", ExprType::Convert},
    {"12", ExprType::Unary},     {"13", ExprType::Load},
    {"14", ExprType::Store},     {"15", ExprType::Br},
    {"16", ExprType::BrIf},      {"17", ExprType::GlobalGet},
    {"18", ExprType::GlobalSet}, {"19", ExprType::LocalGet},
    {"20", ExprType::LocalSet},  {"21", ExprType::LocalTee},
    {"22", ExprType::Call},      {"23", ExprType::CallIndirect},
    {"24", ExprType::First},     {"25", ExprType::Block},
    {"26", ExprType::Loop},      {"27", ExprType::If},
};

const std::map<EdgeType, std::string> EDGE_TYPES_MAP = {
    {EdgeType::AST, "0"},
    {EdgeType::CFG, "1"},
    {EdgeType::PDG, "2"},
    {EdgeType::CG, "3"},
    {EdgeType::PG, "4"}};

const std::map<std::string, EdgeType> EDGE_TYPES_MAP_R = {
    {"0", EdgeType::AST},
    {"1", EdgeType::CFG},
    {"2", EdgeType::PDG},
    {"3", EdgeType::CG},
    {"4", EdgeType::PG}};

const std::map<PDGType, std::string> PDG_TYPE_MAP = {
    {PDGType::Const, "0"},
    {PDGType::Control, "1"},
    {PDGType::Function, "2"},
    {PDGType::Global, "3"},
    {PDGType::Local, "4"}};

const std::map<std::string, PDGType> PDG_TYPE_MAP_R = {
    {"0", PDGType::Const},
    {"1", PDGType::Control},
    {"2", PDGType::Function},
    {"3", PDGType::Global},
    {"4", PDGType::Local}};

Node::~Node() {
    for (auto e : _outEdges) {
        delete e;
    }
}

EdgeSet Node::inEdges(EdgeType type) {
    EdgeSet res;
    for (auto e : _inEdges) {
        if (e->type() == type) {
            res.insert(e);
        }
    }
    return res;
}

EdgeSet Node::outEdges(EdgeType type) {
    EdgeSet res;
    for (auto e : _outEdges) {
        if (e->type() == type) {
            res.insert(e);
        }
    }
    return res;
}

inline Edge* Node::getOutEdge(Index i, EdgeType type) {
    auto edges = outEdges(type);
    assert(i < edges.size());
    std::vector<Edge*> vec(edges.begin(), edges.end());
    std::sort(vec.begin(), vec.end(), [](Edge* a, Edge* b) {
        return a->dest()->getId() < b->dest()->getId();
    });
    return vec[i];
}

inline Edge* Node::getInEdge(Index i, EdgeType type) {
    auto edges = inEdges(type);
    assert(i < edges.size());
    std::vector<Edge*> vec(edges.begin(), edges.end());
    std::sort(vec.begin(), vec.end(), [](Edge* a, Edge* b) {
        return a->src()->getId() < b->src()->getId();
    });

    return vec[i];
}

Node* Node::getChild(Index n, EdgeType type) {
    return getOutEdge(n, type)->dest();
}

Node* Node::getParent(Index n, EdgeType type) {
    return getInEdge(n, type)->src();
}

bool Node::hasEdgesOf(EdgeType type) const {
    return hasInEdgesOf(type) || hasOutEdgesOf(type);
}

bool Node::hasInEdgesOf(EdgeType type) const {
    for (auto e : _inEdges) {
        if (e->type() == type) {
            return true;
        }
    }
    return false;
}

bool Node::hasOutEdgesOf(EdgeType type) const {
    for (auto e : _outEdges) {
        if (e->type() == type) {
            return true;
        }
    }
    return false;
}

void Node::accept(GraphVisitor* visitor) {
    assert(false);
}

void Node::acceptEdges(GraphVisitor* visitor) {
    for (Edge* e : _outEdges) {
        e->accept(visitor);
    }
}

void Graph::populate(std::string filePath) {
    csv2::Reader<csv2::delimiter<','>, csv2::quote_character<'"'>,
                 csv2::first_row_is_header<false>,
                 csv2::trim_policy::trim_whitespace>
        reader;
    reader.mmap(filePath);

    bool seenDelimiter = false;
    Index line = 1;
    for (auto row : reader) {
        line++;
        std::vector<std::string> rowParse;
        for (auto cell : row) {
            std::string cellContent;
            cell.read_raw_value(cellContent);
            rowParse.push_back(cellContent);
        }

        // ignore empty lines
        if (rowParse.size() == 0) {
            continue;
        }

        if (rowParse[0] == "-") {
            seenDelimiter = true;
            continue;
        }

        if (seenDelimiter) {
            Factory::createEdge(rowParse, _nodes);
        } else {
            Node* node = Factory::createNode(rowParse);
            insertNode(node);
            if (node->type() == NodeType::Module) {
                setModule(dynamic_cast<Module*>(node));
            } else if (node->type() == NodeType::Trap) {
                setTrap(dynamic_cast<Trap*>(node));
            } else if (node->type() == NodeType::Start) {
                setStart(dynamic_cast<Start*>(node));
            }
        }
    }
}

void Module::accept(GraphVisitor* visitor) {
    visitor->visitModule(this);
}
void Function::accept(GraphVisitor* visitor) {
    visitor->visitFunction(this);
}
void VarNode::accept(GraphVisitor* visitor) {
    visitor->visitVarNode(this);
}

void ConstInst::accept(GraphVisitor* visitor) {
    visitor->visitConstInst(this);
}

void IfInst::accept(GraphVisitor* visitor) {
    visitor->visitIfInst(this);
}

void ASTEdge::accept(GraphVisitor* visitor) {
    visitor->visitASTEdge(this);
}

void CFGEdge::accept(GraphVisitor* visitor) {
    visitor->visitCFGEdge(this);
}

void PDGEdge::accept(GraphVisitor* visitor) {
    visitor->visitPDGEdge(this);
}

void CGEdge::accept(GraphVisitor* visitor) {
    visitor->visitCGEdge(this);
}

void PGEdge::accept(GraphVisitor* visitor) {
    visitor->visitPGEdge(this);
}

void BeginBlockInst::accept(GraphVisitor* visitor) {
    visitor->visitBeginBlockInst(this);
}

bool Compare::operator()(Node* const& n1, Node* const& n2) const {
    return n1->getId() < n2->getId();
}

std::vector<Const*> Factory::consts;
Node* Factory::createNode(std::vector<std::string>& row) {
    assert(row.size() == 17);
    Index id = std::stoi(row[NodeCol::ID]);
    NodeType nodeType = NODE_TYPE_MAP_R.at(row[NodeCol::NodeType]);
    switch (nodeType) {
    // Module
    case NodeType::Module:
        return new Module(id, row[NodeCol::Name]);
    // Function
    case NodeType::Function:
        return new Function(
            id, row[NodeCol::Name], std::stoi(row[NodeCol::Index]),
            std::stoi(row[NodeCol::Nargs]), std::stoi(row[NodeCol::Nlocals]),
            std::stoi(row[NodeCol::Nresults]),
            std::stoi(row[NodeCol::IsImport]),
            std::stoi(row[NodeCol::IsImport]));
    // VarNode
    case NodeType::VarNode:
        return new VarNode(id, row[NodeCol::VarType],
                           std::stoi(row[NodeCol::Index]), row[NodeCol::Name]);
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
        case ExprType::Nop:
            return new NopInst(id);
        // Unreachable
        case ExprType::Unreachable:
            return new UnreachableInst(id);
        // Return
        case ExprType::Return:
            return new ReturnInst(id);
        // BrTable
        case ExprType::BrTable:
            return new BrTableInst(id);
        // Drop
        case ExprType::Drop:
            return new DropInst(id);
        // Select
        case ExprType::Select:
            return new SelectInst(id);
        // MemorySize
        case ExprType::MemorySize:
            return new MemorySizeInst(id);
        // MemoryGrow
        case ExprType::MemoryGrow:
            return new MemoryGrowInst(id);
        // Const
        case ExprType::Const:
            return new ConstInst(
                id, *Factory::createConst(row[NodeCol::ConstType],
                                          row[NodeCol::ConstValue]));
        // Binary
        case ExprType::Binary:
            return new BinaryInst(id, row[NodeCol::Opcode]);
        // Compare
        case ExprType::Compare:
            return new CompareInst(id, row[NodeCol::Opcode]);
        // Convert
        case ExprType::Convert:
            return new ConvertInst(id, row[NodeCol::Opcode]);
        // Unary
        case ExprType::Unary:
            return new UnaryInst(id, row[NodeCol::Opcode]);
        // Load
        case ExprType::Load:
            return new LoadInst(id, row[NodeCol::Opcode],
                                std::stoi(row[NodeCol::Offset]));
        // Store
        case ExprType::Store:
            return new StoreInst(id, row[NodeCol::Opcode],
                                 std::stoi(row[NodeCol::Offset]));
        // Br
        case ExprType::Br:
            return new BrInst(id, row[NodeCol::Label]);
        // BrIf
        case ExprType::BrIf:
            return new BrIfInst(id, row[NodeCol::Label]);
        // GlobalGet
        case ExprType::GlobalGet:
            return new GlobalGetInst(id, row[NodeCol::Label]);
        // GlobalSet
        case ExprType::GlobalSet:
            return new GlobalSetInst(id, row[NodeCol::Label]);
        // LocalGet
        case ExprType::LocalGet:
            return new LocalGetInst(id, row[NodeCol::Label]);
        // LocalSet
        case ExprType::LocalSet:
            return new LocalSetInst(id, row[NodeCol::Label]);
        // LocalTee
        case ExprType::LocalTee:
            return new LocalTeeInst(id, row[NodeCol::Label]);
        // Call
        case ExprType::Call:
            return new CallInst(id, std::stoi(row[NodeCol::Nargs]),
                                std::stoi(row[NodeCol::Nresults]),
                                row[NodeCol::Label]);
        // CallIndirect
        case ExprType::CallIndirect:
            return new CallIndirectInst(id, std::stoi(row[NodeCol::Nargs]),
                                        std::stoi(row[NodeCol::Nresults]),
                                        row[NodeCol::Label]);
        // BeginBlock
        case ExprType::First:
            return new BeginBlockInst(id, row[NodeCol::Label]);
        // Block
        case ExprType::Block:
            return new BlockInst(id, std::stoi(row[NodeCol::Nresults]),
                                 row[NodeCol::Label]);
        // Loop
        case ExprType::Loop:
            return new LoopInst(id, std::stoi(row[NodeCol::Nresults]),
                                row[NodeCol::Label]);
        // If
        case ExprType::If:
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
Edge* Factory::createEdge(std::vector<std::string>& row,
                          std::vector<Node*>& nodes) {
    assert(row.size() == 7);

    wabt::Index src = std::stoi(row[EdgeCol::Src]);
    wabt::Index dest = std::stoi(row[EdgeCol::Dest]);

    assert(src < nodes.size() && nodes[src]->getId() == src);
    assert(dest < nodes.size() && nodes[dest]->getId() == dest);

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
                *Factory::createConst(row[EdgeCol::ConstType],
                                      row[EdgeCol::ConstValue]));
        } else {
            return new PDGEdge(nodes[src], nodes[dest], label, pdgType);
        }
    }
    case EdgeType::CG:
        return new CGEdge(nodes[src], nodes[dest]);
    case EdgeType::PG:
        return new PGEdge(nodes[src], nodes[dest]);
    default:
        assert(false);
        break;
    }
    return nullptr;
}

Const* Factory::createConst(std::string type, std::string value) {
    Const* res = nullptr;
    if (type == "i32") {
        uint32_t val = std::stoi(value);
        res = new Const(Const::I32(val));
    } else if (type == "i64") {
        uint64_t val = std::stol(value);
        res = new Const(Const::I64(val));
    } else if (type == "f32") {
        float valF = std::stof(value);
        uint32_t val;
        memcpy(&val, &valF, sizeof(valF));
        res = new Const(Const::F32(val));
    } else if (type == "f64") {
        double valF = std::stod(value);
        uint64_t val;
        memcpy(&val, &valF, sizeof(valF));
        res = new Const(Const::F64(val));
    } else {
        assert(false);
    }

    if (res != nullptr) {
        Factory::insertConst(res);
    }

    return res;
}

}  // namespace wasmati
