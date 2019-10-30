#ifndef WABT_AST_WRITER_H_
#define WABT_AST_WRITER_H_

#include "src/common.h"

namespace wabt {

struct Module;
class Stream;

struct WriteAstOptions {
  bool fold_exprs = false;  // Write folded expressions.
  bool inline_export = false;
  bool inline_import = false;
};

Result WriteAst(Stream*, const Module*, const WriteAstOptions&);

}  // namespace wabt

#endif /* WABT_AST_WRITER_H_ */