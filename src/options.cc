#include "options.h"

wasmati::GenerateCPGOptions wasmati::cpgOptions = {};
std::unique_ptr<wabt::FileStream> wasmati::s_verbose_stream =
    wabt::FileStream::CreateStderr();
