#include "src/common.h"
#include "src/option-parser.h"
#include "src/options.h"
#include "src/readers/csv-reader.h"
#include "src/stream.h"
#include "src/vulns.h"

using namespace wabt;
using namespace wasmati;

bool native = false;
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
        CSVReader reader(s_zipfile, graph);
        reader.readGraph();
    }
    Query::setGraph(graph);

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
    return Result::Ok;
}

int main(int argc, char** argv) {
    WABT_TRY
    return ProgramMain(argc, argv);
    WABT_CATCH_BAD_ALLOC_AND_EXIT
}
