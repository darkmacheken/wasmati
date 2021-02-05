#include "src/interpreter/nodes.h"

namespace wasmati {
bool BlockNode::accept(Visitor* visitor) {
    return visitor->visitBlockNode(this);
}
bool Foreach::accept(Visitor* visitor) {
    return visitor->visitForeach(this);
}
bool WhileNode::accept(Visitor* visitor) {
    return visitor->visitWhileNode(this);
}
bool ContinueNode::accept(Visitor* visitor) {
    return visitor->visitContinueNode(this);
}

bool BreakNode::accept(Visitor* visitor) {
    return visitor->visitBreakNode(this);
}
bool IfElseNode::accept(Visitor* visitor) {
    return visitor->visitIfElseNode(this);
}
bool IfNode::accept(Visitor* visitor) {
    return visitor->visitIfNode(this);
}

bool FunctionNode::accept(Visitor* visitor) {
    return visitor->visitFunction(this);
}

bool ReturnNode::accept(Visitor* visitor) {
    return visitor->visitReturnNode(this);
}
bool ImportNode::accept(Visitor* visitor) {
    return visitor->visitImportNode(this);
}

// basic expressions
bool AssignExpr::accept(Visitor* visitor) {
    return visitor->visitAssignExpr(this);
}
bool RValueNode::accept(Visitor* visitor) {
    return visitor->visitRValueNode(this);
}
bool FunctionCall::accept(Visitor* visitor) {
    return visitor->visitFunctionCall(this);
}
bool AttributeCall::accept(Visitor* visitor) {
    return visitor->visitAttributeCall(this);
}
bool MemberFunctionCall::accept(Visitor* visitor) {
    return visitor->visitMemberFunctionCall(this);
}
bool FilterExprNode::accept(Visitor* visitor) {
    return visitor->visitFilterExprNode(this);
}
bool AtNode::accept(Visitor* visitor) {
    return visitor->visitAtNode(this);
}
bool TimeNode::accept(Visitor* visitor) {
    return visitor->visitTimeNode(this);
}

// objects
template <>
bool NodePointer::accept(Visitor* visitor) {
    return visitor->visitNodePointer(this);
}
template <>
bool EdgePointer::accept(Visitor* visitor) {
    return visitor->visitEdgePointer(this);
}

// literals
template <>
bool IntNode::accept(Visitor* visitor) {
    return visitor->visitIntNode(this);
}
template <>
bool FloatNode::accept(Visitor* visitor) {
    return visitor->visitFloatNode(this);
}
template <>
bool BoolNode::accept(Visitor* visitor) {
    return visitor->visitBoolNode(this);
}
template <>
bool StringNode::accept(Visitor* visitor) {
    return visitor->visitStringNode(this);
}
template <>
bool ListNode::accept(Visitor* visitor) {
    return visitor->visitListNode(this);
}
template <>
bool MapNode::accept(Visitor* visitor) {
    return visitor->visitMapNode(this);
}
template <>
bool NilNode::accept(Visitor* visitor) {
    return visitor->visitNilNode(this);
}
template <>
bool NoneNode::accept(Visitor* visitor) {
    return visitor->visitNoneNode(this);
}

// binary expr
template <>
bool EqualNode::accept(Visitor* visitor) {
    return visitor->visitEqualNode(this);
}
template <>
bool NotEqualNode::accept(Visitor* visitor) {
    return visitor->visitNotEqualNode(this);
}
template <>
bool AndNode::accept(Visitor* visitor) {
    return visitor->visitAndNode(this);
}
template <>
bool OrNode::accept(Visitor* visitor) {
    return visitor->visitOrNode(this);
}
template <>
bool InNode::accept(Visitor* visitor) {
    return visitor->visitInNode(this);
}
template <>
bool LessNode::accept(Visitor* visitor) {
    return visitor->visitLessNode(this);
}
template <>
bool LessEqualNode::accept(Visitor* visitor) {
    return visitor->visitLessEqualNode(this);
}
template <>
bool GreaterNode::accept(Visitor* visitor) {
    return visitor->visitGreaterNode(this);
}
template <>
bool GreaterEqualNode::accept(Visitor* visitor) {
    return visitor->visitGreaterEqualNode(this);
}
template <>
bool AddNode::accept(Visitor* visitor) {
    return visitor->visitAddNode(this);
}
template <>
bool SubNode::accept(Visitor* visitor) {
    return visitor->visitSubNode(this);
}
template <>
bool MulNode::accept(Visitor* visitor) {
    return visitor->visitMulNode(this);
}
template <>
bool DivNode::accept(Visitor* visitor) {
    return visitor->visitDivNode(this);
}
template <>
bool ModNode::accept(Visitor* visitor) {
    return visitor->visitModNode(this);
}

// unary expr
template <>
bool NotNode::accept(Visitor* visitor) {
    return visitor->visitNotNode(this);
}

// Sequences
template <>
bool SequenceNode::accept(Visitor* visitor) {
    return visitor->visitSequenceNode(this);
}
template <>
bool SequenceExprNode::accept(Visitor* visitor) {
    return visitor->visitSequenceExprNode(this);
}
template <>
bool SequenceLiteralNode::accept(Visitor* visitor) {
    return visitor->visitSequenceLiteralNode(this);
}

}  // namespace wasmati
