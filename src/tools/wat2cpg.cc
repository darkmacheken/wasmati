#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "config.h"
#include "src/apply-names.h"
#include "src/ast-builder.h"
#include "src/cfg-builder.h"
#include "src/pdg-builder.h"
#include "src/common.h"
#include "src/dot-writer.h"
#include "src/error-formatter.h"
#include "src/feature.h"
#include "src/filenames.h"
#include "src/generate-names.h"
#include "src/graph.h"
#include "src/ir.h"
#include "src/option-parser.h"
#include "src/options.h"
#include "src/resolve-names.h"
#include "src/stream.h"
#include "src/validator.h"
#include "src/wast-parser.h"

using namespace wabt;
using namespace wasmati;

void generateCPG(Graph&, GenerateCPGOptions);

static int s_verbose;
static std::string s_infile;
static std::string s_outfile;
static GenerateCPGOptions cpgOptions;
static Features s_features;
static bool s_fail_on_custom_section_error = true;
static std::unique_ptr<FileStream> s_log_stream;
static bool s_validate = true;

static const char s_description[] =
    R"(  Read a file in the WebAssembly text format, and convert it to
  the Code Property Graph.

examples:
  # parse binary file test.wasm and write text file test.wast
  $ wasm2wat test.wasm -o test.wat

  # parse test.wasm, write test.wat, but ignore the debug names, if any
  $ wasm2wat test.wasm --no-debug-names -o test.wat
)";

static void ParseOptions(int argc, char** argv) {
    OptionParser parser("wasm2wat", s_description);

    parser.AddOption('v', "verbose", "Use multiple times for more info", []() {
        s_verbose++;
        s_log_stream = FileStream::CreateStdout();
    });
    parser.AddOption(
        'o', "output", "FILENAME",
        "Output file for the generated wast file, by default use stdout",
        [](const char* argument) {
            s_outfile = argument;
            ConvertBackslashToSlash(&s_outfile);
        });
    parser.AddOption('f', "function", "FUNCTIONNAME",
                     "Output file for the given function.",
                     [](const char* argument) {
                         cpgOptions.funcName = argument;
                         cpgOptions.funcName = "$" + cpgOptions.funcName;
                     });
    s_features.AddOptions(&parser);
    parser.AddOption("ignore-custom-section-errors",
                     "Ignore errors in custom sections",
                     []() { s_fail_on_custom_section_error = false; });
    parser.AddOption("no-check", "Don't check for invalid modules",
                     []() { s_validate = false; });
    parser.AddArgument("filename", OptionParser::ArgumentCount::One,
                       [](const char* argument) {
                           s_infile = argument;
                           ConvertBackslashToSlash(&s_infile);
                       });
    parser.AddOption("ast", "Output just the Abstract Syntax Tree", []() {
        cpgOptions.printNoCFG = true;
        cpgOptions.printNoPDG = true;
    });
    parser.AddOption("cfg", "Output just the Control Flow Graph", []() {
        cpgOptions.printNoAST = true;
        cpgOptions.printNoPDG = true;
    });
    parser.AddOption("pdg", "Output just the Program Dependence Graph", []() {
        cpgOptions.printNoAST = true;
        cpgOptions.printNoCFG = true;
    });
    parser.AddOption("no-ast", "Output just the Abstract Syntax Tree",
                     []() { cpgOptions.printNoAST = true; });
    parser.AddOption("no-cfg", "Output just the Control Flow Graph",
                     []() { cpgOptions.printNoCFG = true; });
    parser.AddOption("no-pdg", "Output just the Program Dependence Graph",
                     []() { cpgOptions.printNoPDG = true; });
    parser.Parse(argc, argv);
}

int ProgramMain(int argc, char** argv) {
    InitStdio();

    ParseOptions(argc, argv);

    std::vector<uint8_t> file_data;
    Result result = ReadFile(s_infile, &file_data);
    std::unique_ptr<WastLexer> lexer = WastLexer::CreateBufferLexer(
        s_infile, file_data.data(), file_data.size());
    if (Failed(result)) {
        WABT_FATAL("unable to read file: %s\n", s_infile.c_str());
    }

    Errors errors;
    std::unique_ptr<wabt::Module> module;
    WastParseOptions parse_wast_options(s_features);
    result = ParseWatModule(lexer.get(), &module, &errors, &parse_wast_options);

    if (Succeeded(result)) {
        result = ResolveNamesModule(module.get(), &errors);

        if (Succeeded(result) && s_validate) {
            ValidateOptions options(s_features);
            result = ValidateModule(module.get(), &errors, options);
        }

        auto line_finder = lexer->MakeLineFinder();
        FormatErrorsToFile(errors, Location::Type::Text, line_finder.get());

        result = GenerateNames(module.get());

        if (Succeeded(result)) {
            /* TODO(binji): This shouldn't fail; if a name can't be applied
             * (because the index is invalid, say) it should just be skipped. */
            Result dummy_result = ApplyNames(module.get());
            WABT_USE(dummy_result);
        }

        Graph graph(module.get());
        generateCPG(graph, cpgOptions);

        if (Succeeded(result)) {
            FileStream stream(!s_outfile.empty() ? FileStream(s_outfile)
                                                 : FileStream(stdout));
            DotWriter writer(&stream, &graph, cpgOptions);
            writer.writeGraph();
        }
    }

    return result != Result::Ok;
}

void generateCPG(Graph& graph, GenerateCPGOptions options) {
    AST ast(*graph.getModuleContext(), graph);
    ast.generateAST(cpgOptions);
    CFG cfg(*graph.getModuleContext(), graph, ast);
    cfg.generateCFG(cpgOptions);
    PDG pdg(*graph.getModuleContext(), graph);
    //pdg.generatePDG(cpgOptions);
}

int main(int argc, char** argv) {
    WABT_TRY
    return ProgramMain(argc, argv);
    WABT_CATCH_BAD_ALLOC_AND_EXIT
}
