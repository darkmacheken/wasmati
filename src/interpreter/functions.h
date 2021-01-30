#ifndef WASMATI_FUNCTIONS_H
#define WASMATI_FUNCTIONS_H

namespace wasmati {
struct Functions {
    static std::shared_ptr<ListNode> nodeSetToList(int lineno, NodeSet& nodes) {
        auto list = Query::map<std::shared_ptr<LiteralNode>>(
            nodes,
            [](Node* node) { return std::make_shared<NodePointer>(0, node); });
        auto seq = std::make_shared<SequenceLiteralNode>(lineno, list);
        return std::make_shared<ListNode>(lineno, seq);
    }

    static std::shared_ptr<ListNode> edgeSetToList(int lineno, EdgeSet& edges) {
        auto list = Query::map<std::shared_ptr<LiteralNode>>(
            edges, [](Edge* e) { return std::make_shared<EdgePointer>(0, e); });
        auto seq = std::make_shared<SequenceLiteralNode>(lineno, list);
        return std::make_shared<ListNode>(lineno, seq);
    }

    static std::shared_ptr<LiteralNode> functions(
        int lineno,
        std::shared_ptr<ListNode> args) {
        auto nodes = Query::functions();
        return nodeSetToList(lineno, nodes);
    }

    static std::shared_ptr<LiteralNode> instructions(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(args, 1);
        ASSERT_EXPR_TYPE_(args->value()->node(0), LiteralType::Node);
        auto node =
            std::dynamic_pointer_cast<NodePointer>(args->value()->node(0));

        auto nodes = Query::instructions({node->value()}, Query::ALL_NODES);
        return nodeSetToList(lineno, nodes);
    }

    static std::shared_ptr<LiteralNode> PDGEdge(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(args, 3);
        ASSERT_EXPR_TYPE_(args->value()->node(0), LiteralType::Node);
        ASSERT_EXPR_TYPE_(args->value()->node(1), LiteralType::Node);
        ASSERT_EXPR_TYPE_(args->value()->node(2), LiteralType::String);
        auto src =
            std::dynamic_pointer_cast<NodePointer>(args->value()->node(0));
        auto dest =
            std::dynamic_pointer_cast<NodePointer>(args->value()->node(1));
        auto pdgType =
            std::dynamic_pointer_cast<StringNode>(args->value()->node(2));

        auto edge = EdgeStream(src->value()->outEdges(EdgeType::PDG))
                        .filter([&](Edge* e) {
                            return e->dest()->id() == dest->value()->id();
                        })
                        .filterPDG(PDG_TYPE_MAP_R.at(pdgType->value()))
                        .findFirst();

        if (edge.isPresent()) {
            std::make_shared<EdgePointer>(lineno, edge.get());
        }
        return std::make_shared<NilNode>(lineno);
    }

    static std::shared_ptr<LiteralNode> descendantsCFG(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(args, 1);
        ASSERT_EXPR_TYPE_(args->value()->node(0), LiteralType::Node);
        auto node =
            std::dynamic_pointer_cast<NodePointer>(args->value()->node(0));

        auto nodes =
            Query::BFS({node->value()}, Query::ALL_NODES, Query::CFG_EDGES);
        return nodeSetToList(lineno, nodes);
    }

    static std::shared_ptr<LiteralNode> reachesPDG(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(args, 4);
        ASSERT_EXPR_TYPE_(args->value()->node(0), LiteralType::Node);
        ASSERT_EXPR_TYPE_(args->value()->node(1), LiteralType::Node);
        ASSERT_EXPR_TYPE_(args->value()->node(2), LiteralType::String);
        ASSERT_EXPR_TYPE_(args->value()->node(3), LiteralType::String);
        auto src =
            std::dynamic_pointer_cast<NodePointer>(args->value()->node(0));
        auto dest =
            std::dynamic_pointer_cast<NodePointer>(args->value()->node(1));
        auto pdgType =
            std::dynamic_pointer_cast<StringNode>(args->value()->node(2));
        auto label =
            std::dynamic_pointer_cast<StringNode>(args->value()->node(3));

        bool isPresent =
            NodeStream(src->value())
                .BFS(Predicate().id(dest->value()->id()),
                     [&](Edge* e) {
                         return e->type() == EdgeType::PDG &&
                                e->pdgType() ==
                                    PDG_TYPE_MAP_R.at(pdgType->value()) &&
                                e->label() == label->value();
                     })
                .findFirst()
                .isPresent();
        return std::make_shared<BoolNode>(lineno, isPresent);
    }

    static std::shared_ptr<LiteralNode> vulnerability(
        int lineno,
        std::shared_ptr<ListNode> args) {
        ASSERT_NUM_ARGS_(args, 1);
        json vuln;
        Printer printer = Printer(false);
        args->value()->node(0)->accept(&printer);
        vuln["type"] = printer.toString();

        if (args->value()->size() >= 2) {
            args->value()->node(1)->accept(&printer);
            vuln["function"] = printer.toString();
        }
        if (args->value()->size() >= 3) {
            args->value()->node(2)->accept(&printer);
            vuln["caller"] = printer.toString();
        }
        if (args->value()->size() >= 4) {
            args->value()->node(3)->accept(&printer);
            vuln["description"] = printer.toString();
        }
        std::cout << vuln.dump(2) << std::endl;
        return std::make_shared<NilNode>(lineno);
    }
};

struct IntFunctions {
public:
    // Operators
    static std::shared_ptr<LiteralNode> Less(int lineno,
                                             std::shared_ptr<LiteralNode> e1,
                                             std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<IntNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() < b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() < b->value());
        }

        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '<' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " < " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> LessEqual(
        int lineno,
        std::shared_ptr<LiteralNode> e1,
        std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<IntNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() <= b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() <= b->value());
        }

        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '<=' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) +
                " <= " + LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> Greater(
        int lineno,
        std::shared_ptr<LiteralNode> e1,
        std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<IntNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() > b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() > b->value());
        }

        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '>' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " > " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> GreaterEqual(
        int lineno,
        std::shared_ptr<LiteralNode> e1,
        std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<IntNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() >= b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() >= b->value());
        }

        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '>=' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) +
                " >= " + LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> Add(int lineno,
                                            std::shared_ptr<LiteralNode> e1,
                                            std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<IntNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<IntNode>(lineno, a->value() + b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<FloatNode>(lineno, a->value() + b->value());
        }
        case LiteralType::String: {
            auto b = std::dynamic_pointer_cast<StringNode>(e2);
            return std::make_shared<StringNode>(
                lineno, std::to_string(a->value()) + b->value());
        }

        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '+' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " + " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> Sub(int lineno,
                                            std::shared_ptr<LiteralNode> e1,
                                            std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<IntNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<IntNode>(lineno, a->value() - b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<FloatNode>(lineno, a->value() - b->value());
        }
        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '-' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " - " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> Mul(int lineno,
                                            std::shared_ptr<LiteralNode> e1,
                                            std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<IntNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<IntNode>(lineno, a->value() * b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<FloatNode>(lineno, a->value() * b->value());
        }
        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '*' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " * " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> Div(int lineno,
                                            std::shared_ptr<LiteralNode> e1,
                                            std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<IntNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            if (b->value() == 0) {
                throw InterpreterException("Division by 0 in line " +
                                           std::to_string(lineno));
            }
            return std::make_shared<IntNode>(lineno, a->value() / b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            if (b->value() == 0) {
                throw InterpreterException("Division by 0 in line " +
                                           std::to_string(lineno));
            }
            return std::make_shared<FloatNode>(lineno, a->value() / b->value());
        }
        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '/' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " / " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> Mod(int lineno,
                                            std::shared_ptr<LiteralNode> e1,
                                            std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<IntNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            if (b->value() == 0) {
                throw InterpreterException("Modulo by 0 in line " +
                                           std::to_string(lineno));
            }
            return std::make_shared<IntNode>(lineno, a->value() % b->value());
        }
        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '%' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " % " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }
};

struct FloatFunctions {
public:
    // Operators
    static std::shared_ptr<LiteralNode> Less(int lineno,
                                             std::shared_ptr<LiteralNode> e1,
                                             std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<FloatNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() < b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() < b->value());
        }

        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '<' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " < " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> LessEqual(
        int lineno,
        std::shared_ptr<LiteralNode> e1,
        std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<FloatNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() <= b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() <= b->value());
        }

        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '<=' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) +
                " <= " + LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> Greater(
        int lineno,
        std::shared_ptr<LiteralNode> e1,
        std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<FloatNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() > b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() > b->value());
        }

        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '>' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " > " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> GreaterEqual(
        int lineno,
        std::shared_ptr<LiteralNode> e1,
        std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<FloatNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() >= b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<BoolNode>(lineno, a->value() >= b->value());
        }

        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '>=' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) +
                " >= " + LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> Add(int lineno,
                                            std::shared_ptr<LiteralNode> e1,
                                            std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<FloatNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<IntNode>(lineno, a->value() + b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<FloatNode>(lineno, a->value() + b->value());
        }
        case LiteralType::String: {
            auto b = std::dynamic_pointer_cast<StringNode>(e2);
            return std::make_shared<StringNode>(
                lineno, std::to_string(a->value()) + b->value());
        }

        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '+' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " + " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> Sub(int lineno,
                                            std::shared_ptr<LiteralNode> e1,
                                            std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<FloatNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<IntNode>(lineno, a->value() - b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<FloatNode>(lineno, a->value() - b->value());
        }
        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '-' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " - " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> Mul(int lineno,
                                            std::shared_ptr<LiteralNode> e1,
                                            std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<FloatNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            return std::make_shared<IntNode>(lineno, a->value() * b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            return std::make_shared<FloatNode>(lineno, a->value() * b->value());
        }
        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '*' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " * " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> Div(int lineno,
                                            std::shared_ptr<LiteralNode> e1,
                                            std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<FloatNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int: {
            auto b = std::dynamic_pointer_cast<IntNode>(e2);
            if (b->value() == 0) {
                throw InterpreterException("Division by 0 in line " +
                                           std::to_string(lineno));
            }
            return std::make_shared<IntNode>(lineno, a->value() / b->value());
        }
        case LiteralType::Float: {
            auto b = std::dynamic_pointer_cast<FloatNode>(e2);
            if (b->value() == 0) {
                throw InterpreterException("Division by 0 in line " +
                                           std::to_string(lineno));
            }
            return std::make_shared<FloatNode>(lineno, a->value() / b->value());
        }
        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '/' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " / " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }
};

struct StringFunctions {
public:
    // Operators
    static std::shared_ptr<LiteralNode> Add(int lineno,
                                            std::shared_ptr<LiteralNode> e1,
                                            std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<StringNode>(e1);
        switch (e2->type()) {
        case LiteralType::Int:
        case LiteralType::Float:
        case LiteralType::List: {
            Printer printer;
            e2->accept(&printer);
            return std::make_shared<StringNode>(
                lineno, a->value() + printer.toString());
        }
        case LiteralType::String: {
            auto b = std::dynamic_pointer_cast<StringNode>(e2);
            return std::make_shared<StringNode>(lineno,
                                                a->value() + b->value());
        }
        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '+' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " + " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }
};

struct ListFunctions {
public:
    // Constructor
    static std::shared_ptr<LiteralNode> List(int lineno,
                                             std::shared_ptr<ListNode> args) {
        return args;
    }

public:
    // Attributes
    static std::shared_ptr<LiteralNode> last(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto list = std::dynamic_pointer_cast<ListNode>(expr);

        return list->value()->last();
    }

public:
    // Operators
    static std::shared_ptr<LiteralNode> At(int lineno,
                                           std::shared_ptr<LiteralNode> list,
                                           std::shared_ptr<LiteralNode> index) {
        auto l = std::dynamic_pointer_cast<ListNode>(list);
        switch (index->type()) {
        case LiteralType::Int: {
            auto idx = std::dynamic_pointer_cast<IntNode>(index);
            if (static_cast<size_t>(idx->value()) >= l->value()->size()) {
                throw InterpreterException("Index out of bounds in line " +
                                           std::to_string(lineno));
            }
            return l->value()->node(idx->value());
        }
        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(index->type()) +
                " in operator '[]' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(list->type()) + " [" +
                LITERAL_TYPE_MAP.at(index->type()) + "]");
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> In(int lineno,
                                           std::shared_ptr<LiteralNode> list,
                                           std::shared_ptr<LiteralNode> item) {
        auto list_ = std::dynamic_pointer_cast<ListNode>(list);
        for (auto lit : list_->value()->nodes()) {
            if (item->equals(lit)) {
                return std::make_shared<BoolNode>(lineno, true);
            }
        }

        return std::make_shared<BoolNode>(lineno, false);
    }

    static std::shared_ptr<LiteralNode> Add(int lineno,
                                            std::shared_ptr<LiteralNode> e1,
                                            std::shared_ptr<LiteralNode> e2) {
        auto a = std::dynamic_pointer_cast<ListNode>(e1);
        switch (e2->type()) {
        case LiteralType::List: {
            auto b = std::dynamic_pointer_cast<ListNode>(e2);
            auto res = std::make_shared<ListNode>(lineno, a->value());
            res->value()->insert(b->value()->nodes());
            return res;
        }
        case LiteralType::String: {
            auto b = std::dynamic_pointer_cast<StringNode>(e2);
            Printer printer;
            a->accept(&printer);
            return std::make_shared<StringNode>(
                lineno, printer.toString() + b->value());
        }
        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(e2->type()) +
                " in operator '+' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(e1->type()) + " + " +
                LITERAL_TYPE_MAP.at(e2->type()));
            break;
        }

        return NIL(lineno);
    }

public:
    // Member Functions
    static std::shared_ptr<LiteralNode> empty(
        int lineno,
        std::shared_ptr<LiteralNode> expr,
        std::shared_ptr<ListNode> parameters) {
        auto list = std::dynamic_pointer_cast<ListNode>(expr);

        return std::make_shared<BoolNode>(lineno, list->value()->size() == 0);
    }

    static std::shared_ptr<LiteralNode> size(
        int lineno,
        std::shared_ptr<LiteralNode> expr,
        std::shared_ptr<ListNode> parameters) {
        auto list = std::dynamic_pointer_cast<ListNode>(expr);

        return std::make_shared<IntNode>(lineno, list->value()->size());
    }

    static std::shared_ptr<LiteralNode> append(
        int lineno,
        std::shared_ptr<LiteralNode> expr,
        std::shared_ptr<ListNode> parameters) {
        auto list = std::dynamic_pointer_cast<ListNode>(expr);

        for (auto lit : parameters->value()->nodes()) {
            list->value()->insert(lit);
        }

        return expr;
    }
};

struct MapFunctions {
public:
    // Constructor
    static std::shared_ptr<LiteralNode> Map(int lineno,
                                            std::shared_ptr<ListNode> args) {
        return std::make_shared<MapNode>(lineno);
    }

public:
    // Operators
    static std::shared_ptr<LiteralNode> At(int lineno,
                                           std::shared_ptr<LiteralNode> map,
                                           std::shared_ptr<LiteralNode> key) {
        auto map_ = std::dynamic_pointer_cast<MapNode>(map);
        switch (key->type()) {
        case LiteralType::String: {
            auto key_ = std::dynamic_pointer_cast<StringNode>(key);
            if (map_->value().count(key_->value()) == 0) {
                throw InterpreterException("Key not found in line " +
                                           std::to_string(lineno));
            }
            return map_->value().at(key_->value());
        }
        default:
            throw InterpreterException(
                "Unexpected " + LITERAL_TYPE_MAP.at(key->type()) +
                " in operator '[]' in line " + std::to_string(lineno) + ": " +
                LITERAL_TYPE_MAP.at(map->type()) + " [" +
                LITERAL_TYPE_MAP.at(key->type()) + "]");
            break;
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> In(int lineno,
                                           std::shared_ptr<LiteralNode> map,
                                           std::shared_ptr<LiteralNode> key) {
        auto map_ = std::dynamic_pointer_cast<MapNode>(map);
        ASSERT_EXPR_TYPE_(key, LiteralType::String);
        auto key_ = std::dynamic_pointer_cast<StringNode>(key);
        return std::make_shared<BoolNode>(
            lineno, map_->value().count(key_->value()) == 1);
    }

public:
    // Member Functions
    static std::shared_ptr<LiteralNode> empty(
        int lineno,
        std::shared_ptr<LiteralNode> map,
        std::shared_ptr<ListNode> parameters) {
        auto map_ = std::dynamic_pointer_cast<MapNode>(map);

        return std::make_shared<BoolNode>(lineno, map_->value().size() == 0);
    }

    static std::shared_ptr<LiteralNode> size(
        int lineno,
        std::shared_ptr<LiteralNode> map,
        std::shared_ptr<ListNode> parameters) {
        auto map_ = std::dynamic_pointer_cast<MapNode>(map);

        return std::make_shared<IntNode>(lineno, map_->value().size());
    }

    static std::shared_ptr<LiteralNode> insert(
        int lineno,
        std::shared_ptr<LiteralNode> map,
        std::shared_ptr<ListNode> parameters) {
        auto map_ = std::dynamic_pointer_cast<MapNode>(map);
        ASSERT_NUM_ARGS_(parameters, 2);
        ASSERT_EXPR_TYPE_(parameters->value()->node(0), LiteralType::String);
        auto key =
            std::dynamic_pointer_cast<StringNode>(parameters->value()->node(0));

        map_->value()[key->value()] = parameters->value()->node(1);
        return std::make_shared<BoolNode>(lineno, true);
    }
};

struct NodeFunctions {
public:
    // Attributes
    static std::shared_ptr<LiteralNode> type(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<StringNode>(
            lineno, NODE_TYPE_MAP.at(node->value()->type()));
    }

    static std::shared_ptr<LiteralNode> name(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<StringNode>(lineno, node->value()->name());
    }

    static std::shared_ptr<LiteralNode> index(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<IntNode>(lineno, node->value()->index());
    }

    static std::shared_ptr<LiteralNode> nargs(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<IntNode>(lineno, node->value()->nargs());
    }

    static std::shared_ptr<LiteralNode> nlocals(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<IntNode>(lineno, node->value()->nlocals());
    }

    static std::shared_ptr<LiteralNode> nresults(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<IntNode>(lineno, node->value()->nresults());
    }

    static std::shared_ptr<LiteralNode> isImport(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<BoolNode>(lineno, node->value()->isImport());
    }

    static std::shared_ptr<LiteralNode> isExport(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<BoolNode>(lineno, node->value()->isExport());
    }

    static std::shared_ptr<LiteralNode> varType(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<StringNode>(
            lineno, Utils::writeConstType(node->value()->varType()));
    }

    static std::shared_ptr<LiteralNode> opcode(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<StringNode>(lineno,
                                            node->value()->opcode().GetName());
    }

    static std::shared_ptr<LiteralNode> hasElse(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<BoolNode>(lineno, node->value()->hasElse());
    }

    static std::shared_ptr<LiteralNode> offset(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<IntNode>(lineno, node->value()->offset());
    }

    static std::shared_ptr<LiteralNode> instType(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<StringNode>(
            lineno, INST_TYPE_MAP.at(node->value()->instType()));
    }

    static std::shared_ptr<LiteralNode> label(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);

        return std::make_shared<StringNode>(lineno, node->value()->label());
    }

    static std::shared_ptr<LiteralNode> inEdges(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);
        auto edges = node->value()->inEdges();
        return Functions::edgeSetToList(lineno, edges);
    }

    static std::shared_ptr<LiteralNode> outEdges(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);
        auto edges = node->value()->outEdges();
        return Functions::edgeSetToList(lineno, edges);
    }

public:
    // Member functions
    static std::shared_ptr<LiteralNode> child(
        int lineno,
        std::shared_ptr<LiteralNode> expr,
        std::shared_ptr<ListNode> parameters) {
        ASSERT_NUM_ARGS_(parameters, 2);
        ASSERT_EXPR_TYPE_(parameters->value()->node(0), LiteralType::Int);
        ASSERT_EXPR_TYPE_(parameters->value()->node(1), LiteralType::String);
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);
        auto i =
            std::dynamic_pointer_cast<IntNode>(parameters->value()->node(0));
        auto edgeType =
            std::dynamic_pointer_cast<StringNode>(parameters->value()->node(1));

        auto child =
            NodeStream(node->value())
                .child(i->value(), EDGE_TYPES_MAP_R.at(edgeType->value()))
                .findFirst();

        if (child.isPresent()) {
            return std::make_shared<NodePointer>(lineno, child.get());
        }

        return NIL(lineno);
    }

    static std::shared_ptr<LiteralNode> children(
        int lineno,
        std::shared_ptr<LiteralNode> expr,
        std::shared_ptr<ListNode> parameters) {
        ASSERT_NUM_ARGS_(parameters, 1);
        ASSERT_EXPR_TYPE_(parameters->value()->node(0), LiteralType::String);
        auto node = std::dynamic_pointer_cast<NodePointer>(expr);
        auto edgeType =
            std::dynamic_pointer_cast<StringNode>(parameters->value()->node(0));

        auto children =
            NodeStream(node->value())
                .children([&](Edge* e) {
                    return EDGE_TYPES_MAP.at(e->type()) == edgeType->value();
                })
                .toNodeSet();

        return Functions::nodeSetToList(lineno, children);
    }
};

struct EdgeFunctions {
public:
    // Attributes
    static std::shared_ptr<LiteralNode> type(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto edge = std::dynamic_pointer_cast<EdgePointer>(expr);

        return std::make_shared<StringNode>(
            lineno, EDGE_TYPES_MAP.at(edge->value()->type()));
    }

    static std::shared_ptr<LiteralNode> src(int lineno,
                                            std::shared_ptr<LiteralNode> expr) {
        auto edge = std::dynamic_pointer_cast<EdgePointer>(expr);

        return std::make_shared<NodePointer>(lineno, edge->value()->src());
    }

    static std::shared_ptr<LiteralNode> dest(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto edge = std::dynamic_pointer_cast<EdgePointer>(expr);

        return std::make_shared<NodePointer>(lineno, edge->value()->dest());
    }

    static std::shared_ptr<LiteralNode> label(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto edge = std::dynamic_pointer_cast<EdgePointer>(expr);

        return std::make_shared<StringNode>(lineno, edge->value()->label());
    }

    static std::shared_ptr<LiteralNode> pdgType(
        int lineno,
        std::shared_ptr<LiteralNode> expr) {
        auto edge = std::dynamic_pointer_cast<EdgePointer>(expr);

        return std::make_shared<StringNode>(
            lineno, PDG_TYPE_MAP.at(edge->value()->pdgType()));
    }
};
}  // namespace wasmati
#endif
