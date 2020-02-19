#ifndef WABT_AST_WRITER_H_
#define WABT_AST_WRITER_H_

#include "src/common.h"
#include "src/stream.h"
#include "src/ir.h"

namespace wasmati {

struct WriteAstOptions {
	bool fold_exprs = false;  // Write folded expressions.
	bool inline_export = false;
	bool inline_import = false;
};

wabt::Result WriteAst(wabt::Stream*, const wabt::Module*, const WriteAstOptions&);

}  // namespace wabt

#endif /* WABT_AST_WRITER_H_ */