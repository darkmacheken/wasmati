#ifndef WASMATI_PDG_BUILDER_H_
#define WASMATI_PDG_BUILDER_H_

#include <list>
#include <map>
#include <set>
#include <stack>
#include "src/graph.h"
#include "src/query.h"

using namespace wabt;

namespace wasmati {
class ReachDefinition;

class PDG : GraphVisitor {
private:
    ModuleContext& mc;
    Graph& graph;

    Func* currentFunction = nullptr;
    std::map<Node*, std::vector<std::shared_ptr<ReachDefinition>>> _reachDef;
    std::map<Node*, std::shared_ptr<ReachDefinition>> _loops;
    std::map<Node*, std::set<Node*>> _loopsInsts;
    std::stack<LoopInst*> _loopsStack;

public:
    PDG(ModuleContext& mc, Graph& graph) : mc(mc), graph(graph) {}

    ~PDG() {}

    void generatePDG(GenerateCPGOptions& options);

private:
    // Inherited via GraphVisitor
    virtual void visitASTEdge(ASTEdge* e) override;
    virtual void visitCFGEdge(CFGEdge* e) override;
    virtual void visitPDGEdge(PDGEdge* e) override;
    virtual void visitModule(Module* node) override;
    virtual void visitFunction(Function* node) override;
    virtual void visitFunctionSignature(FunctionSignature* node) override;
    virtual void visitParameters(Parameters* node) override;
    virtual void visitInstructions(Instructions* node) override;
    virtual void visitLocals(Locals* node) override;
    virtual void visitResults(Results* node) override;
    virtual void visitElse(Else* node) override;
    virtual void visitStart(Start* node) override;
    virtual void visitTrap(Trap* node) override;
    virtual void visitVarNode(VarNode* node) override;
    virtual void visitNopInst(NopInst* node) override;
    virtual void visitUnreachableInst(UnreachableInst* node) override;
    virtual void visitReturnInst(ReturnInst* node) override;
    virtual void visitBrTableInst(BrTableInst* node) override;
    virtual void visitCallIndirectInst(CallIndirectInst* node) override;
    virtual void visitDropInst(DropInst* node) override;
    virtual void visitSelectInst(SelectInst* node) override;
    virtual void visitMemorySizeInst(MemorySizeInst* node) override;
    virtual void visitMemoryGrowInst(MemoryGrowInst* node) override;
    virtual void visitConstInst(ConstInst* node) override;
    virtual void visitBinaryInst(BinaryInst* node) override;
    virtual void visitCompareInst(CompareInst* node) override;
    virtual void visitConvertInst(ConvertInst* node) override;
    virtual void visitUnaryInst(UnaryInst* node) override;
    virtual void visitLoadInst(LoadInst* node) override;
    virtual void visitStoreInst(StoreInst* node) override;
    virtual void visitBrInst(BrInst* node) override;
    virtual void visitBrIfInst(BrIfInst* node) override;
    virtual void visitCallInst(CallInst* node) override;
    virtual void visitGlobalGetInst(GlobalGetInst* node) override;
    virtual void visitGlobalSetInst(GlobalSetInst* node) override;
    virtual void visitLocalGetInst(LocalGetInst* node) override;
    virtual void visitLocalSetInst(LocalSetInst* node) override;
    virtual void visitLocalTeeInst(LocalTeeInst* node) override;
    virtual void visitBeginBlockInst(BeginBlockInst* node) override;
    virtual void visitBlockInst(BlockInst* node) override;
    virtual void visitLoopInst(LoopInst* node) override;
    virtual void visitIfInst(IfInst* node) override;

    // Auxiliars
    inline bool waitPaths(Instruction* inst, bool isLoop = false);
    inline std::shared_ptr<ReachDefinition> getReachDef(Instruction* inst);
    inline void advance(Instruction* inst,
                        std::shared_ptr<ReachDefinition> resultReachDef);
};

struct Label {
    std::string name;
    Index pointer;

    Label(std::string name, Index pointer) : name(name), pointer(pointer) {}

    inline bool equals(const Label& other) const {
        return name.compare(other.name) == 0 && pointer == other.pointer;
    }
};

// A set
class Definition {
public:
    struct Var {
        const std::string name;
        const Const value;
        const PDGType type;

        Var(const std::string& name, const PDGType type)
            : name(name), value(Const::I32()), type(type) {}

        Var(const Const& value)
            : name(ConstInst::writeConst(value)),
              value(value),
              type(PDGType::Const) {}

        Var(const std::string& name, const Const& value, const PDGType type)
            : name(name), value(value), type(type) {}

        bool operator<(const Var& o) const {
            std::hash<std::string> str_hash;
            return str_hash(name + std::to_string(static_cast<int>(type))) <
                   str_hash(o.name + std::to_string(static_cast<int>(o.type)));
        }

        inline bool equals(const Var& other) const {
            return name.compare(other.name) == 0 && type == other.type;
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

    inline void insert(const Const& value, Node* node) {
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

    inline void insertPDGEdge(Node* target, bool isLoopStackEmpty) {
        if (!isLoopStackEmpty) {
            return;
        }
        for (auto const& kv : _def) {
            for (auto& node : kv.second) {
                if (kv.first.type == PDGType::Const) {
                    new PDGEdgeConst(node, target, kv.first.value);
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
            if (!it->first.equals(ito->first)) {
                return false;
            } else if (it->second != ito->second) {
                return false;
            }
        }
        return true;
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

        assert(_stack.size() == otherDef.stackSize());

        for (auto it = std::make_pair(_stack.begin(), otherDef._stack.begin());
             it.first != _stack.end(); it.first++, it.second++) {
            (*it.first)->unionDef(*it.second);
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
};

}  // namespace wasmati
#endif  // WABT_PDG_BUILDER_H_
