#ifndef WASMATI_UTILS_H_
#define WASMATI_UTILS_H_

#include "src/cast.h"
#include "src/ir-util.h"

using namespace wabt;
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

    static std::string writeConstType(const Const& _const) {
        switch (_const.type) {
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
};

}  // namespace wasmati
#endif  // WABT_AST_BUILDER_H_
