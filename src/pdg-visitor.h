#ifndef WASMATI_PDG_H
#define WASMATI_PDG_H
#include <list>
#include <set>

#include "src/cast.h"
#include "src/graph-visitor.h"
#include "src/graph.h"

namespace wasmati {
class ReachDefinition;

class PDGvisitor : public GraphVisitor {
    Graph* _graph;
    Func* _currentFunction;
    Instruction* _currentInstruction;
    std::map<Node*, std::vector<std::shared_ptr<ReachDefinition>>> _reachDef;

    // Edges
    void visitASTEdge(ASTEdge*) override { assert(false); };
    void visitCFGEdge(CFGEdge*) override;
    void visitPDGEdge(PDGEdge*) override { assert(false); };

    // Nodes
    void visitModule(Module*) override;
    void visitFunction(Function*) override;
    void visitTypeNode(TypeNode*) override { assert(false); };
    void visitSimpleNode(SimpleNode*) override { assert(false); };
    void visitInstructions(Instructions*) override;
    void visitInstruction(Instruction*) override;
    void visitReturn(Return*) override;
    void visitElse(Else*) override;
    void visitStart(Start*) override { assert(false); };
    void visitTrap(Trap*) override { /*Do nothing*/};
    void visitIndexNode(IndexNode*) override { assert(false); };

protected:
    // Expressions
    void OnBinaryExpr(BinaryExpr*) override;
    void OnBlockExpr(BlockExpr*) override;
    void OnBrExpr(BrExpr*) override;
    void OnBrIfExpr(BrIfExpr*) override;
    void OnBrOnExnExpr(BrOnExnExpr*) override;
    void OnBrTableExpr(BrTableExpr*) override;
    void OnCallExpr(CallExpr*) override;
    void OnCallIndirectExpr(CallIndirectExpr*) override;
    void OnCompareExpr(CompareExpr*) override;
    void OnConstExpr(ConstExpr*) override;
    void OnConvertExpr(ConvertExpr*) override;
    void OnDropExpr(DropExpr*) override;
    void OnGlobalGetExpr(GlobalGetExpr*) override;
    void OnGlobalSetExpr(GlobalSetExpr*) override;
    void OnIfExpr(IfExpr*) override;
    void OnLoadExpr(LoadExpr*) override;
    void OnLocalGetExpr(LocalGetExpr*) override;
    void OnLocalSetExpr(LocalSetExpr*) override;
    void OnLocalTeeExpr(LocalTeeExpr*) override;
    void OnLoopExpr(LoopExpr*) override;
    void OnMemoryCopyExpr(MemoryCopyExpr*) override;
    void OnDataDropExpr(DataDropExpr*) override;
    void OnMemoryFillExpr(MemoryFillExpr*) override;
    void OnMemoryGrowExpr(MemoryGrowExpr*) override;
    void OnMemoryInitExpr(MemoryInitExpr*) override;
    void OnMemorySizeExpr(MemorySizeExpr*) override;
    void OnTableCopyExpr(TableCopyExpr*) override;
    void OnElemDropExpr(ElemDropExpr*) override;
    void OnTableInitExpr(TableInitExpr*) override;
    void OnTableGetExpr(TableGetExpr*) override;
    void OnTableSetExpr(TableSetExpr*) override;
    void OnTableGrowExpr(TableGrowExpr*) override;
    void OnTableSizeExpr(TableSizeExpr*) override;
    void OnTableFillExpr(TableFillExpr*) override;
    void OnRefFuncExpr(RefFuncExpr*) override;
    void OnRefNullExpr(RefNullExpr*) override;
    void OnRefIsNullExpr(RefIsNullExpr*) override;
    void OnNopExpr(NopExpr*) override;
    void OnReturnExpr(ReturnExpr*) override;
    void OnReturnCallExpr(ReturnCallExpr*) override;
    void OnReturnCallIndirectExpr(ReturnCallIndirectExpr*) override;
    void OnSelectExpr(SelectExpr*) override;
    void OnStoreExpr(StoreExpr*) override;
    void OnUnaryExpr(UnaryExpr*) override;
    void OnUnreachableExpr(UnreachableExpr*) override;
    void OnTryExpr(TryExpr*) override;
    void OnThrowExpr(ThrowExpr*) override;
    void OnRethrowExpr(RethrowExpr*) override;
    void OnAtomicWaitExpr(AtomicWaitExpr*) override;
    void OnAtomicNotifyExpr(AtomicNotifyExpr*) override;
    void OnAtomicLoadExpr(AtomicLoadExpr*) override;
    void OnAtomicStoreExpr(AtomicStoreExpr*) override;
    void OnAtomicRmwExpr(AtomicRmwExpr*) override;
    void OnAtomicRmwCmpxchgExpr(AtomicRmwCmpxchgExpr*) override;
    void OnTernaryExpr(TernaryExpr*) override;
    void OnSimdLaneOpExpr(SimdLaneOpExpr*) override;
    void OnSimdShuffleOpExpr(SimdShuffleOpExpr*) override;
    void OnLoadSplatExpr(LoadSplatExpr*) override;

public:
    PDGvisitor(Graph* graph) : _graph(graph) { _currentFunction = nullptr; }
};

// A set
class Definition {
    std::map<std::string, std::set<Node*>> _def;

public:
    Definition() {}

    Definition(const Definition& def) : _def(def._def) {}

    inline void insert(const std::string& var, Node* node) {
        _def[var].insert(node);
    }

    inline void unionDef(std::shared_ptr<Definition> otherDef) {
        for (auto kv : otherDef->_def) {
            _def[kv.first].insert(kv.second.begin(), kv.second.end());
        }
    }

    inline void clear(const std::string& var) { _def[var].clear(); }

    inline void clear(Node* inst) {
        for (auto kv : _def) {
            _def[kv.first].clear();
            _def[kv.first].insert(inst);
        }
    }

    inline void clear() { _def.clear(); }

    inline bool isEmpty() const { return _def.size() == 0; }

    inline void insertPDGEdge(Node* target) {
        for (auto kv : _def) {
            for (Node* node : kv.second) {
                new PDGEdge(node, target, kv.first);
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

    Definitions _globals;
    Definitions _locals;
    Stack _stack;

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

    inline Index stackSize() const { return _stack.size(); }
};

}  // namespace wasmati

#endif /*end WASMATI_PDG_H*/
