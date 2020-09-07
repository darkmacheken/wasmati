#ifndef WASMATI_OPTIONS_H_
#define WASMATI_OPTIONS_H_
namespace wasmati {
struct GenerateCPGOptions {
    std::string funcName;
    bool printNoAST = false;
    bool printNoCFG = false;
    bool printNoPDG = false;
};

}  // namespace wasmati
#endif  // WABT_OPTIONS_H_
