#ifndef WASMATI_PDG_BUILDER_H_
#define WASMATI_PDG_BUILDER_H_

#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include "graph.h"
#include "query.h"
#include "src/cast.h"

using namespace wabt;

namespace wasmati {
class ReachDefinition;

class PDG {
private:
    ModuleContext& mc;
    Graph& graph;

    std::list<std::tuple<Node*, std::shared_ptr<std::stack<LoopInst*>>, Node*>>
        _dfsList;

    Func* currentFunction = nullptr;
    std::map<Node*, std::set<std::shared_ptr<ReachDefinition>>> _reachDef;
    std::map<Node*, std::shared_ptr<ReachDefinition>> _loops;
    std::map<Node*,
             std::pair<std::shared_ptr<ReachDefinition>,
                       std::shared_ptr<ReachDefinition>>>
        _loopsEntrances;
    std::map<Node*, NodeSet> _loopsInsts;
    std::stack<LoopInst*> _loopsStack;
    Node* _lastNode;

    json _verbose;
    NodeSet _verboseLoops;

public:
    PDG(ModuleContext& mc, Graph& graph)
        : mc(mc), graph(graph), _verbose(json::array()) {}

    ~PDG() {}

    void generatePDG();

private:
    void visitCFGEdge(Edge* e, std::shared_ptr<std::stack<LoopInst*>> stack);
    void visitInstructions(Instructions* e);
    void visitNopInst(NopInst* node);
    void visitUnreachableInst(UnreachableInst* node);
    void visitReturnInst(ReturnInst* node);
    void visitBrTableInst(BrTableInst* node);
    void visitCallIndirectInst(CallIndirectInst* node);
    void visitDropInst(DropInst* node);
    void visitSelectInst(SelectInst* node);
    void visitMemorySizeInst(MemorySizeInst* node);
    void visitMemoryGrowInst(MemoryGrowInst* node);
    void visitConstInst(ConstInst* node);
    void visitBinaryInst(BinaryInst* node);
    void visitCompareInst(CompareInst* node);
    void visitConvertInst(ConvertInst* node);
    void visitUnaryInst(UnaryInst* node);
    void visitLoadInst(LoadInst* node);
    void visitStoreInst(StoreInst* node);
    void visitBrInst(BrInst* node);
    void visitBrIfInst(BrIfInst* node);
    void visitCallInst(CallInst* node);
    void visitGlobalGetInst(GlobalGetInst* node);
    void visitGlobalSetInst(GlobalSetInst* node);
    void visitLocalGetInst(LocalGetInst* node);
    void visitLocalSetInst(LocalSetInst* node);
    void visitLocalTeeInst(LocalTeeInst* node);
    void visitBeginBlockInst(BeginBlockInst* node);
    void visitBlockInst(BlockInst* node);
    void visitLoopInst(LoopInst* node);
    void visitIfInst(IfInst* node);

    // Auxiliars
    inline bool waitPaths(Instruction* inst, bool isLoop = false);
    inline std::shared_ptr<ReachDefinition> getReachDef(Instruction* inst);
    inline void advance(Instruction* inst,
                        std::shared_ptr<ReachDefinition> resultReachDef);

    inline void logDefinition(Node* inst, std::shared_ptr<ReachDefinition> def);
};

struct Label {
    std::string name;
    Index pointer;

    Label(std::string name, Index pointer) : name(name), pointer(pointer) {}

    inline bool equals(const Label& other) const {
        return name.compare(other.name) == 0 && pointer == other.pointer;
    }

    bool operator==(const Label& o) const { return equals(o); }

    friend void to_json(json& j, const Label& l) {
        j.push_back({l.name, l.pointer});
    }

    friend void from_json(const json& j, Label& l) {
        // Do not care.
    }
};

// A set
class Definition {
public:
    struct Var {
        const std::string name;
        const Const* value;
        const PDGType type;

        Var(const std::string& name, const PDGType type)
            : name(name), value(nullptr), type(type) {}

        Var(const Const* value)
            : name(Utils::writeConst(*value)),
              value(value),
              type(PDGType::Const) {}

        Var(const std::string& name, const Const* value, const PDGType type)
            : name(name), value(value), type(type) {}

        Var(const Var& var) : Var(var.name, var.value, var.type) {}

        bool operator==(const Var& o) const {
            return name.compare(o.name) == 0 && value == o.value &&
                   type == o.type;
        }

        bool operator<(const Var& o) const {
            static std::ostringstream left, right;
            left.clear();
            right.clear();
            left << name << static_cast<int>(type) << value;
            right << o.name << static_cast<int>(o.type) << o.value;
            return name < o.name;
        }
    };

private:
    std::map<const Var, std::set<Node*>> _def;

public:
    Definition() {}

    Definition(const Definition& def) : _def(def._def) {}

    inline void insert(const std::string& name, PDGType type, Node* node) {
        _def[Var(name, type)].insert(node);
    }

    inline void insert(const Const* value, Node* node) {
        _def[Var(value)].insert(node);
    }

    inline void unionDef(std::shared_ptr<Definition> otherDef) {
        for (auto& kv : otherDef->_def) {
            _def[kv.first].insert(kv.second.begin(), kv.second.end());
        }
    }

    inline void clear(const Var& var) { _def[var].clear(); }

    inline void clear(Node* inst) {
        for (auto& kv : _def) {
            kv.second.clear();
            kv.second.insert(inst);
        }
    }

    inline void clear() { _def.clear(); }

    inline bool isEmpty() const { return _def.size() == 0; }

    inline void insertPDGEdge(Node* target) {
        for (auto const& kv : _def) {
            for (auto& node : kv.second) {
                auto inEdges = target->inEdges(EdgeType::PDG);
                auto filter = Query::filterEdges(inEdges, [&](Edge* e) {
                    return e->src() == node && e->dest() == target &&
                           e->pdgType() == kv.first.type &&
                           e->label() == kv.first.name;
                });
                if (filter.size() > 0) {
                    continue;
                }
                if (kv.first.type == PDGType::Const) {
                    new PDGEdgeConst(node, target, *kv.first.value);
                } else {
                    new PDGEdge(node, target, kv.first.name, kv.first.type);
                }
            }
        }
    }

    inline void removeConsts() {
        for (auto it = _def.begin(); it != _def.end();) {
            if (it->first.type == PDGType::Const) {
                it = _def.erase(it);
            } else {
                ++it;
            }
        }
    }

    inline bool equals(const Definition& other) {
        if (_def.size() != other._def.size()) {
            return false;
        }
        for (auto it = _def.cbegin(), ito = other._def.cbegin();
             it != _def.cend(); ++it, ++ito) {
            if (!(it->first == ito->first)) {
                return false;
            } else if (it->second != ito->second) {
                return false;
            }
        }
        return true;
    }

    friend void to_json(json& j, const Definition& d) {
        for (auto const& kv : d._def) {
            json def;
            def["name"] = kv.first.name;
            def["type"] = kv.first.type;
            json nodes = json::array();
            for (auto node : kv.second) {
                nodes.push_back(node->getId());
            }
            def["nodes"] = nodes;
            j.push_back(def);
        }
    }

    friend void from_json(const json& j, Definition& d) {
        // Do not care.
    }
};

// A set of sets indexed by name
class Definitions {
    std::map<std::string, std::shared_ptr<Definition>> _defs;

public:
    Definitions() {}

    Definitions(const Definitions& defs) {
        for (auto const& kv : defs._defs) {
            _defs.insert(std::make_pair(
                kv.first, std::make_shared<Definition>(*kv.second)));
        }
    }

    inline void insert(const std::string& var, Definition& def) {
        _defs.emplace(std::make_pair(var, std::make_shared<Definition>(def)));
    }

    inline void insert(const std::string& var) {
        _defs[var] = std::make_shared<Definition>();
    }

    inline void insert(const std::string& var,
                       std::shared_ptr<Definition> def) {
        _defs[var] = def;
    }

    inline std::shared_ptr<Definition> get(const std::string& var) {
        return _defs.at(var);
    }

    inline void unionDef(const Definitions& otherDefs) {
        for (auto const& kv : otherDefs._defs) {
            _defs[kv.first]->unionDef(kv.second);
        }
    }

    inline bool equals(const Definitions& other) {
        if (_defs.size() != other._defs.size()) {
            return false;
        }

        for (auto it = _defs.cbegin(), ito = other._defs.cbegin();
             it != _defs.cend(); ++it, ++ito) {
            if (it->first.compare(ito->first) != 0) {
                return false;
            } else if (!it->second.get()->equals(*ito->second.get())) {
                return false;
            }
        }
        return true;
    }

    friend void to_json(json& j, const Definitions& d) {
        for (auto const& kv : d._defs) {
            j[kv.first] = *kv.second;
        }
    }

    friend void from_json(const json& j, Definitions& v) {
        // Do not care.
    }
};

class ReachDefinition {
    typedef std::list<std::shared_ptr<Definition>> Stack;
    typedef std::list<Label> Labels;

    Definitions _globals;
    Definitions _locals;
    Stack _stack;
    Labels _labels;

public:
    ReachDefinition() {}

    ReachDefinition(const ReachDefinition& reachDef)
        : _globals(reachDef._globals),
          _locals(reachDef._locals),
          _stack(reachDef._stack),
          _labels(reachDef._labels) {}

    inline void insertGlobal(const std::string& var) { _globals.insert(var); }

    inline void insertGlobal(const std::string& var,
                             std::shared_ptr<Definition> def) {
        _globals.insert(var, def);
    }

    inline std::shared_ptr<Definition> getGlobal(const std::string& var) {
        return _globals.get(var);
    }

    inline void insertLocal(const std::string& var) { _locals.insert(var); }

    inline void insertLocal(const std::string& var,
                            std::shared_ptr<Definition> def) {
        _locals.insert(var, def);
    }

    inline std::shared_ptr<Definition> getLocal(const std::string& var) {
        return _locals.get(var);
    }

    inline void push() { _stack.emplace_front(std::make_shared<Definition>()); }

    inline void push(std::list<std::shared_ptr<Definition>> list) {
        _stack.splice(_stack.begin(), list);
    }

    inline void push(const Definition& def) {
        _stack.emplace_front(std::make_shared<Definition>(def));
    }

    inline void push(std::shared_ptr<Definition> def) {
        _stack.emplace_front(std::make_shared<Definition>(*def));
    }

    inline std::shared_ptr<Definition> pop() {
        auto top = peek();
        _stack.pop_front();
        return top;
    }

    inline std::list<std::shared_ptr<Definition>> pop(size_t n) {
        auto end = std::next(_stack.begin(), std::min(n, _stack.size()));
        std::list<std::shared_ptr<Definition>> result(_stack.begin(), end);
        _stack.erase(_stack.begin(), end);
        return result;
    }

    inline std::shared_ptr<Definition> peek() { return _stack.front(); }

    inline void unionDef(const ReachDefinition& otherDef) {
        _globals.unionDef(otherDef._globals);
        _locals.unionDef(otherDef._locals);

        assert(_stack.size() == otherDef._stack.size());
        assert(_labels == otherDef._labels);

        for (auto it = std::make_pair(_stack.begin(), otherDef._stack.begin());
             it.first != _stack.end(); it.first++, it.second++) {
            (*it.first)->unionDef(*it.second);
        }
    }

    inline void unionDef(std::shared_ptr<ReachDefinition> otherDef) {
        if (this != otherDef.get()) {
            unionDef(*otherDef);
        }
    }

    inline Index stackSize() const {
        if (_labels.size() > 0) {
            return _stack.size() - _labels.front().pointer;
        }
        return _stack.size();
    }

    inline bool containsLabel(std::string name) {
        for (Label label : _labels) {
            if (name.compare(label.name) == 0) {
                return true;
            }
        }
        return false;
    }

    inline void pushLabel(std::string name) {
        _labels.emplace_front(name, _stack.size());
    }

    inline void popLabel(std::string name) {
        assert(containsLabel(name));
        while (true) {
            Label top = _labels.front();
            while (stackSize() != 0) {
                pop();
            }
            _labels.pop_front();
            if (top.name.compare(name) == 0) {
                return;
            }
        }
    }

    inline void popAllLabel() {
        _labels.clear();
        _stack.clear();
    }

    inline bool equals(const ReachDefinition& other) {
        if (!_globals.equals(other._globals)) {
            return false;
        }
        if (!_locals.equals(other._locals)) {
            return false;
        }
        if (_stack.size() == other._stack.size()) {
            for (auto it = _stack.cbegin(), ito = other._stack.cbegin();
                 it != _stack.cend(); ++it, ++ito) {
                if (!(*it)->equals(*(*ito))) {
                    return false;
                }
            }
        }
        if (_labels.size() == other._labels.size()) {
            for (auto it = _labels.cbegin(), ito = other._labels.cbegin();
                 it != _labels.cend(); ++it, ++ito) {
                if (!(*it).equals((*ito))) {
                    return false;
                }
            }
        }
        return true;
    }

    friend void to_json(json& j, const ReachDefinition& v) {
        j["globals"] = v._globals;
        j["locals"] = v._locals;
        j["labels"] = v._labels;
        json stack = json::array();
        for (auto def : v._stack) {
            stack.push_back(*def);
        }
        j["stack"] = stack;
    }

    friend void from_json(const json& j, ReachDefinition& v) {
        // Do not care.
    }
};

}  // namespace wasmati
#endif  // WABT_PDG_BUILDER_H_
