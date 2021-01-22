#include "src/interpreter/evaluator.h"

namespace wasmati {

const std::map<ExprType, std::string> EXPR_TYPE_MAP = {
#define WASMATI_ENUMS_EXPR_TYPE(type, name) {type, name},
#include "src/config/enums.def"
#undef WASMATI_ENUMS_EXPR_TYPE
};

std::map<
    std::string,
    std::function<std::shared_ptr<LiteralNode>(int, std::shared_ptr<ListNode>)>>
    functionsMap = {
        // attributes
        {"name", Functions::name},
        {"index", Functions::index},
        {"nargs", Functions::nargs},
        {"nlocals", Functions::nlocals},
        {"nresults", Functions::nresults},
        {"isImport", Functions::isImport},
        {"isExport", Functions::isExport},
        {"varType", Functions::varType},
        {"instType", Functions::instType},
        {"opcode", Functions::opcode},
        //{"value", Functions::value},
        {"label", Functions::label},
        {"hasElse", Functions::hasElse},
        {"offset", Functions::offset},
        // functions
        {"functions", Functions::functions},
        {"instructions", Functions::instructions},
        {"child", Functions::child},
        {"PDGEdge", Functions::PDGEdge},
        {"descendantsCFG", Functions::descendantsCFG},
        {"reachesPDG", Functions::reachesPDG},
        {"vulnerability", Functions::vulnerability},

};

}  // namespace wasmati
