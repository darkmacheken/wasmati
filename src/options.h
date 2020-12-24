#ifndef WASMATI_OPTIONS_H_
#define WASMATI_OPTIONS_H_

#include <chrono>
#include "src/stream.h"

#define debug(format, ...)                             \
    if (cpgOptions.verbose) {                          \
        s_verbose_stream->Writef(format, __VA_ARGS__); \
    }

#define warning(expr)                                                         \
    if (cpgOptions.verbose && !(expr)) {                                      \
        s_verbose_stream->Writef(                                             \
            "[WARNING] Assert failed '%s' in file %s at line %d in function " \
            "%s\n",                                                           \
            __STRING(expr), __FILE__, __LINE__, __ASSERT_FUNCTION);           \
    }

namespace wasmati {
struct GenerateCPGOptions {
    std::string funcName;
    bool printAST = false;
    bool printCFG = false;
    bool printPDG = false;
    bool printCG = false;
    bool printAll = true;
    bool verbose = false;
    bool info = false;
    std::string loopName;
};

extern GenerateCPGOptions cpgOptions;
extern std::unique_ptr<wabt::FileStream> s_verbose_stream;
}  // namespace wasmati
#endif  // WABT_OPTIONS_H_
