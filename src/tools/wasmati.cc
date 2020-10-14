#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include "src/apply-names.h"
#include "src/ast-builder.h"
#include "src/binary-reader-ir.h"
#include "src/binary-reader.h"
#include "src/cfg-builder.h"
#include "src/decompiler.h"
#include "src/dot-writer.h"
#include "src/error-formatter.h"
#include "src/feature.h"
#include "src/generate-names.h"
#include "src/graph.h"
#include "src/ir.h"
#include "src/option-parser.h"
#include "src/options.h"
#include "src/pdg-builder.h"
#include "src/resolve-names.h"
#include "src/stream.h"
#include "src/validator.h"
#include "src/vulns.h"
#include "src/wast-lexer.h"
#include "src/wast-parser.h"

using namespace wabt;
using namespace wasmati;

void generateCPG(Graph&, GenerateCPGOptions);
bool hasEnding(std::string const& fullString, std::string const& ending);
Result watFile(std::unique_ptr<wabt::Module>* mod);
Result wasmFile(std::unique_ptr<wabt::Module>* mod);

static int s_verbose;
static std::string s_configfile;
static std::string s_infile;
static std::string s_outfile;
static std::string s_doutfile;
static bool generate_dot = false;
static bool is_wat = false;
static bool is_wasm = false;
static GenerateCPGOptions cpgOptions;
static Features s_features;
static bool s_read_debug_names = true;
static bool s_fail_on_custom_section_error = true;
static std::unique_ptr<FileStream> s_log_stream;
static bool s_validate = true;

static const char s_description[] =
    R"(  Read a file in the WebAssembly binary format or text format, and produces its
  Code Property Graph to perform queries and find vulnerabilities in the code.

examples:
  # parse binary file test.wasm and write text file test.wast
  $ wasm2cpg test.wasm -o test.wat

  # parse test.wasm, write test.wat, but ignore the debug names, if any
  $ wasm2cpg test.wasm --no-debug-names -o test.wat
)";

static void ParseOptions(int argc, char** argv) {
    OptionParser parser("wasmati", s_description);

    parser.AddOption('v', "verbose", "Use multiple times for more info", []() {
        s_verbose++;
        s_log_stream = FileStream::CreateStdout();
    });
    parser.AddOption(
        'o', "output", "FILENAME",
        "Output file for vulnerability report, by default use stdout",
        [](const char* argument) {
            s_outfile = argument;
            ConvertBackslashToSlash(&s_outfile);
        });
    parser.AddOption('d', "dot-output", "FILENAME",
                     "Output file for vulnerability report.",
                     [](const char* argument) {
                         s_doutfile = argument;
                         ConvertBackslashToSlash(&s_doutfile);
                         generate_dot = true;
                     });
    parser.AddOption('c', "config", "FILENAME",
                     "Output file for vulnerability report.",
                     [](const char* argument) {
                         s_configfile = argument;
                         ConvertBackslashToSlash(&s_configfile);
                     });
    parser.AddOption("wat", "Treat input file as a wat file.",
                     []() { is_wat = true; });
    parser.AddOption("wasm", "Treat input file as a wasm file.",
                     []() { is_wasm = true; });
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
    parser.AddOption("ast", "Output the Abstract Syntax Tree", []() {
        cpgOptions.printNoAST = false;
    });
    parser.AddOption("cfg", "Output the Control Flow Graph", []() {
        cpgOptions.printNoCFG = false;
    });
    parser.AddOption("pdg", "Output the Program Dependence Graph", []() {
        cpgOptions.printNoPDG = false;
    });
    parser.AddOption("cg", "Output the Call Graph", []() {
        cpgOptions.printNoCG = false;
    });
    parser.AddOption("pg", "Output the Parameters Graph", []() {
        cpgOptions.printNoPG = false;
    });
    parser.Parse(argc, argv);
}

int ProgramMain(int argc, char** argv) {
    InitStdio();
    ParseOptions(argc, argv);

    std::unique_ptr<wabt::Module> module;
    Result result;

    if (is_wat || hasEnding(s_infile, ".wat") || hasEnding(s_infile, ".wast")) {
        result = watFile(&module);
    } else if (is_wasm || hasEnding(s_infile, ".wasm")) {
        result = wasmFile(&module);
    } else {
        WABT_FATAL("Unable to verify file type: %s\n", s_infile.c_str());
    }

    // Config file
    json config;
    if (s_configfile.empty()) {
        config = defaultConfig;
    } else {
        std::ifstream stream(s_configfile);
        stream >> config;
    }

    Graph graph(*module.get());
    Query::setGraph(&graph);
    generateCPG(graph, cpgOptions);

    std::list<Vulnerability> vulns;
    checkVulnerabilities(config, vulns);

    json list = json::array();
    for (auto const& vuln : vulns) {
        json j;
        to_json(j, vuln);
        list.insert(list.end(), j);
    }

    if (s_outfile.empty()) {
        std::cout << list.dump(4) << std::endl;
    } else {
        std::ofstream o(s_outfile);
        o << list.dump(4) << std::endl;
    }

    if (Succeeded(result) && generate_dot) {
        FileStream stream(!s_doutfile.empty() ? FileStream(s_doutfile)
                                              : FileStream(stdout));
        DotWriter writer(&stream, &graph, cpgOptions);
        writer.writeGraph();
    }
    return result != Result::Ok;
}

Result watFile(std::unique_ptr<wabt::Module>* module) {
    std::vector<uint8_t> file_data;
    Result result = ReadFile(s_infile, &file_data);
    std::unique_ptr<WastLexer> lexer = WastLexer::CreateBufferLexer(
        s_infile, file_data.data(), file_data.size());
    if (Failed(result)) {
        WABT_FATAL("unable to read file: %s\n", s_infile.c_str());
    }

    Errors errors;
    WastParseOptions parse_wast_options(s_features);
    result = ParseWatModule(lexer.get(), module, &errors, &parse_wast_options);

    if (Succeeded(result)) {
        result = ResolveNamesModule(module->get(), &errors);

        if (Succeeded(result) && s_validate) {
            ValidateOptions options(s_features);
            result = ValidateModule(module->get(), &errors, options);
        }

        auto line_finder = lexer->MakeLineFinder();
        FormatErrorsToFile(errors, Location::Type::Text, line_finder.get());

        result = GenerateNames(module->get());

        if (Succeeded(result)) {
            /* TODO(binji): This shouldn't fail; if a name can't be applied
             * (because the index is invalid, say) it should just be skipped. */
            Result dummy_result = ApplyNames(module->get());
            WABT_USE(dummy_result);
        }
    }

    return result;
}

Result wasmFile(std::unique_ptr<wabt::Module>* mod) {
    auto module = MakeUnique<wabt::Module>();
    std::vector<uint8_t> file_data;
    Result result = ReadFile(s_infile.c_str(), &file_data);
    if (Succeeded(result)) {
        Errors errors;
        const bool kStopOnFirstError = true;
        ReadBinaryOptions options(s_features, s_log_stream.get(),
                                  s_read_debug_names, kStopOnFirstError,
                                  s_fail_on_custom_section_error);
        result = ReadBinaryIr(s_infile.c_str(), file_data.data(),
                              file_data.size(), options, &errors, module.get());
        if (Succeeded(result)) {
            if (Succeeded(result) && s_validate) {
                ValidateOptions options(s_features);
                result = ValidateModule(module.get(), &errors, options);
            }

            result = GenerateNames(module.get());

            if (Succeeded(result)) {
                /* TODO(binji): This shouldn't fail; if a name can't be applied
                 * (because the index is invalid, say) it should just be
                 * skipped. */
                Result dummy_result = ApplyNames(module.get());
                WABT_USE(dummy_result);
            }
        }
        FormatErrorsToFile(errors, Location::Type::Binary);
    }
    *mod = std::move(module);
    return result;
}

void generateCPG(Graph& graph, GenerateCPGOptions options) {
    AST ast(graph.getModuleContext(), graph);
    ast.generateAST(cpgOptions);
    CFG cfg(graph.getModuleContext(), graph, ast);
    cfg.generateCFG(cpgOptions);
    PDG pdg(graph.getModuleContext(), graph);
    pdg.generatePDG(cpgOptions);
}

bool hasEnding(std::string const& fullString, std::string const& ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(),
                                        ending.length(), ending));
    } else {
        return false;
    }
}

int main(int argc, char** argv) {
    WABT_TRY
    return ProgramMain(argc, argv);
    WABT_CATCH_BAD_ALLOC_AND_EXIT
}
