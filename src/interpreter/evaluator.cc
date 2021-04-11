#include "src/interpreter/evaluator.h"
#include "src/interpreter/functions.h"

namespace wasmati {

// allocate space for static variable
json Functions::_vulns;

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
          {"value", NodeFunctions::value},
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
          {"value", EdgeFunctions::value},
          {"varType", EdgeFunctions::varType},
      }}};

std::map<LiteralType, std::map<std::string, MemberFunction>>
    memberFunctionsMap = {{LiteralType::List,
                           {
                               {"empty", ListFunctions::empty},
                               {"size", ListFunctions::size},
                               {"append", ListFunctions::append},
                               {"pop_back", ListFunctions::pop_back},
                               {"sort", ListFunctions::sort},
                               {"adjacent_difference", ListFunctions::adjacent_difference},

                           }},
                          {LiteralType::Map,
                           {
                               {"empty", MapFunctions::empty},
                               {"size", MapFunctions::size},
                               {"keys", MapFunctions::keys},
                               {"values", MapFunctions::values},
                               {"insert", MapFunctions::insert},
                           }},
                          {LiteralType::Node,
                           {
                               {"edges", NodeFunctions::edges},
                               {"child", NodeFunctions::child},
                               {"children", NodeFunctions::children},
                           }}};

std::map<std::string, Func> functionsMap = {
    // constructors
    {"List", ListFunctions::List},
    {"Map", MapFunctions::Map},
    // functions
    {"functions", Functions::functions},
    {"parameters", Functions::parameters},
    {"instructions", Functions::instructions},
    {"PDGEdge", Functions::PDGEdge},
    {"ascendantsCFG", Functions::ascendantsCFG},
    {"descendantsCFG", Functions::descendantsCFG},
    {"descendantsAST", Functions::descendantsAST},
    {"reachesPDG", Functions::reachesPDG},
    {"vulnerability", Functions::vulnerability},
    {"print_vulns", Functions::print_vulns},

    {"range", Functions::range},
    {"print", Functions::print},

};

}  // namespace wasmati
