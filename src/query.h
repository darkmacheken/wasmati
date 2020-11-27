#ifndef WASMATI_QUERY_BUILDER_H_
#define WASMATI_QUERY_BUILDER_H_

#include <list>
#include <stack>
#include "graph.h"
#include "include/nlohmann/json.hpp"

using nlohmann::json;

namespace wasmati {
typedef std::function<bool(Node*)> NodeCondition;
typedef std::function<bool(Edge*)> EdgeCondition;
class Predicate;

class Query {
    static const Graph* _graph;
    static NodeSet emptyNodeSet;

public:
    static NodeSet& getEmptyNodeSet() {
        emptyNodeSet.clear();
        return emptyNodeSet;
    }

    static void setGraph(const Graph* graph) {
        assert(graph != nullptr);
        _graph = graph;
    }

public:
    static const Predicate& TRUE_PREDICATE;
    /// @brief Condition to return all edges
    static const EdgeCondition& ALL_EDGES;
    /// @brief Condition to return only AST edges
    static const EdgeCondition& AST_EDGES;
    /// @brief Condition to return only CFG edges
    static const EdgeCondition& CFG_EDGES;
    /// @brief Condition to return only PDG edges
    static const EdgeCondition& PDG_EDGES;
    /// @brief Condition to return only CG edges
    static const EdgeCondition& CG_EDGES;
    /// @brief Condition to return only PG edges
    static const EdgeCondition& PG_EDGES;
    /// @brief Condition to return all inst nodes
    static const NodeCondition& ALL_INSTS;
    /// @brief Condition to return all nodes
    static const NodeCondition& ALL_NODES;

    /// @brief Returns the childrens of the given nodes.
    /// @param nodes Set of nodes
    /// @param edgeCondition Edge condition to be taken.
    /// @return A set of nodes that are the children.
    static NodeSet children(const NodeSet& nodes,
                            const EdgeCondition& edgeCondition = ALL_EDGES);

    /// @brief Returns the parents of the given nodes.
    /// @param nodes Set of nodes
    /// @param edgeCondition Edge condition to be taken.
    /// @return A set of nodes that are the parents.
    static NodeSet parents(const NodeSet& nodes,
                           const EdgeCondition& edgeCondition = ALL_EDGES);

    /// @brief Given a set of Edges, returns the edges that satisfies the
    /// given condition.
    /// @param edges Set of edges.
    /// @param edgeCondition Edge condition to be present in the result.
    /// @return Set of edges
    static EdgeSet filterEdges(const EdgeSet& edges,
                               const EdgeCondition& edgeCondition = ALL_EDGES);

    /// @brief Filter the given nodes and return those that satisfies the given
    /// nodeCondition
    /// @param nodes Set of nodes
    /// @param cond Node condition.
    /// @return Set of nodes filtered
#define WASMATI_EVALUATION(type, var, eval, rALL) \
    static NodeSet filter(const NodeSet& nodes, const type& var);
#include "src/config/predicates.def"
#undef WASMATI_EVALUATION

    /// @brief Returns true if there is a node that satisfies the condition
    /// nodeCondition and false otehrwise
    /// @param nodes Set of nodes
    /// @param cond  Node condition
    /// @return true if the set contains a node that satisfies the given
    /// condition
#define WASMATI_EVALUATION(type, var, eval, rALL) \
    static bool contains(const NodeSet& nodes, const type& var);
#include "src/config/predicates.def"
#undef WASMATI_EVALUATION

    /// @brief Returns true if there is an edge that satisfies the condition
    /// edgeCondition and false otherwise
    /// @param edges Set of edges
    /// @param edgeCondition Edgte condition
    /// @return true if the set contains an edge that satisfies the given
    /// condition
    static bool containsEdge(const EdgeSet& edges,
                             const EdgeCondition& edgeCondition);

    /// @brief Creates a new NodeSet with the results of calling a function for
    /// every node element.
    /// @param nodes Set of nodes
    /// @param func Function to call to each element.
    /// @return A NodeSet with the results of calling a function for every node
    /// element.
    static NodeSet map(const NodeSet& nodes,
                       const std::function<Node*(Node*)> func);

    /// @brief  Creates a new NodeSet with the results of calling a function for
    /// every node element
    /// @param nodes Set of nodes
    /// @param func Function to call to each element.
    /// @return A NodeSet with the results of calling a function for every node
    /// element.
    static NodeSet map(const NodeSet& nodes,
                       const std::function<NodeSet(Node*)> func);

    /// @brief Creates a new NodeSet with the results of calling a function for
    /// every node element.
    /// @tparam T Type of return
    /// @param nodes Set of nodes
    /// @param func Function to call to each element.
    /// @return A set of type T  with the results of calling a function for
    /// every node element.
    template <class T>
    static std::list<T> map(const NodeSet& nodes,
                            const std::function<T(Node*)> func) {
        std::list<T> result;
        for (Node* node : nodes) {
            result.push_back(func(node));
        }
        return result;
    }

    /// @brief Creates a new NodeSet with the results of calling a function for
    /// every node element.
    /// @tparam T Type of return
    /// @param nodes Set of nodes
    /// @param func Function to call to each element.
    /// @return A set of type T  with the results of calling a function for
    /// every node element.
    template <class T>
    static std::list<T> map(const NodeSet& nodes,
                            const std::function<std::list<T>(Node*)> func) {
        std::list<T> result;
        for (Node* node : nodes) {
            result.splice(result.begin(), func(node));
        }
        return result;
    }

    /// @brief Creates a new set of type T with the results of calling a
    /// function for every edge element
    /// @tparam T Type of return
    /// @param edges Set of edges
    /// @param func Function to call to each element
    /// @return A set of Type T with the results of calling a function for every
    /// edge element
    template <class T>
    static std::list<T> map(const EdgeSet& edges,
                            const std::function<T(Edge*)> func) {
        std::list<T> result;
        for (Edge* e : edges) {
            result.push_back(func(e));
        }
        return result;
    }

    /// @brief Makes a classic BFS alongside the edges that satisfies the
    /// condition edgeCondition, and returns the nodes that satisfies the
    /// condition nodeCondition
    /// @param nodes Set of nodes
    /// @param cond Node Condition to be present in the result.
    /// @param edgeCondition Edge Condition to be taken.
    /// @param limit The maximum number of elements in the result set.
    /// @param reverse If true, performs a backward BFS
    /// @return Set of nodes
#define WASMATI_EVALUATION(type, var, eval, rALL)                      \
    static NodeSet BFS(const NodeSet& nodes, const type& var = rALL,   \
                       const EdgeCondition& edgeCondition = ALL_EDGES, \
                       Index limit = UINT32_MAX, bool reverse = false);
#include "src/config/predicates.def"
#undef WASMATI_EVALUATION

    /// @brief Makes a classic BFS alongside the edges that satisfies the
    /// condition edgeCondition, and returns the nodes that satisfies the
    /// condition nodeCondition including the begining nodes.
    /// @param nodes Set of nodes
    /// @param nodeCondition Node Condition to be present in the result.
    /// @param edgeCondition Edge Condition to be taken.
    /// @param limit The maximum number os elements in the result set.
    /// @param reverse If true, performs a backward BFS
    /// @return Set of nodes
#define WASMATI_EVALUATION(type, var, eval, rALL)                              \
    static NodeSet BFSincludes(const NodeSet& nodes, const type& var = rALL,   \
                               const EdgeCondition& edgeCondition = ALL_EDGES, \
                               Index limit = UINT32_MAX,                       \
                               bool reverse = false);
#include "src/config/predicates.def"
#undef WASMATI_EVALUATION

    /// @brief Performs a DFS start at node source
    /// @tparam T return type of the consumer
    /// @param source Source node
    /// @param edgeCondition Edge condition to be taken
    /// @param aux Auxiliary type for the consumer
    /// @param consumer Function to consume
    template <class T>
    static void DFS(
        Node* source,
        const EdgeCondition& edgeCondition,
        T aux,
        const std::function<std::pair<bool, T>(Node*, T)> consumer) {
        std::list<std::pair<Node*, T>> dfsNodes;
        std::set<Node*> visited;

        dfsNodes.emplace_front(source, aux);
        while (!dfsNodes.empty()) {
            auto firstNode = dfsNodes.front();
            dfsNodes.pop_front();

            std::pair<bool, T> nextAux =
                consumer(firstNode.first, firstNode.second);

            if (!nextAux.first) {
                continue;
            }

            visited.insert(firstNode.first);
            std::list<std::pair<Node*, T>> nodesToInsert;
            for (Edge* e : Query::filterEdges(firstNode.first->outEdges(),
                                              edgeCondition)) {
                if (visited.count(e->dest()) == 0) {
                    nodesToInsert.emplace_back(e->dest(), nextAux.second);
                }
            }
            dfsNodes.splice(dfsNodes.begin(), nodesToInsert);
        }
    }

    /// @brief Returns the module of the graph
    /// @return A set cointaining the node module
    static NodeSet module();

    /// @brief Returns all the function nodes that satisfies teh condition
    /// nodeCondition
    /// @param nodeCondition Node condition to be present in the result.
    /// @return Set of nodes containing function nodes.
    static NodeSet functions(const NodeCondition& nodeCondition = ALL_NODES);

    static Node* function(Node* node);

    /// @brief Returns all the instructions of the given functions that
    /// satisfies the nodeCondition
    /// @param nodes A set of function nodes.
    /// @param nodeCondition Condition to filter the nodes
    /// @return A set of instructions from the given functions that satisfies
    /// the condition
#define WASMATI_EVALUATION(type, var, eval, rALL) \
    static NodeSet instructions(const NodeSet& nodes, const type& var = rALL);
#include "src/config/predicates.def"
#undef WASMATI_EVALUATION

    /// @brief Returns all the parameters nodes of the given functions that
    /// satisfies the nodeCondition
    /// @param nodes A set of function nodes.
    /// @param nodeCondition Condition to filter the nodes.
    /// @return A set of parameters nodes from the given functions that
    /// satisfies the condition.
    static NodeSet parameters(const NodeSet& nodes,
                              const NodeCondition& nodeCondition = ALL_NODES);
};

// This class is to store more complex queries.
struct Queries {
    static NodeSet loopsInsts(std::string& loopName);
};

class Predicate {
private:
    struct SimplePredicate {
        virtual bool evaluate(Node* node) const = 0;
    };

    struct TruePredicate : SimplePredicate {
        bool evaluate(Node* node) const override { return true; }
    };

    struct TestHolder : SimplePredicate {
        std::function<bool(Node* node)> func;

    public:
        TestHolder(std::function<bool(Node*)> func) : func(func) {}

        bool evaluate(Node* node) const override { return func(node); }
    };

private:
    std::vector<std::vector<std::shared_ptr<SimplePredicate>>> _predicates;
    /// @brief Each row is a vector of SimplePredicates. The evaluation of a row
    /// is the AND between them. The evaluation between rows is an OR.
    Index currentRow = 0;

private:
    void insertPredicate(std::shared_ptr<SimplePredicate> predicate) {
        assert(currentRow + 1 == _predicates.size());
        _predicates[currentRow].emplace_back(predicate);
    }

    bool evaluateRow(
        Node* node,
        const std::vector<std::shared_ptr<SimplePredicate>>& predicates) const {
        if (predicates.size() == 0) {
            return false;
        }
        bool res = true;
        Index i = 0;
        while (res && i < predicates.size()) {
            res = res && predicates[i]->evaluate(node);
            i++;
        }
        return res;
    }

public:
    Predicate() { _predicates.emplace_back(); }

    bool evaluate(Node* node) const {
        bool res = false;
        Index i = 0;
        while (!res && i < _predicates.size()) {
            res = res || evaluateRow(node, _predicates[i]);
            i++;
        }
        return res;
    }

    inline Predicate& insert(std::function<bool(Node* node)> f) {
        insertPredicate(std::make_shared<TestHolder>(f));
        return *this;
    }

    Predicate& truePredicate() {
        insertPredicate(std::make_shared<TruePredicate>());
        return *this;
    }

#define WASMATI_PREDICATE(funcName, TypeVal)                                  \
    Predicate& funcName(TypeVal val, bool eq = true) {                        \
        auto f = [=](Node* node) { return (node->funcName() == val) == eq; }; \
        insert(f);                                                            \
        return *this;                                                         \
    }

    Predicate& value(Type val, bool eq = true) {
        auto f = [&](Node* node) { return (node->value().type == val) == eq; };
        insert(f);
        return *this;
    }

#define WASMATI_PREDICATE_VALUES_I(funcName, valType, field, rtype) \
    Predicate& value##funcName(valType& val) {                      \
        auto f = [&](Node* node) {                                  \
            if (node->value().type == rtype) {                      \
                val = node->value().field;                          \
                return true;                                        \
            }                                                       \
            return false;                                           \
        };                                                          \
        insert(f);                                                  \
        return *this;                                               \
    }

#define WASMATI_PREDICATE_VALUES_F(funcName, valType, field, rtype) \
    Predicate& value##funcName(valType& val) {                      \
        auto f = [&](Node* node) {                                  \
            if (node->value().type == rtype) {                      \
                valType fval;                                       \
                memcpy(&fval, &node->value().field, sizeof(fval));  \
                val = fval;                                         \
                return true;                                        \
            }                                                       \
            return false;                                           \
        };                                                          \
        insert(f);                                                  \
        return *this;                                               \
    }
#include "src/config/predicates.def"
#undef WASMATI_PREDICATE
#undef WASMATI_PREDICATE_VALUES_I
#undef WASMATI_PREDICATE_VALUES_F

    Predicate& test(std::function<bool(Node* node)> f) {
        insert(f);
        return *this;
    }

#define TEST(expr) test([&](Node* node) { return expr; })

    Predicate& execute(std::function<bool(Node* node)> f) {
        insert(f);
        return *this;
    }

#define EXEC(expr)            \
    execute([&](Node* node) { \
        expr;                 \
        return true;          \
    })

#define GET_MACRO(_1, _2, _3, _4, _5, NAME, ...) NAME
#define EDGE(...) GET_MACRO(__VA_ARGS__, EDGE5, EDGE4)(__VA_ARGS__)
#define EDGE4(src, dest, val, eq)                        \
    insert([&](Node* node) {                             \
        auto edges = src->outEdges();                    \
        for (Edge * e : edges) {                         \
            if (e->dest() == dest && e->type() == val) { \
                return true == eq;                       \
            }                                            \
        }                                                \
        return false == eq;                              \
    })

#define EDGE5(src, dest, val, label, eq)                    \
    insert([&](Node* node) {                                \
        auto edges = src->inEdges(val);                     \
        for (auto e : edges) {                              \
            if (e->dest() == dest && e->label() == label) { \
                return true == eq;                          \
            }                                               \
        }                                                   \
        return false == eq;                                 \
    })

#define PDG_EDGE(...) GET_MACRO(__VA_ARGS__, PDG_EDGE5, PDG_EDGE4)(__VA_ARGS__)

#define PDG_EDGE4(SRC, DEST, PDG_TYPE, EQ)                       \
    insert([&](Node* node) {                                     \
        assert(SRC != nullptr);                                  \
        auto edges = SRC->outEdges(EdgeType::PDG);               \
        for (auto e : edges) {                                   \
            if (e->dest() == DEST && e->pdgType() == PDG_TYPE) { \
                return true == EQ;                               \
            }                                                    \
        }                                                        \
        return false == EQ;                                      \
    })

#define PDG_EDGE5(SRC, DEST, LABEL, PDG_TYPE, EQ)           \
    insert([&, PDG_TYPE, EQ](Node* node) {                  \
        assert(SRC != nullptr);                             \
        auto edges = SRC->outEdges(EdgeType::PDG);          \
        for (auto e : edges) {                              \
            if (e->dest() == DEST && e->label() == LABEL && \
                e->pdgType() == PDG_TYPE) {                 \
                return true == EQ;                          \
            }                                               \
        }                                                   \
        return false == EQ;                                 \
    })

    Predicate& inEdge(EdgeType val, bool eq = true) {
        auto f = [=](Node* node) {
            return (node->inEdges(val).size() > 0) == eq;
        };
        insert(f);
        return *this;
    }

    Predicate& inEdge(EdgeType val, std::string label, bool eq = true) {
        auto f = [=](Node* node) {
            auto edges = node->inEdges(val);
            for (auto e : edges) {
                if (e->label() == label) {
                    return true == eq;
                }
            }
            return false == eq;
        };
        insert(f);
        return *this;
    }

    Predicate& inPDGEdge(PDGType pdgType, bool eq = true) {
        auto f = [=](Node* node) {
            auto edges = node->inEdges(EdgeType::PDG);
            for (auto e : edges) {
                if (e->pdgType() == pdgType) {
                    return true == eq;
                }
            }
            return false == eq;
        };
        insert(f);
        return *this;
    }

    Predicate& inPDGEdge(std::string label, PDGType pdgType, bool eq = true) {
        auto f = [=](Node* node) {
            auto edges = node->inEdges(EdgeType::PDG);
            for (auto e : edges) {
                if (e->label() == label && e->pdgType() == pdgType) {
                    return true == eq;
                }
            }
            return false == eq;
        };
        insert(f);
        return *this;
    }

#define WASMATI_PREDICATE_VALUES_I(funcName, valType, field, rtype) \
    Predicate& inPDGConstEdge##funcName(valType& val) {             \
        auto f = [&](Node* node) {                                  \
            auto edges = node->inEdges(EdgeType::PDG);              \
            for (auto e : edges) {                                  \
                if (e->pdgType() == PDGType::Const &&               \
                    node->value().type == rtype) {                  \
                    val = node->value().field;                      \
                    return true;                                    \
                }                                                   \
            }                                                       \
            return false;                                           \
        };                                                          \
        insert(f);                                                  \
        return *this;                                               \
    }

#define WASMATI_PREDICATE_VALUES_F(funcName, valType, field, rtype)    \
    Predicate& inPDGConstEdge##funcName(valType& val) {                \
        auto f = [&](Node* node) {                                     \
            auto edges = node->inEdges(EdgeType::PDG);                 \
            for (auto e : edges) {                                     \
                if (e->pdgType() == PDGType::Const &&                  \
                    node->value().type == rtype) {                     \
                    valType fval;                                      \
                    memcpy(&fval, &node->value().field, sizeof(fval)); \
                    val = fval;                                        \
                    return true;                                       \
                }                                                      \
            }                                                          \
            return false;                                              \
        };                                                             \
        insert(f);                                                     \
        return *this;                                                  \
    }
#include "src/config/predicates.def"
#undef WASMATI_PREDICATE_VALUES_I
#undef WASMATI_PREDICATE_VALUES_F

};  // namespace wasmati

template <class T>
class Optional {
    const T value;
    bool _isPresent;

public:
    Optional() : value(T()), _isPresent(false) {}
    Optional(const T& value) : value(value), _isPresent(true) {}

    T get() {
        assert(_isPresent);
        return value;
    }

    bool isPresent() { return _isPresent; }

    T orElse(T other) {
        if (_isPresent) {
            return value;
        } else {
            return other;
        }
    }

    T orElseGet(std::function<T()> func) {
        if (_isPresent) {
            return value;
        } else {
            return func();
        }
    }
};

class EdgeStream {
    EdgeSet edges;

public:
    EdgeStream(EdgeSet edges) : edges(edges) {}

    size_t size() { return edges.size(); }

    EdgeStream& filter(const EdgeCondition& edgeCondition = Query::ALL_EDGES) {
        edges = Query::filterEdges(edges, edgeCondition);
        return *this;
    }

    EdgeStream& filterPDG(PDGType type, std::string label = "") {
        edges = Query::filterEdges(edges, [&](Edge* e) {
            if (type == PDGType::Const || label == "") {
                return e->type() == EdgeType::PDG && e->pdgType() == type;
            }
            return e->type() == EdgeType::PDG && e->pdgType() == type &&
                   e->label() == label;
        });
        return *this;
    }

    EdgeStream& setUnion(EdgeSet b) {
        edges.insert(b.begin(), b.end());
        return *this;
    }

    EdgeStream& distincLabel() {
        std::set<std::string> labels;
        EdgeSet result;
        for (Edge* e : edges) {
            if (labels.count(e->label()) == 0) {
                result.insert(e);
                labels.insert(e->label());
            }
        }
        edges = result;
        return *this;
    }

    bool contains(const EdgeCondition& edgeCondition) {
        return Query::containsEdge(edges, edgeCondition);
    }

    bool containsPDG(PDGType type, std::string label = "") {
        return Query::containsEdge(edges, [&](Edge* e) {
            if (type == PDGType::Const || label == "") {
                return e->type() == EdgeType::PDG && e->pdgType() == type;
            }
            return e->type() == EdgeType::PDG && e->pdgType() == type &&
                   e->label() == label;
        });
    }

    template <class T>
    std::list<T> map(const std::function<T(Edge*)> func) {
        return Query::map<T>(edges, func);
    }

    Optional<Edge*> findFirst() {
        if (edges.size() > 0) {
            return Optional<Edge*>(*edges.begin());
        } else {
            return Optional<Edge*>();
        }
    }
};

class NodeStream {
    NodeSet nodes;

public:
    NodeStream(NodeSet nodes) : nodes(nodes) {}
    NodeStream(Node* node) {
        nodes.clear();
        nodes.insert(node);
    }

    NodeStream& children(
        const EdgeCondition& edgeCondition = Query::ALL_EDGES) {
        nodes = Query::children(nodes, edgeCondition);
        return *this;
    }

    NodeStream& parents(const EdgeCondition& edgeCondition = Query::ALL_EDGES) {
        nodes = Query::parents(nodes, edgeCondition);
        return *this;
    }

    NodeStream& filter(const NodeCondition& nodeCondition) {
        nodes = Query::filter(nodes, nodeCondition);
        return *this;
    }

    bool contains(const NodeCondition& nodeCondition) {
        return Query::contains(nodes, nodeCondition);
    }

    Optional<Node*> findFirst() {
        if (nodes.size() > 0) {
            return Optional<Node*>(*nodes.begin());
        } else {
            return Optional<Node*>();
        }
    }

    size_t size() { return nodes.size(); }

    NodeStream& map(const std::function<Node*(Node*)> func) {
        nodes = Query::map(nodes, func);
        return *this;
    }

    NodeStream& map(const std::function<NodeSet(Node*)> func) {
        nodes = Query::map(nodes, func);
        return *this;
    }

    template <class T>
    std::set<T> map(const std::function<T(Node*)> func) {
        return Query::map<T>(nodes, func);
    }

    NodeStream& BFS(const NodeCondition& nodeCondition = Query::ALL_NODES,
                    const EdgeCondition& edgeCondition = Query::ALL_EDGES,
                    Index limit = UINT32_MAX,
                    bool reverse = false) {
        nodes = Query::BFS(nodes, nodeCondition, edgeCondition, limit, reverse);
        return *this;
    }

    NodeStream& BFSincludes(
        const NodeCondition& nodeCondition = Query::ALL_NODES,
        const EdgeCondition& edgeCondition = Query::ALL_EDGES,
        Index limit = UINT32_MAX,
        bool reverse = false) {
        nodes = Query::BFSincludes(nodes, nodeCondition, edgeCondition, limit,
                                   reverse);
        return *this;
    }

    NodeStream& instructions(
        const NodeCondition& nodeCondition = Query::ALL_NODES) {
        nodes = Query::instructions(nodes, nodeCondition);
        return *this;
    }

    NodeStream& parameters(
        const NodeCondition& nodeCondition = Query::ALL_NODES) {
        nodes = Query::parameters(nodes, nodeCondition);
        return *this;
    }

    NodeSet toNodeSet() { return nodes; }

    void forEach(std::function<void(Node*)> func) {
        for (auto node : nodes) {
            func(node);
        }
    }
};
}  // namespace wasmati
#endif  // WABT_AST_BUILDER_H_
