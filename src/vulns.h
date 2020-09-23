#ifndef WASMATI_VULNS_BUILDER_H_
#define WASMATI_VULNS_BUILDER_H_

//#include "include/nlohmann/json.hpp"
#include <map>
#include "query.h"

// using json = nlohmann::json;
//
namespace wasmati {
//
// static const json defaultConfig = R"(
//{
//    "importAsSources": true,
//    "exportedAsSinks": true,
//    "blackList": [],
//    "whiteList": [],
//    "tainted": {
//        "main": {"params": [1,2]}
//    },
//    "bufferOverflow": {
//        "read" : {
//            "buffer": 2,
//            "size": 3
//        },
//        "fgets" : {
//            "buffer": 1,
//            "size": 2
//        },
//        "gets" : {
//            "buffer": 1,
//        }
//    },
//}
//)"_json;

enum class VulnType {
    Unreachable,
    FormatStrings,
    BufferOverflow,
    Tainted,
    UaF,
    DoubleFree,
    IntegerOverflow
};

// NLOHMANN_JSON_SERIALIZE_ENUM(VulnType,
//                             {
//                                 {Unreachable, "Unreachable"},
//                                 {FormatStrings, "Format Strings"},
//                                 {BufferOverflow, "Buffer Overflow"},
//                                 {Tainted, "Tainted Variable"},
//                                 {UaF, "Use After Free"},
//                                 {DoubleFree, "Double Free"},
//                                 {IntegerOverflow, "IntegerOverflow"},
//                             });

void checkVulnerabilities(Graph* graph);
void checkUnreachableCode();
void checkBufferOverflow();
void checkIntegerOverflow();
void checkUseAfterFree();
std::map<int, int> checkBufferSizes(Node* func);

}  // namespace wasmati
#endif  // WABT_AST_BUILDER_H_
