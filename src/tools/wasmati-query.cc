#include <string>
#include "src/common.h"
#include "src/interpreter/evaluator.h"
#include "src/interpreter/interpreter.h"
#include "src/option-parser.h"
#include "src/options.h"
#include "src/readers/csv-reader.h"
#include "src/stream.h"
#include "src/vulns.h"

using namespace wabt;
using namespace wasmati;

bool native = false;
bool interactive = false;
static std::string s_configfile;
static std::string s_infile;
static std::string s_outfile;
static std::string s_zipfile;

static const char s_description[] =
    R"(  Query the imported Code Property Graph.
)";

static void ParseOptions(int argc, char** argv) {
    OptionParser parser("wasmati", s_description);

    parser.AddOption('v', "verbose",
                     "Output information quering of Code Property Graph",
                     []() { cpgOptions.verbose = true; });
    parser.AddOption(
        'o', "output", "FILENAME",
        "Output file for vulnerability report, by default use stdout",
        [](const char* argument) {
            s_outfile = argument;
            ConvertBackslashToSlash(&s_outfile);
        });
    parser.AddOption('i', "interactive", "Activate interative mode.",
                     []() { interactive = true; });
    parser.AddOption('q', "query", "FILENAME", "Load DSL query.",
                     [](const char* argument) {
                         s_infile = argument;
                         ConvertBackslashToSlash(&s_infile);
                     });
    parser.AddOption('g', "graph", "FILENAME", "Load serialised graph.",
                     [](const char* argument) {
                         s_zipfile = argument;
                         ConvertBackslashToSlash(&s_zipfile);
                     });
    parser.AddOption('c', "config", "FILENAME", "JSON configuration file.",
                     [](const char* argument) {
                         s_configfile = argument;
                         ConvertBackslashToSlash(&s_configfile);
                     });
    parser.AddOption("native", "Execute native queries.",
                     []() { native = true; });
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

    graph = new Graph();
    if (!s_zipfile.empty()) {
        auto start = std::chrono::high_resolution_clock::now();
        CSVReader reader(s_zipfile, graph);
        auto stat = reader.readGraph();
        auto end = std::chrono::high_resolution_clock::now();
        auto loadDuration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count();
        if (interactive) {
            std::cout << "Loaded " << stat.first << " nodes and " << stat.second
                      << " edges in " << loadDuration << " ms." << std::endl
                      << std::endl;
        }
    }
    Query::setGraph(graph);

    // Evaluate file if provided
    Evaluator evaluator = Evaluator(config, s_infile);
    Interpreter interp = Interpreter();
    if (!s_infile.empty()) {
        assert(interp.parse_file(s_infile));
        // JsonPrinter printer;
        // interp.evaluate(&printer);
        // std::cout << printer.toString() << std::endl;
        try {
            auto time = interp.evaluate(&evaluator);
            std::cout << "\nTime: " << time << " ms" << std::endl;

        } catch (const InterpreterException& e) {
            std::cout << e.what() << std::endl;
        }
    }

    // Check vulnerabilities
    if (native) {
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

    // interactive mode
    while (interactive) {
        std::cout << ">>> ";
        std::string line;
        std::getline(std::cin, line);
        if (line == "exit") {
            break;
        } else if (line == "...") {
            std::string text;
            line = "";
            do {
                std::cout << "... ";
                std::getline(std::cin, line);
                text += line + "\n";
            } while (!line.empty());
            line = text;
        }
        line += std::char_traits<char>::eof();
        try {
            if (!interp.parse_string(line)) {
                continue;
            }
            auto time = interp.evaluate(&evaluator);
            if (evaluator.resultToString() != "") {
                std::cout << evaluator.resultToString() << std::endl;
            }
            // std::cout << "\nTime: " << time << " ms" << std::endl;
        } catch (InterpreterException& e) {
            std::cout << e.what() << std::endl;
        }
    }
    return Result::Ok;
}

int main(int argc, char** argv) {
    WABT_TRY
    return ProgramMain(argc, argv);
    WABT_CATCH_BAD_ALLOC_AND_EXIT
}
