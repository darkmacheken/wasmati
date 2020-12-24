#include "src/apply-names.h"
#include "src/ast-builder.h"
#include "src/binary-reader-ir.h"
#include "src/binary-reader.h"
#include "src/cfg-builder.h"
#include "src/decompiler.h"
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
#include "src/writers/csv-writer.h"
#include "src/writers/datalog-writer.h"
#include "src/writers/dot-writer.h"
#include "src/writers/json-writer.h"

using namespace wabt;
using namespace wasmati;

void generateCPG(Graph&);
bool hasEnding(std::string const& fullString, std::string const& ending);
Result watFile(std::unique_ptr<wabt::Module>* mod);
Result wasmFile(std::unique_ptr<wabt::Module>* mod);

static std::string s_configfile;
static std::string s_infile;
static std::string s_outfile;
static std::string s_csv_outfile;
static std::string s_doutfile;
static std::string s_dlogdir;
static std::string s_json_outfile;
static bool generate_csv = false;
static bool generate_dot = false;
static bool generate_datalog_dir = false;
static bool generate_json = false;
static bool is_csv = false;
static bool is_wat = false;
static bool is_wasm = false;
static bool skip_queries = false;
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
                     "Serialize the graph as dot.", [](const char* argument) {
                         s_doutfile = argument;
                         ConvertBackslashToSlash(&s_doutfile);
                         generate_dot = true;
                     });
    parser.AddOption('D', "datalog", "DIRECTORY",
                     "Serialize the graph as a soufflé datalog program, facts "
                     "are csv files (node.facts and edge.facts files).",
                     [](const char* argument) {
                         s_dlogdir = argument;
                         ConvertBackslashToSlash(&s_dlogdir);
                         generate_datalog_dir = true;
                     });
    parser.AddOption('s', "csv-output", "FILENAME",
                     "Serialize the graph as csv file.",
                     [](const char* argument) {
                         s_csv_outfile = argument;
                         ConvertBackslashToSlash(&s_csv_outfile);
                         generate_csv = true;
                     });
    parser.AddOption('j', "json-output", "FILENAME",
                     "Serialize the graph as JSON file.",
                     [](const char* argument) {
                         s_json_outfile = argument;
                         ConvertBackslashToSlash(&s_json_outfile);
                         generate_json = true;
                     });
    parser.AddOption('c', "config", "FILENAME", "JSON configuration file.",
                     [](const char* argument) {
                         s_configfile = argument;
                         ConvertBackslashToSlash(&s_configfile);
                     });
    parser.AddOption("csv", "Treat input file as a csv file.",
                     []() { is_csv = true; });
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
    parser.AddOption("skip-queries", "Do not execute queries.",
                     []() { skip_queries = true; });
    parser.AddOption("ast", "Output the Abstract Syntax Tree", []() {
        cpgOptions.printAST = true;
        cpgOptions.printAll = false;
    });
    parser.AddOption("cfg", "Output the Control Flow Graph", []() {
        cpgOptions.printCFG = true;
        cpgOptions.printAll = false;
    });
    parser.AddOption("pdg", "Output the Program Dependence Graph", []() {
        cpgOptions.printPDG = true;
        cpgOptions.printAll = false;
    });
    parser.AddOption("cg", "Output the Call Graph", []() {
        cpgOptions.printCG = true;
        cpgOptions.printAll = false;
    });
    parser.Parse(argc, argv);
}

int ProgramMain(int argc, char** argv) {
    InitStdio();
    ParseOptions(argc, argv);
    Graph* graph = nullptr;

    // Parse and validate Config file
    json config;
    if (s_configfile.empty()) {
        config = defaultConfig;
    } else {
        std::ifstream stream(s_configfile);
        stream >> config;
    }
    VulnerabilityChecker::verifyConfig(config);

    std::unique_ptr<wabt::Module> module;
    Result result;

    auto start = std::chrono::high_resolution_clock::now();
    // Check given program
    if (is_wat || hasEnding(s_infile, ".wat") || hasEnding(s_infile, ".wast")) {
        result = watFile(&module);
    } else if (is_wasm || hasEnding(s_infile, ".wasm")) {
        result = wasmFile(&module);
    } else if (is_csv || hasEnding(s_infile, ".csv")) {
        is_csv = true;
        result = Result::Ok;
        graph = new Graph();
        graph->populate(s_infile);
        Query::setGraph(graph);
    } else {
        WABT_FATAL("Unable to verify file type: %s\n", s_infile.c_str());
    }
    auto parsing = std::chrono::high_resolution_clock::now();

    // if fail in parsing, stop the program
    if (Failed(result)) {
        return result != Result::Ok;
    }

    if (cpgOptions.info) {
        auto parsingDuration =
            std::chrono::duration_cast<std::chrono::milliseconds>(parsing -
                                                                  start);
        info["parsing"] = parsingDuration.count();
    }

    // Generate graph
    if (!is_csv) {
        graph = new Graph(*module.get());
        Query::setGraph(graph);
        generateCPG(*graph);
    }

    // Check vulnerabilities
    if (!skip_queries) {
        auto startVulns = std::chrono::high_resolution_clock::now();
        std::list<Vulnerability> vulns;
        VulnerabilityChecker(config, vulns).checkVulnerabilities();
        auto endVulns = std::chrono::high_resolution_clock::now();

        if (cpgOptions.info) {
            auto queryDuration =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    endVulns - startVulns);
            info["query"] = queryDuration.count();
        }

        // Print vulns
        json list = vulns;
        if (s_outfile.empty()) {
            FileStream(stdout).Writef("%s\n", list.dump(4).c_str());
        } else {
            std::ofstream o(s_outfile);
            o << list.dump(4) << std::endl;
        }
    }

    // generate csv
    if (Succeeded(result) && generate_csv) {
        FileStream stream(!s_csv_outfile.empty() ? FileStream(s_csv_outfile)
                                                 : FileStream(stdout));
        CSVWriter writer(&stream, graph);
        writer.writeGraph();
    }
    // generate dot
    if (Succeeded(result) && generate_dot) {
        FileStream stream(!s_doutfile.empty() ? FileStream(s_doutfile)
                                              : FileStream(stdout));
        DotWriter writer(&stream, graph);
        writer.writeGraph();
    }
    // generate json
    if (Succeeded(result) && generate_json) {
        FileStream stream(!s_json_outfile.empty() ? FileStream(s_json_outfile)
                                                  : FileStream(stdout));
        JSONWriter writer(&stream, graph);
        writer.writeGraph();
    }
    // generate datalog facts
    if (Succeeded(result) && generate_datalog_dir) {
        assert(!s_dlogdir.empty());
        auto stream = FileStream(s_dlogdir + "/base.dl");
        auto edges = FileStream(s_dlogdir + "/edge.facts");
        auto nodes = FileStream(s_dlogdir + "/node.facts");
        DatalogWriter writer(&stream, &edges, &nodes, graph);
        writer.writeGraph();
    }
    if (cpgOptions.info) {
        auto total = std::chrono::high_resolution_clock::now();
        auto totalDuration =
            std::chrono::duration_cast<std::chrono::milliseconds>(total -
                                                                  start);
        info["total"] = totalDuration.count();
        info["nodes"] = graph->getNumberNodes();
        info["edges"] = graph->getNumberEdges();
        info["memory"] = graph->getMemoryUsage();
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
    } else {
        auto line_finder = lexer->MakeLineFinder();
        FormatErrorsToFile(errors, Location::Type::Text, line_finder.get());
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
        info["cfg"] = cfgDuration.count() - cfg.totalTime;
        info["pdg"] = pdgDuration.count();
        info["cg"] = cfg.totalTime;
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
