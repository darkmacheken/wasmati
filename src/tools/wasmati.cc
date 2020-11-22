#include <chrono>
#include "src/apply-names.h"
#include "src/ast-builder.h"
#include "src/binary-reader-ir.h"
#include "src/binary-reader.h"
#include "src/cfg-builder.h"
#include "src/datalog-writer.h"
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

void generateCPG(Graph&);
bool hasEnding(std::string const& fullString, std::string const& ending);
Result watFile(std::unique_ptr<wabt::Module>* mod);
Result wasmFile(std::unique_ptr<wabt::Module>* mod);

static std::string s_configfile;
static std::string s_infile;
static std::string s_outfile;
static std::string s_doutfile;
static std::string s_dlogfile;
static bool generate_dot = false;
static bool generate_datalog = false;
static bool is_wat = false;
static bool is_wasm = false;
static Features s_features;
static bool s_read_debug_names = true;
static bool s_fail_on_custom_section_error = true;
static std::unique_ptr<FileStream> s_log_stream;
static std::unique_ptr<FileStream> s_info_stream;
static bool s_validate = true;
json info;

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

    parser.AddOption(
        'v', "verbose",
        "Output information about the generation of Code Property Graph",
        []() { cpgOptions.verbose = true; });
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
    parser.AddOption('g', "datalog", "FILENAME",
                     "Output file for vulnerability report.",
                     [](const char* argument) {
                         s_dlogfile = argument;
                         ConvertBackslashToSlash(&s_dlogfile);
                         generate_datalog = true;
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
    parser.AddOption('i', "info",
                     "Print time information of the generation of CPG.",
                     []() { cpgOptions.info = true; });
    parser.AddOption('l', "loop", "LOOPNAME",
                     "Print all information during generation of CPG.",
                     [](const char* argument) {
                         cpgOptions.loopName = argument;
                         cpgOptions.loopName = "$" + cpgOptions.loopName;
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
    parser.AddOption("ast", "Output the Abstract Syntax Tree",
                     []() { cpgOptions.printNoAST = false; });
    parser.AddOption("cfg", "Output the Control Flow Graph",
                     []() { cpgOptions.printNoCFG = false; });
    parser.AddOption("pdg", "Output the Program Dependence Graph",
                     []() { cpgOptions.printNoPDG = false; });
    parser.AddOption("cg", "Output the Call Graph",
                     []() { cpgOptions.printNoCG = false; });
    parser.AddOption("pg", "Output the Parameters Graph",
                     []() { cpgOptions.printNoPG = false; });
    parser.AddOption("all", "Output the Parameters Graph", []() {
        cpgOptions.printNoAST = false;
        cpgOptions.printNoCFG = false;
        cpgOptions.printNoPDG = false;
        cpgOptions.printNoCG = false;
        cpgOptions.printNoPG = false;
    });
    parser.Parse(argc, argv);
}

int ProgramMain(int argc, char** argv) {
    InitStdio();
    ParseOptions(argc, argv);

    std::unique_ptr<wabt::Module> module;
    Result result;
    auto start = std::chrono::high_resolution_clock::now();
    //s_features.set_bulk_memory_enabled(true);
    if (is_wat || hasEnding(s_infile, ".wat") || hasEnding(s_infile, ".wast")) {
        result = watFile(&module);
    } else if (is_wasm || hasEnding(s_infile, ".wasm")) {
        result = wasmFile(&module);
    } else {
        WABT_FATAL("Unable to verify file type: %s\n", s_infile.c_str());
    }
    if (Failed(result)) {
        return result != Result::Ok;
    }
    if (cpgOptions.info) {
        auto parsing = std::chrono::high_resolution_clock::now();
        auto parsingDuration =
            std::chrono::duration_cast<std::chrono::milliseconds>(parsing -
                                                                  start);
        info["parsing"] = parsingDuration.count();
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
    generateCPG(graph);

    auto startVulns = std::chrono::high_resolution_clock::now();
    std::list<Vulnerability> vulns;
    checkVulnerabilities(config, vulns);

    if (cpgOptions.info) {
        auto endVulns = std::chrono::high_resolution_clock::now();
        auto queryDuration =
            std::chrono::duration_cast<std::chrono::milliseconds>(endVulns -
                                                                  startVulns);
        info["query"] = queryDuration.count();
    }

    json list = vulns;

    if (s_outfile.empty()) {
        FileStream(stdout).Writef("%s\n", list.dump(4).c_str());
    } else {
        std::ofstream o(s_outfile);
        o << list.dump(4) << std::endl;
    }

    if (Succeeded(result) && generate_dot) {
        FileStream stream(!s_doutfile.empty() ? FileStream(s_doutfile)
                                              : FileStream(stdout));
        DotWriter writer(&stream, &graph);
        writer.writeGraph();
    }
    if (Succeeded(result) && generate_datalog) {
        FileStream stream(!s_dlogfile.empty() ? FileStream(s_dlogfile)
                                              : FileStream(stdout));
        DatalogWriter writer(&stream, &graph);
        writer.writeGraph();
    }
    if (cpgOptions.info) {
        auto total = std::chrono::high_resolution_clock::now();
        auto totalDuration =
            std::chrono::duration_cast<std::chrono::milliseconds>(total -
                                                                  start);
        info["total"] = totalDuration.count();
        info["nodes"] = graph.getNumberNodes();
        info["edges"] = graph.getNumberEdges();
        info["memory"] = graph.getMemoryUsage();
        FileStream(stdout).Writef("%s\n", info.dump(2).c_str());
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

void generateCPG(Graph& graph) {
    auto start = std::chrono::high_resolution_clock::now();

    AST ast(graph.getModuleContext(), graph);
    ast.generateAST();
    auto astTime = std::chrono::high_resolution_clock::now();

    CFG cfg(graph.getModuleContext(), graph, ast);
    cfg.generateCFG();
    auto cfgTime = std::chrono::high_resolution_clock::now();

    PDG pdg(graph.getModuleContext(), graph);
    pdg.generatePDG();
    auto pdgTime = std::chrono::high_resolution_clock::now();

    if (cpgOptions.info) {
        auto astDuration =
            std::chrono::duration_cast<std::chrono::milliseconds>(astTime -
                                                                  start);
        auto cfgDuration =
            std::chrono::duration_cast<std::chrono::milliseconds>(cfgTime -
                                                                  astTime);
        auto pdgDuration =
            std::chrono::duration_cast<std::chrono::milliseconds>(pdgTime -
                                                                  cfgTime);
        info["ast"] = astDuration.count();
        info["cfg"] = cfgDuration.count();
        info["pdg"] = pdgDuration.count();
    }
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
