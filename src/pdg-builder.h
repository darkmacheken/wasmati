#ifndef WASMATI_PDG_BUILDER_H_
#define WASMATI_PDG_BUILDER_H_

#include <list>
#include <map>
#include <set>
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
    inline bool waitPaths(Instruction* inst);
    inline std::shared_ptr<ReachDefinition> getReachDef(Instruction* inst);
    inline void advance(Instruction* inst,
                        std::shared_ptr<ReachDefinition> resultReachDef);
};

struct Label {
    std::string name;
    Index pointer;

    Label(std::string name, Index pointer) : name(name), pointer(pointer) {}
};

// A set
class Definition {
public:
    struct Var {
        const std::string name;
        const PDGEdge::Type type;

        Var(const std::string& name, const PDGEdge::Type type)
            : name(name), type(type) {}

        bool operator<(const Var& o) const {
            std::hash<std::string> str_hash;
            return str_hash(name + std::to_string(static_cast<int>(type))) <
                   str_hash(o.name + std::to_string(static_cast<int>(o.type)));
        }
    };

private:
    std::map<Var, std::set<Node*>> _def;

public:
    Definition() {}

    Definition(const Definition& def) : _def(def._def) {}

    inline void insert(const std::string& name,
                       PDGEdge::Type type,
                       Node* node) {
        _def[Var(name, type)].insert(node);
    }

    inline void unionDef(std::shared_ptr<Definition> otherDef) {
        for (auto kv : otherDef->_def) {
            _def[Var(kv.first.name, kv.first.type)].insert(kv.second.begin(),
                                                           kv.second.end());
        }
    }

    inline void clear(const Var& var) { _def[var].clear(); }

    inline void clear(Node* inst) {
        for (auto kv : _def) {
            auto test = kv.first;
            _def[kv.first].clear();
            _def[kv.first].insert(inst);
        }
    }

    inline void clear() { _def.clear(); }

    inline bool isEmpty() const { return _def.size() == 0; }

    inline void insertPDGEdge(Node* target) {
        for (auto kv : _def) {
            for (auto& node : kv.second) {
                new PDGEdge(node, target, kv.first.name, kv.first.type);
            }
        }
    }
};

// A set of sets indexed by name
class Definitions {
    std::map<std::string, std::shared_ptr<Definition>> _defs;

public:
    Definitions() {}

    Definitions(const Definitions& defs) {
        for (auto kv : defs._defs) {
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
        for (auto kv : otherDefs._defs) {
            _defs[kv.first]->unionDef(kv.second);
        }
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
          _stack(reachDef._stack) {}

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
        _stack.insert(_stack.end(), list.begin(), list.end());
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
            while (_stack.size() != 0) {
                pop();
            }
            if (top.name.compare(name) == 0) {
                return;
            }
        }
    }
};

}  // namespace wasmati
#endif  // WABT_PDG_BUILDER_H_
