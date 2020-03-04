#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>


#include "src/apply-names.h"
#include "src/binary-reader-ir.h"
#include "src/binary-reader.h"
#include "src/error-formatter.h"
#include "src/feature.h"
#include "src/generate-names.h"
#include "src/ir.h"
#include "src/option-parser.h"
#include "src/stream.h"
#include "src/validator.h"
#include "src/wast-lexer.h"
#include "src/decompiler.h"
#include "src/graph.h"
#include "src/dot-writer.h"

using namespace wabt;
using namespace wasmati;

static int s_verbose;
static std::string s_infile;
static std::string s_outfile;
static GenerateCPGOptions cpgOptions;
static Features s_features;
static bool s_read_debug_names = true;
static bool s_fail_on_custom_section_error = true;
static std::unique_ptr<FileStream> s_log_stream;
static bool s_validate = true;

static const char s_description[] =
    R"(  Read a file in the WebAssembly binary format, and convert it to
  the WebAssembly text format.

examples:
  # parse binary file test.wasm and write text file test.wast
  $ wasm2cpg test.wasm -o test.wat

  # parse test.wasm, write test.wat, but ignore the debug names, if any
  $ wasm2cpg test.wasm --no-debug-names -o test.wat
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
  parser.AddOption('f', "function", "FUNCTIONNAME", "Output file for the given function.",
      [](const char* argument) {
          cpgOptions.funcName = argument;
          cpgOptions.funcName = "$" + cpgOptions.funcName;
      });
  s_features.AddOptions(&parser);
  parser.AddOption("no-debug-names", "Ignore debug names in the binary file",
                   []() { s_read_debug_names = false; });
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
  parser.Parse(argc, argv);
}

int ProgramMain(int argc, char** argv) {
	Result result;

	InitStdio();
	ParseOptions(argc, argv);

	std::vector<uint8_t> file_data;
	result = ReadFile(s_infile.c_str(), &file_data);
	if (Succeeded(result)) {
		Errors errors;
		wabt::Module module;
		const bool kStopOnFirstError = true;
		ReadBinaryOptions options(s_features, s_log_stream.get(), s_read_debug_names, kStopOnFirstError,
		                          s_fail_on_custom_section_error);
		result = ReadBinaryIr(s_infile.c_str(), file_data.data(), file_data.size(), options, &errors, &module);
		if (Succeeded(result)) {
			if (Succeeded(result) && s_validate) {
				ValidateOptions options(s_features);
				result = ValidateModule(&module, &errors, options);
			}

			result = GenerateNames(&module);

			if (Succeeded(result)) {
				/* TODO(binji): This shouldn't fail; if a name can't be applied
				 * (because the index is invalid, say) it should just be skipped. */
				Result dummy_result = ApplyNames(&module);
				WABT_USE(dummy_result);
			}
            
            Graph graph(&module);
            graph.generateCPG(cpgOptions);

			if (Succeeded(result)) {
				FileStream stream(!s_outfile.empty() ? FileStream(s_outfile) : FileStream(stdout));
                DotWriter writer(&stream, &graph);
                writer.writeGraph();
			}
		}
		FormatErrorsToFile(errors, Location::Type::Binary);
	}
	return result != Result::Ok;
}

int main(int argc, char** argv) {
	WABT_TRY
	return ProgramMain(argc, argv);
	WABT_CATCH_BAD_ALLOC_AND_EXIT
}
