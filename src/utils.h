#ifndef WASMATI_UTILS_H_
#define WASMATI_UTILS_H_
#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "include/nlohmann/json.hpp"
#include "src/cast.h"
#include "src/ir-util.h"

#define ID "id"
#define NODE_TYPE "nodeType"
#define NAME "name"
#define INDEX "index"
#define NARGS "nargs"
#define NLOCALS "nlocals"
#define NRESULTS "nresults"
#define IS_IMPORT "isImport"
#define IS_EXPORT "isImport"
#define VAR_TYPE "varType"
#define INST_TYPE "instType"
#define OPCODE "opcode"
#define CONST_TYPE "constType"
#define CONST_VALUE_I "constValueI"
#define CONST_VALUE_F "constValueF"
#define LABEL "label"
#define OFFSET "offset"
#define HAS_ELSE "hasElse"
#define SRC "src"
#define DEST "dest"
#define EDGE_TYPE "edgeType"
#define PDG_TYPE "pdgType"
#define AST_ORDER "astOrder"

using namespace wabt;
using nlohmann::json;
namespace wasmati {

struct Utils {
#define WASMATI_PREDICATE_VALUES_I(funcName, valType, field, rtype) \
    static valType value##funcName(const Const& _const) {           \
        return static_cast<valType>(_const.field);                  \
    }

#define WASMATI_PREDICATE_VALUES_F(funcName, valType, field, rtype) \
    static valType value##funcName(const Const& _const) {           \
        valType fval;                                               \
        memcpy(&fval, &_const.field, sizeof(fval));                 \
        return static_cast<valType>(fval);                          \
    }
#include "src/config/predicates.def"
#undef WASMATI_PREDICATE_VALUES_I
#undef WASMATI_PREDICATE_VALUES_F

    static std::string writeConstType(const Type type) {
        switch (type) {
        case Type::I32:
            return "i32";
        case Type::I64:
            return "i64";
        case Type::F32:
            return "f32";
        case Type::F64:
            return "f64";
        case Type::V128: {
            assert(false);
            break;
        }
        default:
            assert(false);
            break;
        }
    }

    static std::string writeConstType(const Const& _const) {
        return writeConstType(_const.type);
    }

    static std::string writeConst(const Const& _const, bool prefix = true) {
        std::string s;
        switch (_const.type) {
        case Type::I32:
            if (prefix) {
                s += Opcode::I32Const_Opcode.GetName();
                s += " ";
            }
            s += std::to_string(Utils::valueI32(_const));
            break;

        case Type::I64:
            if (prefix) {
                s += Opcode::I64Const_Opcode.GetName();
                s += " ";
            }
            s += std::to_string(Utils::valueI64(_const));
            break;

        case Type::F32: {
            if (prefix) {
                s += Opcode::F32Const_Opcode.GetName();
                s += " ";
            }
            s += std::to_string(Utils::valueF32(_const));
            break;
        }

        case Type::F64: {
            if (prefix) {
                s += Opcode::F64Const_Opcode.GetName();
                s += " ";
            }
            s += std::to_string(Utils::valueF64(_const));
            break;
        }

        case Type::V128: {
            assert(false);
            break;
        }

        default:
            assert(false);
            break;
        }
        return s;
    }

    static json jsonConst(const Const& _const) {
        json res;
        switch (_const.type) {
        case Type::I32:
            res["type"] = "i32";
            res["value"] = Utils::valueI32(_const);
            break;

        case Type::I64:
            res["type"] = "i64";
            res["value"] = Utils::valueI64(_const);
            break;

        case Type::F32: {
            res["type"] = "f32";
            res["value"] = Utils::valueF32(_const);
            break;
        }

        case Type::F64: {
            res["type"] = "f64";
            res["value"] = Utils::valueF64(_const);
            break;
        }

        case Type::V128: {
            assert(false);
            break;
        }

        default:
            assert(false);
            break;
        }
        return res;
    }

    static std::string currentDate() {
        // Get current time
        std::chrono::time_point<std::chrono::system_clock> time_now =
            std::chrono::system_clock::now();

        // Format time and print using put_time
        std::time_t time_now_t = std::chrono::system_clock::to_time_t(time_now);
        std::tm now_tm = *std::localtime(&time_now_t);
        std::stringstream ss;
        ss << std::put_time(&now_tm, "%d-%m-%Y %H:%M:%S");
        return ss.str();
    }

    static std::vector<std::string> split(std::string& str, char delim) {
        std::vector<std::string> result;
        std::size_t current, previous = 0;
        if (str.empty()) {
            return std::vector<std::string>();
        }
        current = str.find(delim);
        while (current != std::string::npos) {
            result.push_back(str.substr(previous, current - previous));
            previous = current + 1;
            current = str.find(delim, previous);
        }
        result.push_back(str.substr(previous, current - previous));
        return result;
    }
};

class Path {
    std::vector<std::string> _path;

    std::vector<std::string> parsePath(std::string path) {
        if (path.size() == 0) {
            return std::vector<std::string>();
        }
        std::vector<std::string> result;
        std::size_t current, previous = 0;
        current = path.find('/');
        if (current == 0) {
            current = path.find('/', 1);
            result.push_back(path.substr(0, current));
            previous = current;
        }
        while (current != std::string::npos) {
            current = path.find('/', previous + 1);
            result.push_back(path.substr(previous, current - previous));
            previous = current;
        }

        return result;
    }

public:
    Path(std::string path) { _path = parsePath(path); }

    std::string directory() {
        if (_path.size() == 0) {
            return "";
        } else if (_path.size() == 1) {
            return _path[0];
        }

        std::string result = "";
        if (_path[_path.size() - 1].find('.') != std::string::npos) {
            for (size_t i = 0; i < _path.size() - 1; i++) {
                result += _path[i];
            }
        } else {
            for (size_t i = 0; i < _path.size(); i++) {
                result += _path[i];
            }
        }
        return result;
    }

    bool empty() { return _path.empty(); }

    static std::string getCurrentDir() {
        char buff[FILENAME_MAX]; //create string buffer to hold path
        GetCurrentDir(buff, FILENAME_MAX);
        std::string current_working_dir(buff);
        return current_working_dir;
    }
};

}  // namespace wasmati
#endif  // WABT_AST_BUILDER_H_
