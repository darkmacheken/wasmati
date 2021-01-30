#include "src/interpreter/evaluator.h"
#include "src/interpreter/functions.h"

namespace wasmati {

const std::map<LiteralType, std::string> LITERAL_TYPE_MAP = {
#define WASMATI_ENUMS_LITERAL_TYPE(type, name) {type, name},
#include "src/config/enums.def"
#undef WASMATI_ENUMS_LITERAL_TYPE
};

const std::map<BinopType, std::string> BINARY_TYPE_MAP = {
#define WASMATI_ENUMS_BINARY_TYPE(type, name) {type, name},
#include "src/config/enums.def"
#undef WASMATI_ENUMS_BINARY_TYPE
};

const std::map<UnopType, std::string> UNARY_TYPE_MAP = {
#define WASMATI_ENUMS_UNARY_TYPE(type, name) {type, name},
#include "src/config/enums.def"
#undef WASMATI_ENUMS_UNARY_TYPE
};

std::map<LiteralType, std::map<std::string, OperatorsFunction>> operatorsMap = {
    {LiteralType::Int,
     {
         {"<", IntFunctions::Less},
         {"<=", IntFunctions::LessEqual},
         {">", IntFunctions::Greater},
         {">=", IntFunctions::GreaterEqual},
         {"+", IntFunctions::Add},
         {"-", IntFunctions::Sub},
         {"*", IntFunctions::Mul},
         {"/", IntFunctions::Div},
         {"%", IntFunctions::Mod},
     }},
    {LiteralType::Float,
     {
         {"<", FloatFunctions::Less},
         {"<=", FloatFunctions::LessEqual},
         {">", FloatFunctions::Greater},
         {">=", FloatFunctions::GreaterEqual},
         {"+", FloatFunctions::Add},
         {"-", FloatFunctions::Sub},
         {"*", FloatFunctions::Mul},
         {"/", FloatFunctions::Div},
     }},
    {LiteralType::List,
     {
         {"[]", ListFunctions::At},
         {"in", ListFunctions::In},
         {"+", ListFunctions::Add},
     }},
    {LiteralType::Map,
     {
         {"[]", MapFunctions::At},
         {"in", MapFunctions::In},
     }},
    {LiteralType::String,
     {
         {"+", StringFunctions::Add},
     }}};

std::map<LiteralType, std::map<std::string, AttributeFunction>> attributesMap =
    {{LiteralType::List,
      {
          {"last", ListFunctions::last},
      }},
     {LiteralType::Node,
      {
          {"type", NodeFunctions::type},
          {"name", NodeFunctions::name},
          {"index", NodeFunctions::index},
          {"nargs", NodeFunctions::nargs},
          {"nlocals", NodeFunctions::nlocals},
          {"nresults", NodeFunctions::nresults},
          {"isImport", NodeFunctions::isImport},
          {"isExport", NodeFunctions::isExport},
          {"varType", NodeFunctions::varType},
          {"instType", NodeFunctions::instType},
          {"opcode", NodeFunctions::opcode},
          //{"value", NodeFunctions::value},
          {"label", NodeFunctions::label},
          {"hasElse", NodeFunctions::hasElse},
          {"offset", NodeFunctions::offset},
          {"inEdges", NodeFunctions::inEdges},
          {"outEdges", NodeFunctions::outEdges},
      }},
     {LiteralType::Edge,
      {
          {"type", EdgeFunctions::type},
          {"label", EdgeFunctions::label},
          {"src", EdgeFunctions::src},
          {"dest", EdgeFunctions::dest},
          {"pdgType", EdgeFunctions::pdgType},
      }}};

std::map<LiteralType, std::map<std::string, MemberFunction>>
    memberFunctionsMap = {{LiteralType::List,
                           {
                               {"empty", ListFunctions::empty},
                               {"size", ListFunctions::size},
                               {"append", ListFunctions::append},
                           }},
                          {LiteralType::Map,
                           {
                               {"empty", MapFunctions::empty},
                               {"size", MapFunctions::size},
                               {"insert", MapFunctions::insert},
                           }},
                          {LiteralType::Node,
                           {
                               {"child", NodeFunctions::child},
                               {"children", NodeFunctions::children},
                           }}};

std::map<std::string, Func> functionsMap = {
    // constructors
    {"List", ListFunctions::List},
    {"Map", MapFunctions::Map},
    // functions
    {"functions", Functions::functions},
    {"instructions", Functions::instructions},
    {"PDGEdge", Functions::PDGEdge},
    {"descendantsCFG", Functions::descendantsCFG},
    {"reachesPDG", Functions::reachesPDG},
    {"vulnerability", Functions::vulnerability},

};

}  // namespace wasmati
