#ifndef WASMATI_OPTIONS_H_
#define WASMATI_OPTIONS_H_
namespace wasmati {
struct GenerateCPGOptions {
    std::string funcName;
    bool printNoAST = true;
    bool printNoCFG = true;
    bool printNoPDG = true;
    bool printNoCG = true;
    bool printNoPG = true;
};

}  // namespace wasmati
#endif  // WABT_OPTIONS_H_
