/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "config.h"

#include "src/ast-writer.h"
#include "src/common.h"
#include "src/error-formatter.h"
#include "src/feature.h"
#include "src/filenames.h"
#include "src/generate-names.h"
#include "src/ir.h"
#include "src/option-parser.h"
#include "src/resolve-names.h"
#include "src/stream.h"
#include "src/validator.h"
#include "src/wast-parser.h"

using namespace wabt;

static int s_verbose;
static std::string s_infile;
static std::string s_outfile;
static Features s_features;
static WriteAstOptions s_write_ast_options;
static bool s_generate_names;
static bool s_read_debug_names = true;
static bool s_fail_on_custom_section_error = true;
static std::unique_ptr<FileStream> s_log_stream;
static bool s_validate = true;

static const char s_description[] =
    R"(  Read a file in the WebAssembly binary format, and convert it to
  the WebAssembly text format.

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
	parser.AddOption('o', "output", "FILENAME", "Output file for the generated wast file, by default use stdout",
	                 [](const char* argument) {
		                 s_outfile = argument;
		                 ConvertBackslashToSlash(&s_outfile);
	                 });
	parser.AddOption('f', "fold-exprs", "Write folded expressions where possible",
	                 []() { s_write_ast_options.fold_exprs = true; });
	s_features.AddOptions(&parser);
	parser.AddOption("inline-exports", "Write all exports inline", []() { s_write_ast_options.inline_export = true; });
	parser.AddOption("inline-imports", "Write all imports inline", []() { s_write_ast_options.inline_import = true; });
	parser.AddOption("no-debug-names", "Ignore debug names in the binary file", []() { s_read_debug_names = false; });
	parser.AddOption("ignore-custom-section-errors", "Ignore errors in custom sections",
	                 []() { s_fail_on_custom_section_error = false; });
	parser.AddOption("generate-names", "Give auto-generated names to non-named functions, types, etc.",
	                 []() { s_generate_names = true; });
	parser.AddOption("no-check", "Don't check for invalid modules", []() { s_validate = false; });
	parser.AddArgument("filename", OptionParser::ArgumentCount::One, [](const char* argument) {
		s_infile = argument;
		ConvertBackslashToSlash(&s_infile);
	});
	parser.Parse(argc, argv);
}

int ProgramMain(int argc, char** argv) {
	InitStdio();

	ParseOptions(argc, argv);

	std::vector<uint8_t> file_data;
	Result result = ReadFile(s_infile, &file_data);
	std::unique_ptr<WastLexer> lexer = WastLexer::CreateBufferLexer(s_infile, file_data.data(), file_data.size());
	if (Failed(result)) {
		WABT_FATAL("unable to read file: %s\n", s_infile.c_str());
	}

	Errors errors;
	std::unique_ptr<Module> module;
	WastParseOptions parse_wast_options(s_features);
	result = ParseWatModule(lexer.get(), &module, &errors, &parse_wast_options);

	if (Succeeded(result)) {
		result = ResolveNamesModule(module.get(), &errors);

		if (Succeeded(result) && s_validate) {
			ValidateOptions options(s_features);
			result = ValidateModule(module.get(), &errors, options);
		}

		if (s_generate_names) {
			result = GenerateNames(module.get());
		}

		if (Succeeded(result)) {
			FileStream stream(!s_outfile.empty() ? FileStream(s_outfile) : FileStream(stdout));
			result = WriteAst(&stream, module.get(), s_write_ast_options);
		}
	}

	auto line_finder = lexer->MakeLineFinder();
	FormatErrorsToFile(errors, Location::Type::Text, line_finder.get());

	return result != Result::Ok;
}

int main(int argc, char** argv) {
	WABT_TRY
	return ProgramMain(argc, argv);
	WABT_CATCH_BAD_ALLOC_AND_EXIT
}
