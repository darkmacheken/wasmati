#include <fstream>
#include <sstream>

#include "src/interpreter/interpreter.h"
#include "src/interpreter/scanner.h"

namespace wasmati {

Interpreter::Interpreter(class Context& _context)
    : trace_scanning(false), trace_parsing(false), context(_context) {}

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
    std::ifstream in(filename.c_str());
    if (!in.good())
        return false;
    return parse_stream(in, filename);
}

bool Interpreter::parse_string(const std::string& input, const std::string& sname) {
    std::istringstream iss(input);
    return parse_stream(iss, sname);
}

void Interpreter::error(const class location& l, const std::string& m) {
    std::cerr << l << ": " << m << std::endl;
}

void Interpreter::error(const std::string& m) {
    std::cerr << m << std::endl;
}

}  // namespace wasmati
