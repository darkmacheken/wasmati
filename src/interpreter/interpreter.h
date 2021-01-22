#ifndef WASMATI_INTERPRETER_H
#define WASMATI_INTERPRETER_H

#include <string>
#include <vector>
#include "src/interpreter/nodes.h"

namespace wasmati {

/** The Interpreter class brings together all components. It creates an instance
 * of the Parser and Scanner classes and connects them. Then the input stream is
 * fed into the scanner object and the parser gets it's token
 * sequence. Furthermore the interpreter object is available in the grammar
 * rules as a parameter. Therefore the interpreter class contains a reference to
 * the structure into which the parsed data is saved. */
class Interpreter {
    BasicNode* _ast;

public:
    /// construct a new parser interpreter context
    Interpreter()
        : _ast(nullptr), trace_scanning(false), trace_parsing(false) {}

    /// enable debug output in the flex scanner
    bool trace_scanning;

    /// enable debug output in the bison parser
    bool trace_parsing;

    /// stream name (file or input stream) used for error messages.
    std::string streamname;

    /** Invoke the scanner and parser for a stream.
     * @param in	input stream
     * @param sname	stream name for error messages
     * @return		true if successfully parsed
     */
    bool parse_stream(std::istream& in,
                      const std::string& sname = "stream input");

    /** Invoke the scanner and parser on an input string.
     * @param input	input string
     * @param sname	stream name for error messages
     * @return		true if successfully parsed
     */
    bool parse_string(const std::string& input,
                      const std::string& sname = "string stream");

    /** Invoke the scanner and parser on a file. Use parse_stream with a
     * std::ifstream if detection of file reading errors is required.
     * @param filename	input file name
     * @return		true if successfully parsed
     */
    bool parse_file(const std::string& filename);

    int lineno() const;

    // To demonstrate pure handling of parse errors, instead of
    // simply dumping them on the standard error output, we will pass
    // them to the interpreter using the following two member functions.

    /** Error handling with associated line number. This can be modified to
     * output the error e.g. to a dialog box. */
    void error(const class location& l, const std::string& m);

    /** General error handling. This can be modified to output the error
     * e.g. to a dialog box. */
    void error(const std::string& m);

    /** Pointer to the current lexer instance, this is used to connect the
     * parser to the scanner. It is used in the yylex macro. */
    class Scanner* lexer;

public:
    BasicNode* ast() { return _ast; }
    void ast(BasicNode* ast) { _ast = ast; }

    int64_t evaluate(Visitor* visitor);
};

}  // namespace wasmati

#endif  // WASMATI_INTERPRETER_H
