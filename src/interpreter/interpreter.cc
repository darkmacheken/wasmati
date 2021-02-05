#include <chrono>
#include <fstream>
#include <sstream>

#include "src/interpreter/interpreter.h"
#include "src/interpreter/scanner.h"

namespace wasmati {

bool Interpreter::parse_stream(std::istream& in, const std::string& sname) {
    streamname = sname;

    Scanner scanner(&in);
    scanner.set_debug(trace_scanning);
    this->lexer = &scanner;

    Parser parser(*this);
    parser.set_debug_level(trace_parsing);
    return (parser.parse() == 0);
}

bool Interpreter::parse_file(const std::string& filename) {
    _file = filename;
    std::ifstream in(filename.c_str());
    if (!in.good())
        return false;
    return parse_stream(in, filename);
}

int Interpreter::lineno() const {
    return lexer->lineno();
}

bool Interpreter::parse_string(const std::string& input,
                               const std::string& sname) {
    std::istringstream iss(input);
    return parse_stream(iss, sname);
}

void Interpreter::error(const class location& l, const std::string& m) {
    std::cerr << l << ": " << m << std::endl;
}

void Interpreter::error(const std::string& m) {
    std::cerr << m << std::endl;
}

int64_t Interpreter::evaluate(Visitor* visitor) {
    auto start = std::chrono::high_resolution_clock::now();
    if (!_ast.empty()) {
        _ast.back()->accept(visitor);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
        .count();
}

}  // namespace wasmati
