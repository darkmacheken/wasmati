#include "src/ast-writer.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cinttypes>
#include <cstdarg>
#include <cstdio>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#include "src/cast.h"
#include "src/common.h"
#include "src/expr-visitor.h"
#include "src/ir-util.h"
#include "src/ir.h"
#include "src/literal.h"
#include "src/stream.h"
#include "src/stream.h"

#define WABT_TRACING 0
#include "src/tracing.h"

#define INDENT_SIZE 2
#define NO_FORCE_NEWLINE 0
#define FORCE_NEWLINE 1
using namespace wabt;

namespace wasmati {

namespace {

static const uint8_t s_is_char_escaped[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

// This table matches the characters allowed by wast-lexer.cc for `symbol`.
// The disallowed printable characters are: "(),;[]{} and <space>.
static const uint8_t s_valid_name_chars[256] = {
    //         0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    /* 0x00 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x10 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x20 */ 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1,
    /* 0x30 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
    /* 0x40 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 0x50 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1,
    /* 0x60 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 0x70 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0,
};

enum class NextChar {
  None,
  Space,
  Newline,
  ForceNewline,
};

struct ExprTree {
  explicit ExprTree(const Expr* expr, Index result_count)
      : expr(expr), result_count(result_count) {}

  const Expr* expr;
  std::vector<ExprTree> children;
  Index result_count;
};

class AstWriter : ModuleContext {
 public:
  AstWriter(wabt::Stream* stream,
            const WriteAstOptions& options,
            const wabt::Module& module)
      : ModuleContext(module), options_(options), stream_(stream) {}

  Result WriteModule();

 private:
  void Indent();
  void Dedent();
  void WriteIndent();
  void WriteNextChar();
  void WriteDataWithNextChar(const void* src, size_t size);
  void Writef(const char* format, ...);
  void WritePutc(char c);
  void WritePuts(const char* s, NextChar next_char);
  void WritePutsSpace(const char* s);
  void WritePutsNewline(const char* s);
  void WriteNewline(bool force);
  void WriteOpen(const char* name, NextChar next_char);
  void WriteOpenAttribute(const char* name);
  void WriteOpenNewline(const char* name);
  void WriteOpenSpace(const char* name);
  void WriteOpenTag(const char* name);
  void WriteOpenTag(const char* name, NextChar next_char);
  void WriteOpenTag(const char* name, std::string nameAttribute);
  void WriteOpenTagSemi(const char* name);
  void WriteOpenCloseTag(const char* name);
  void WriteClose(bool end);
  void WriteClose(NextChar next_char);
  void WriteCloseAttribute(NextChar next_char);
  void WriteCloseNewline();
  void WriteCloseSpace();
  void WriteCloseTag(const char* name);
  void WriteString(const std::string& str, NextChar next_char);
  void WriteName(string_view str, NextChar next_char);
  void WriteNameOrIndex(string_view str, Index index, NextChar next_char);
  void WriteQuotedData(const void* data, size_t length);
  void WriteQuotedString(string_view str, NextChar next_char);
  void WriteVar(const Var& var, NextChar next_char);
  void WriteBrVar(const Var& var, NextChar next_char);
  void WriteType(Type type, NextChar next_char);
  void WriteTypes(const TypeVector& types, const char* name);
  void WriteFuncSigSpace(const FuncSignature& func_sig);
  void WriteBeginBlock(LabelType label_type,
                       const Block& block,
                       const char* text);
  void WriteEndBlock(const char* text);
  void WriteConst(const Const& const_);
  void WriteExpr(const Expr* expr);
  template <typename T>
  void WriteLoadStoreExpr(const Expr* expr);
  void WriteExprList(const ExprList& exprs);
  void WriteInitExpr(const ExprList& expr);
  template <typename T>
  void WriteTypeBindings(const char* root_prefix,
                         const char* prefix,
                         const T& types,
                         const std::vector<std::string>& index_to_name,
                         Index binding_index_offset = 0);
  void WriteBeginFunc(const Func& func);
  void WriteFunc(const Func& func);
  void WriteBeginGlobal(const Global& global);
  void WriteGlobal(const Global& global);
  void WriteEvent(const Event& event);
  void WriteLimits(const Limits& limits);
  void WriteTable(const Table& table);
  void WriteElemSegment(const ElemSegment& segment);
  void WriteMemory(const Memory& memory);
  void WriteDataSegment(const DataSegment& segment);
  void WriteImport(const Import& import);
  void WriteExport(const Export& export_);
  void WriteFuncType(const FuncType& func_type);
  void WriteStartFunction(const Var& start);

  class ExprVisitorDelegate;

  void PushExpr(const Expr* expr, Index operand_count, Index result_count);
  void FlushExprTree(const ExprTree& expr_tree);
  void FlushExprTreeVector(const std::vector<ExprTree>&);
  void FlushExprTreeStack();
  void WriteFoldedExpr(const Expr*);
  void WriteFoldedExprList(const ExprList&);

  void BuildInlineExportMap();
  void WriteInlineExports(ExternalKind, Index);
  bool IsInlineExport(const Export& export_);
  void BuildInlineImportMap();
  void WriteInlineImport(ExternalKind, Index);

  const WriteAstOptions& options_;
  wabt::Stream* stream_ = nullptr;
  Result result_ = Result::Ok;
  int indent_ = 0;
  NextChar next_char_ = NextChar::None;
  std::vector<ExprTree> expr_tree_stack_;
  std::multimap<std::pair<ExternalKind, Index>, const Export*>
      inline_export_map_;
  std::vector<const Import*> inline_import_map_[kExternalKindCount];

  Index func_index_ = 0;
  Index global_index_ = 0;
  Index table_index_ = 0;
  Index memory_index_ = 0;
  Index func_type_index_ = 0;
  Index event_index_ = 0;
  Index data_segment_index_ = 0;
  Index elem_segment_index_ = 0;
};

void AstWriter::Indent() {
  indent_ += INDENT_SIZE;
}

void AstWriter::Dedent() {
  indent_ -= INDENT_SIZE;
  assert(indent_ >= 0);
}

void AstWriter::WriteIndent() {
  static char s_indent[] =
      "                                                                       "
      "                                                                       ";
  static size_t s_indent_len = sizeof(s_indent) - 1;
  size_t to_write = indent_;
  while (to_write >= s_indent_len) {
    stream_->WriteData(s_indent, s_indent_len);
    to_write -= s_indent_len;
  }
  if (to_write > 0) {
    stream_->WriteData(s_indent, to_write);
  }
}

void AstWriter::WriteNextChar() {
  switch (next_char_) {
    case NextChar::Space:
      stream_->WriteChar(' ');
      break;
    case NextChar::Newline:
    case NextChar::ForceNewline:
      stream_->WriteChar('\n');
      WriteIndent();
      break;
    case NextChar::None:
      break;
  }
  next_char_ = NextChar::None;
}

void AstWriter::WriteDataWithNextChar(const void* src, size_t size) {
  WriteNextChar();
  stream_->WriteData(src, size);
}

void WABT_PRINTF_FORMAT(2, 3) AstWriter::Writef(const char* format, ...) {
  WABT_SNPRINTF_ALLOCA(buffer, length, format);
  /* default to following space */
  WriteDataWithNextChar(buffer, length);
}

void AstWriter::WritePutc(char c) {
  stream_->WriteChar(c);
}

void AstWriter::WritePuts(const char* s, NextChar next_char) {
  size_t len = strlen(s);
  WriteDataWithNextChar(s, len);
  next_char_ = next_char;
}

void AstWriter::WritePutsSpace(const char* s) {
  WritePuts(s, NextChar::Space);
}

void AstWriter::WritePutsNewline(const char* s) {
  WritePuts(s, NextChar::Newline);
}

void AstWriter::WriteNewline(bool force) {
  if (next_char_ == NextChar::ForceNewline) {
    WriteNextChar();
  }
  next_char_ = force ? NextChar::ForceNewline : NextChar::Newline;
}

void AstWriter::WriteOpen(const char* name, NextChar next_char) {
  WritePuts("(", NextChar::None);
  WritePuts(name, next_char);
  Indent();
}

void AstWriter::WriteOpenAttribute(const char* name) {
  WritePuts(name, NextChar::None);
  WritePuts("=\"", NextChar::None);
}

void AstWriter::WriteOpenNewline(const char* name) {
  WriteOpen(name, NextChar::Newline);
}

void AstWriter::WriteOpenSpace(const char* name) {
  WriteOpen(name, NextChar::Space);
}

void AstWriter::WriteOpenTag(const char* name) {
  WritePuts("<", NextChar::None);
  WritePuts(name, NextChar::None);
  WritePuts(">", NextChar::Newline);
  Indent();
}

void AstWriter::WriteOpenTag(const char* name, NextChar next_char) {
  WritePuts("<", NextChar::None);
  WritePuts(name, NextChar::None);
  WritePuts(">", next_char);
  Indent();
}

void AstWriter::WriteOpenTag(const char* name, std::string nameAttribute) {
  WritePuts("<", NextChar::None);
  WritePuts(name, NextChar::Space);
  WritePuts("name=\"", NextChar::None);
  WriteName(nameAttribute, NextChar::None);
  WritePuts("\">", NextChar::Newline);
  Indent();
}

void AstWriter::WriteOpenTagSemi(const char* name) {
  WritePuts("<", NextChar::None);
  WritePuts(name, NextChar::Space);
}

void AstWriter::WriteOpenCloseTag(const char* name) {
  WritePuts("<", NextChar::None);
  WritePuts(name, NextChar::None);
  WritePuts("/>", NextChar::Newline);
}

void AstWriter::WriteClose(bool end) {
  Indent();
  if (end) {
    WritePuts("/>", NextChar::Newline);
    Dedent();
  } else {
    WritePuts(">", NextChar::Newline);
  }
}

void AstWriter::WriteClose(NextChar next_char) {
  if (next_char_ != NextChar::ForceNewline) {
    next_char_ = NextChar::None;
  }
  Dedent();
  WritePuts(")", next_char);
}

void AstWriter::WriteCloseAttribute(NextChar next_char) {
  if (next_char_ != NextChar::ForceNewline) {
    next_char_ = NextChar::None;
  }
  WritePuts("\"", next_char);
}

void AstWriter::WriteCloseNewline() {
  WriteClose(NextChar::Newline);
}

void AstWriter::WriteCloseSpace() {
  WriteClose(NextChar::Space);
}

void AstWriter::WriteCloseTag(const char* name) {
  Dedent();
  WritePuts("</", NextChar::None);
  WritePuts(name, NextChar::None);
  WritePuts(">", NextChar::Newline);
}

void AstWriter::WriteString(const std::string& str, NextChar next_char) {
  WritePuts(str.c_str(), next_char);
}

void AstWriter::WriteName(string_view str, NextChar next_char) {
  // Debug names must begin with a $ for for wast file to be valid
  assert(!str.empty() && str.front() == '$');
  bool has_invalid_chars = std::any_of(
      str.begin(), str.end(), [](uint8_t c) { return !s_valid_name_chars[c]; });

  if (has_invalid_chars) {
    std::string valid_str;
    std::transform(str.begin(), str.end(), std::back_inserter(valid_str),
                   [](uint8_t c) { return s_valid_name_chars[c] ? c : '_'; });
    WriteDataWithNextChar(valid_str.data(), valid_str.length());
  } else {
    WriteDataWithNextChar(str.data(), str.length());
  }

  next_char_ = next_char;
}

void AstWriter::WriteNameOrIndex(string_view str,
                                 Index index,
                                 NextChar next_char) {
  if (!str.empty()) {
    WriteName(str, next_char);
  } else {
    Writef("(;%u;)", index);
  }
}

void AstWriter::WriteQuotedData(const void* data, size_t length) {
  const uint8_t* u8_data = static_cast<const uint8_t*>(data);
  static const char s_hexdigits[] = "0123456789abcdef";
  WriteNextChar();
  WritePutc('\"');
  for (size_t i = 0; i < length; ++i) {
    uint8_t c = u8_data[i];
    if (s_is_char_escaped[c]) {
      WritePutc('\\');
      WritePutc(s_hexdigits[c >> 4]);
      WritePutc(s_hexdigits[c & 0xf]);
    } else {
      WritePutc(c);
    }
  }
  WritePutc('\"');
  next_char_ = NextChar::Space;
}

void AstWriter::WriteQuotedString(string_view str, NextChar next_char) {
  WriteQuotedData(str.data(), str.length());
  next_char_ = next_char;
}

void AstWriter::WriteVar(const Var& var, NextChar next_char) {
  if (var.is_index()) {
    WriteOpenTag("natural", NextChar::None);
    Writef("%" PRIindex, var.index());
    WriteCloseTag("natural");

    next_char_ = next_char;
  } else {
    WriteOpenTag("name", NextChar::None);
    WriteName(var.name(), next_char);
    WriteCloseTag("name");
  }
}

void AstWriter::WriteBrVar(const Var& var, NextChar next_char) {
  if (var.is_index()) {
    // if (var.index() < GetLabelStackSize()) {
    //   Writef("%" PRIindex " (;@%" PRIindex ";)", var.index(),
    //          GetLabelStackSize() - var.index() - 1);
    // } else {
    //   Writef("%" PRIindex " (; INVALID ;)", var.index());
    // }
    WriteOpenTag("natural", NextChar::None);
    Writef("%" PRIindex, var.index());
    WriteCloseTag("natural");

    next_char_ = next_char;
  } else {
    WriteOpenTag("name", NextChar::None);
    WriteString(var.name(), next_char);
    WriteCloseTag("name");
  }
}

void AstWriter::WriteType(Type type, NextChar next_char) {
  const char* type_name = GetTypeName(type);
  assert(type_name);
  WritePuts(type_name, next_char);
}

void AstWriter::WriteTypes(const TypeVector& types, const char* name) {
  if (types.size()) {
    for (Type type : types) {
      if (name) {
        WriteOpenTagSemi(name);
      }
      WriteOpenAttribute("type");
      WriteType(type, NextChar::None);
      WriteCloseAttribute(NextChar::None);
      if (name) {
        WriteClose(true);
      }
    }
  }
}

void AstWriter::WriteFuncSigSpace(const FuncSignature& func_sig) {
  WriteTypes(func_sig.param_types, "param");
  WriteTypes(func_sig.result_types, "result");
}

void AstWriter::WriteBeginBlock(LabelType label_type,
                                const Block& block,
                                const char* text) {
  WriteOpenTag(text);
  // bool has_label = !block.label.empty();
  // if (has_label) {
  //   WriteString(block.label, NextChar::Space);
  // }
  WriteTypes(block.decl.sig.param_types, "param");
  WriteTypes(block.decl.sig.result_types, "result");
  // if (!has_label) {
  //   Writef(" ;; label = @%" PRIindex, GetLabelStackSize());
  // }
  WriteNewline(FORCE_NEWLINE);
  BeginBlock(label_type, block);
}

void AstWriter::WriteEndBlock(const char* text) {
  EndBlock();
  WriteCloseTag(text);
}

void AstWriter::WriteConst(const Const& const_) {
  switch (const_.type) {
    case Type::I32:
      WriteOpenTag(Opcode::I32Const_Opcode.GetName());
      WriteOpenTag("integer", NextChar::None);
      Writef("%d", static_cast<int32_t>(const_.u32));
      WriteCloseTag("integer");
      WriteCloseTag(Opcode::I32Const_Opcode.GetName());
      break;

    case Type::I64:
      WriteOpenTag(Opcode::I64Const_Opcode.GetName());
      WriteOpenTag("integer", NextChar::None);
      Writef("%" PRId64, static_cast<int64_t>(const_.u64));
      WriteCloseTag("integer");
      WriteCloseTag(Opcode::I64Const_Opcode.GetName());
      break;

    case Type::F32: {
      WriteOpenTag(Opcode::F32Const_Opcode.GetName());
      WriteOpenTagSemi("float");
      WriteOpenAttribute("decimalRepresentation");
      float f32;
      memcpy(&f32, &const_.f32_bits, sizeof(f32));
      Writef("%g", f32);
      WriteCloseAttribute(NextChar::None);
      WriteClose(false);

      char buffer[128];
      WriteFloatHex(buffer, 128, const_.f32_bits);
      WritePutsNewline(buffer);
      WriteCloseTag("float");
      WriteCloseTag(Opcode::F32Const_Opcode.GetName());
      break;
    }

    case Type::F64: {
      WriteOpenTag(Opcode::F64Const_Opcode.GetName());
      WriteOpenTagSemi("float");
      WriteOpenAttribute("decimalRepresentation");
      double f64;
      memcpy(&f64, &const_.f64_bits, sizeof(f64));
      Writef("%g", f64);
      WriteCloseAttribute(NextChar::None);
      WriteClose(false);

      char buffer[128];
      WriteDoubleHex(buffer, 128, const_.f64_bits);
      WritePutsNewline(buffer);
      WriteCloseTag("float");
      WriteCloseTag(Opcode::F64Const_Opcode.GetName());
      break;
    }

    case Type::V128: {
      WritePutsSpace(Opcode::V128Const_Opcode.GetName());
      Writef("i32x4 0x%08x 0x%08x 0x%08x 0x%08x", const_.vec128.v[0],
             const_.vec128.v[1], const_.vec128.v[2],
             const_.vec128.v[3]);
      WriteNewline(NO_FORCE_NEWLINE);
      break;
    }

    default:
      assert(0);
      break;
  }
}

template <typename T>
void AstWriter::WriteLoadStoreExpr(const Expr* expr) {
  auto typed_expr = cast<T>(expr);
  WriteOpenTagSemi(typed_expr->opcode.GetName());
  if (typed_expr->offset) {
    WriteOpenAttribute("offset");
    Writef("%u", typed_expr->offset);
    WriteCloseAttribute(NextChar::Space);
  }
  if (!typed_expr->opcode.IsNaturallyAligned(typed_expr->align)) {
    WriteOpenAttribute("align");
    Writef("%u", typed_expr->align);
    WriteCloseAttribute(NextChar::None);
  }
  WriteClose(true);
}

class AstWriter::ExprVisitorDelegate : public ExprVisitor::Delegate {
 public:
  explicit ExprVisitorDelegate(AstWriter* writer) : writer_(writer) {}

  Result OnBinaryExpr(BinaryExpr*) override;
  Result BeginBlockExpr(BlockExpr*) override;
  Result EndBlockExpr(BlockExpr*) override;
  Result OnBrExpr(BrExpr*) override;
  Result OnBrIfExpr(BrIfExpr*) override;
  Result OnBrOnExnExpr(BrOnExnExpr*) override;
  Result OnBrTableExpr(BrTableExpr*) override;
  Result OnCallExpr(CallExpr*) override;
  Result OnCallIndirectExpr(CallIndirectExpr*) override;
  Result OnCompareExpr(CompareExpr*) override;
  Result OnConstExpr(ConstExpr*) override;
  Result OnConvertExpr(ConvertExpr*) override;
  Result OnDropExpr(DropExpr*) override;
  Result OnGlobalGetExpr(GlobalGetExpr*) override;
  Result OnGlobalSetExpr(GlobalSetExpr*) override;
  Result BeginIfExpr(IfExpr*) override;
  Result AfterIfTrueExpr(IfExpr*) override;
  Result EndIfExpr(IfExpr*) override;
  Result OnLoadExpr(LoadExpr*) override;
  Result OnLocalGetExpr(LocalGetExpr*) override;
  Result OnLocalSetExpr(LocalSetExpr*) override;
  Result OnLocalTeeExpr(LocalTeeExpr*) override;
  Result BeginLoopExpr(LoopExpr*) override;
  Result EndLoopExpr(LoopExpr*) override;
  Result OnMemoryCopyExpr(MemoryCopyExpr*) override;
  Result OnDataDropExpr(DataDropExpr*) override;
  Result OnMemoryFillExpr(MemoryFillExpr*) override;
  Result OnMemoryGrowExpr(MemoryGrowExpr*) override;
  Result OnMemoryInitExpr(MemoryInitExpr*) override;
  Result OnMemorySizeExpr(MemorySizeExpr*) override;
  Result OnTableCopyExpr(TableCopyExpr*) override;
  Result OnElemDropExpr(ElemDropExpr*) override;
  Result OnTableInitExpr(TableInitExpr*) override;
  Result OnTableGetExpr(TableGetExpr*) override;
  Result OnTableSetExpr(TableSetExpr*) override;
  Result OnTableGrowExpr(TableGrowExpr*) override;
  Result OnTableSizeExpr(TableSizeExpr*) override;
  Result OnTableFillExpr(TableFillExpr*) override;
  Result OnRefFuncExpr(RefFuncExpr*) override;
  Result OnRefNullExpr(RefNullExpr*) override;
  Result OnRefIsNullExpr(RefIsNullExpr*) override;
  Result OnNopExpr(NopExpr*) override;
  Result OnReturnExpr(ReturnExpr*) override;
  Result OnReturnCallExpr(ReturnCallExpr*) override;
  Result OnReturnCallIndirectExpr(ReturnCallIndirectExpr*) override;
  Result OnSelectExpr(SelectExpr*) override;
  Result OnStoreExpr(StoreExpr*) override;
  Result OnUnaryExpr(UnaryExpr*) override;
  Result OnUnreachableExpr(UnreachableExpr*) override;
  Result BeginTryExpr(TryExpr*) override;
  Result OnCatchExpr(TryExpr*) override;
  Result EndTryExpr(TryExpr*) override;
  Result OnThrowExpr(ThrowExpr*) override;
  Result OnRethrowExpr(RethrowExpr*) override;
  Result OnAtomicWaitExpr(AtomicWaitExpr*) override;
  Result OnAtomicNotifyExpr(AtomicNotifyExpr*) override;
  Result OnAtomicLoadExpr(AtomicLoadExpr*) override;
  Result OnAtomicStoreExpr(AtomicStoreExpr*) override;
  Result OnAtomicRmwExpr(AtomicRmwExpr*) override;
  Result OnAtomicRmwCmpxchgExpr(AtomicRmwCmpxchgExpr*) override;
  Result OnTernaryExpr(TernaryExpr*) override;
  Result OnSimdLaneOpExpr(SimdLaneOpExpr*) override;
  Result OnSimdShuffleOpExpr(SimdShuffleOpExpr*) override;
  Result OnLoadSplatExpr(LoadSplatExpr*) override;

 private:
  AstWriter* writer_;
};

Result AstWriter::ExprVisitorDelegate::OnBinaryExpr(BinaryExpr* expr) {
  if (writer_->options_.fold_exprs) {
    writer_->WriteOpenTag(expr->opcode.GetName());
  } else {
    writer_->WriteOpenCloseTag(expr->opcode.GetName());
  }
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::BeginBlockExpr(BlockExpr* expr) {
  writer_->WriteBeginBlock(LabelType::Block, expr->block,
                           Opcode::Block_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::EndBlockExpr(BlockExpr* expr) {
  writer_->WriteEndBlock(Opcode::Block_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnBrExpr(BrExpr* expr) {
  writer_->WriteOpenTag(Opcode::Br_Opcode.GetName());
  writer_->WriteBrVar(expr->var, NextChar::Newline);
  writer_->WriteCloseTag(Opcode::Br_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnBrIfExpr(BrIfExpr* expr) {
  writer_->WriteOpenTag(Opcode::BrIf_Opcode.GetName());
  writer_->WriteBrVar(expr->var, NextChar::Newline);
  writer_->WriteCloseTag(Opcode::BrIf_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnBrOnExnExpr(BrOnExnExpr* expr) {
  writer_->WritePutsSpace(Opcode::BrOnExn_Opcode.GetName());
  writer_->WriteBrVar(expr->label_var, NextChar::Space);
  writer_->WriteVar(expr->event_var, NextChar::Newline);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnBrTableExpr(BrTableExpr* expr) {
  writer_->WritePutsSpace(Opcode::BrTable_Opcode.GetName());
  for (const Var& var : expr->targets) {
    writer_->WriteBrVar(var, NextChar::Space);
  }
  writer_->WriteBrVar(expr->default_target, NextChar::Newline);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnCallExpr(CallExpr* expr) {
  writer_->WriteOpenTag(Opcode::Call_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::None);
  writer_->WriteCloseTag(Opcode::Call_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnCallIndirectExpr(
    CallIndirectExpr* expr) {
  writer_->WritePutsSpace(Opcode::CallIndirect_Opcode.GetName());
  writer_->WriteOpenSpace("type");
  writer_->WriteVar(expr->decl.type_var, NextChar::Space);

  if (expr->table.is_index() && expr->table.index() == 0) {
    writer_->WriteCloseNewline();
  } else {
    writer_->WriteCloseSpace();
    writer_->WriteVar(expr->table, NextChar::Newline);
  }
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnCompareExpr(CompareExpr* expr) {
  if (writer_->options_.fold_exprs) {
    writer_->WriteOpenTag(expr->opcode.GetName());
  } else {
    writer_->WriteOpenCloseTag(expr->opcode.GetName());
  }
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnConstExpr(ConstExpr* expr) {
  writer_->WriteConst(expr->const_);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnConvertExpr(ConvertExpr* expr) {
  writer_->WriteOpenCloseTag(expr->opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnDropExpr(DropExpr* expr) {
  writer_->WriteOpenCloseTag(Opcode::Drop_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnGlobalGetExpr(GlobalGetExpr* expr) {
  writer_->WriteOpenTag(Opcode::GlobalGet_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  writer_->WriteCloseTag(Opcode::GlobalGet_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnGlobalSetExpr(GlobalSetExpr* expr) {
  writer_->WriteOpenTag(Opcode::GlobalSet_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  writer_->WriteCloseTag(Opcode::GlobalSet_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::BeginIfExpr(IfExpr* expr) {
  writer_->WriteBeginBlock(LabelType::If, expr->true_,
                           Opcode::If_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::AfterIfTrueExpr(IfExpr* expr) {
  if (!expr->false_.empty()) {
    writer_->WriteOpenTag(Opcode::Else_Opcode.GetName());
  }
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::EndIfExpr(IfExpr* expr) {
  if (!expr->false_.empty()) {
    writer_->WriteCloseTag(Opcode::Else_Opcode.GetName());
  }
  writer_->WriteEndBlock(Opcode::If_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnLoadExpr(LoadExpr* expr) {
  writer_->WriteLoadStoreExpr<LoadExpr>(expr);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnLocalGetExpr(LocalGetExpr* expr) {
  writer_->WriteOpenTag(Opcode::LocalGet_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  writer_->WriteCloseTag(Opcode::LocalGet_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnLocalSetExpr(LocalSetExpr* expr) {
  writer_->WriteOpenTag(Opcode::LocalSet_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  writer_->WriteCloseTag(Opcode::LocalSet_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnLocalTeeExpr(LocalTeeExpr* expr) {
  writer_->WriteOpenTag(Opcode::LocalTee_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  writer_->WriteCloseTag(Opcode::LocalTee_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::BeginLoopExpr(LoopExpr* expr) {
  writer_->WriteBeginBlock(LabelType::Loop, expr->block,
                           Opcode::Loop_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::EndLoopExpr(LoopExpr* expr) {
  writer_->WriteEndBlock(Opcode::Loop_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnMemoryCopyExpr(MemoryCopyExpr* expr) {
  writer_->WritePutsNewline(Opcode::MemoryCopy_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnDataDropExpr(DataDropExpr* expr) {
  writer_->WriteOpenTag(Opcode::DataDrop_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  writer_->WriteCloseTag(Opcode::DataDrop_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnMemoryFillExpr(MemoryFillExpr* expr) {
  writer_->WritePutsNewline(Opcode::MemoryFill_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnMemoryGrowExpr(MemoryGrowExpr* expr) {
  writer_->WritePutsNewline(Opcode::MemoryGrow_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnMemorySizeExpr(MemorySizeExpr* expr) {
  writer_->WritePutsNewline(Opcode::MemorySize_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnMemoryInitExpr(MemoryInitExpr* expr) {
  writer_->WritePutsSpace(Opcode::MemoryInit_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnTableCopyExpr(TableCopyExpr* expr) {
  writer_->WritePutsNewline(Opcode::TableCopy_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnElemDropExpr(ElemDropExpr* expr) {
  writer_->WritePutsSpace(Opcode::ElemDrop_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnTableInitExpr(TableInitExpr* expr) {
  writer_->WritePutsSpace(Opcode::TableInit_Opcode.GetName());
  writer_->WriteVar(expr->segment_index, NextChar::Newline);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnTableGetExpr(TableGetExpr* expr) {
  writer_->WritePutsSpace(Opcode::TableGet_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnTableSetExpr(TableSetExpr* expr) {
  writer_->WritePutsSpace(Opcode::TableSet_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnTableGrowExpr(TableGrowExpr* expr) {
  writer_->WritePutsSpace(Opcode::TableGrow_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnTableSizeExpr(TableSizeExpr* expr) {
  writer_->WritePutsSpace(Opcode::TableSize_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnTableFillExpr(TableFillExpr* expr) {
  writer_->WritePutsSpace(Opcode::TableFill_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnRefFuncExpr(RefFuncExpr* expr) {
  writer_->WritePutsSpace(Opcode::RefFunc_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnRefNullExpr(RefNullExpr* expr) {
  writer_->WritePutsNewline(Opcode::RefNull_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnRefIsNullExpr(RefIsNullExpr* expr) {
  writer_->WritePutsNewline(Opcode::RefIsNull_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnNopExpr(NopExpr* expr) {
  writer_->WriteOpenCloseTag(Opcode::Nop_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnReturnExpr(ReturnExpr* expr) {
  writer_->WriteOpenCloseTag(Opcode::Return_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnReturnCallExpr(ReturnCallExpr* expr) {
  writer_->WritePutsSpace(Opcode::ReturnCall_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnReturnCallIndirectExpr(
    ReturnCallIndirectExpr* expr) {
  writer_->WritePutsSpace(Opcode::ReturnCallIndirect_Opcode.GetName());
  writer_->WriteOpenSpace("type");
  writer_->WriteVar(expr->decl.type_var, NextChar::Space);
  writer_->WriteCloseNewline();
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnSelectExpr(SelectExpr* expr) {
  writer_->WritePutsNewline(Opcode::Select_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnStoreExpr(StoreExpr* expr) {
  writer_->WriteLoadStoreExpr<StoreExpr>(expr);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnUnaryExpr(UnaryExpr* expr) {
  writer_->WriteOpenCloseTag(expr->opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnUnreachableExpr(
    UnreachableExpr* expr) {
  writer_->WritePutsNewline(Opcode::Unreachable_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::BeginTryExpr(TryExpr* expr) {
  writer_->WriteBeginBlock(LabelType::Try, expr->block,
                           Opcode::Try_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnCatchExpr(TryExpr* expr) {
  writer_->Dedent();
  writer_->WritePutsSpace(Opcode::Catch_Opcode.GetName());
  writer_->Indent();
  writer_->SetTopLabelType(LabelType::Catch);
  writer_->WriteNewline(FORCE_NEWLINE);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::EndTryExpr(TryExpr* expr) {
  writer_->WriteEndBlock(Opcode::Try_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnThrowExpr(ThrowExpr* expr) {
  writer_->WritePutsSpace(Opcode::Throw_Opcode.GetName());
  writer_->WriteVar(expr->var, NextChar::Newline);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnRethrowExpr(RethrowExpr* expr) {
  writer_->WritePutsSpace(Opcode::Rethrow_Opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnAtomicWaitExpr(AtomicWaitExpr* expr) {
  writer_->WriteLoadStoreExpr<AtomicWaitExpr>(expr);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnAtomicNotifyExpr(
    AtomicNotifyExpr* expr) {
  writer_->WriteLoadStoreExpr<AtomicNotifyExpr>(expr);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnAtomicLoadExpr(AtomicLoadExpr* expr) {
  writer_->WriteLoadStoreExpr<AtomicLoadExpr>(expr);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnAtomicStoreExpr(
    AtomicStoreExpr* expr) {
  writer_->WriteLoadStoreExpr<AtomicStoreExpr>(expr);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnAtomicRmwExpr(AtomicRmwExpr* expr) {
  writer_->WriteLoadStoreExpr<AtomicRmwExpr>(expr);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnAtomicRmwCmpxchgExpr(
    AtomicRmwCmpxchgExpr* expr) {
  writer_->WriteLoadStoreExpr<AtomicRmwCmpxchgExpr>(expr);
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnTernaryExpr(TernaryExpr* expr) {
  writer_->WritePutsNewline(expr->opcode.GetName());
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnSimdLaneOpExpr(SimdLaneOpExpr* expr) {
  writer_->WritePutsSpace(expr->opcode.GetName());
  writer_->Writef("%" PRIu64, (expr->val));
  writer_->WritePutsNewline("");
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnSimdShuffleOpExpr(
    SimdShuffleOpExpr* expr) {
  writer_->WritePutsSpace(expr->opcode.GetName());
  std::array<uint8_t, 16> values = Bitcast<std::array<uint8_t, 16>>(expr->val);
  for (int32_t lane = 0; lane < 16; ++lane) {
    writer_->Writef(" %u", values[lane]);
  }
  writer_->WritePutsNewline("");
  return Result::Ok;
}

Result AstWriter::ExprVisitorDelegate::OnLoadSplatExpr(LoadSplatExpr* expr) {
  writer_->WriteLoadStoreExpr<LoadSplatExpr>(expr);
  return Result::Ok;
}

void AstWriter::WriteExpr(const Expr* expr) {
  WABT_TRACE(WriteExprList);
  ExprVisitorDelegate delegate(this);
  ExprVisitor visitor(&delegate);
  visitor.VisitExpr(const_cast<Expr*>(expr));
}

void AstWriter::WriteExprList(const ExprList& exprs) {
  WABT_TRACE(WriteExprList);
  ExprVisitorDelegate delegate(this);
  ExprVisitor visitor(&delegate);
  visitor.VisitExprList(const_cast<ExprList&>(exprs));
}

void AstWriter::WriteFoldedExpr(const Expr* expr) {
  WABT_TRACE_ARGS(WriteFoldedExpr, "%s", GetExprTypeName(*expr));
  auto arity = GetExprArity(*expr);
  PushExpr(expr, arity.nargs, arity.nreturns);
}

void AstWriter::WriteFoldedExprList(const ExprList& exprs) {
  WABT_TRACE(WriteFoldedExprList);
  for (const Expr& expr : exprs) {
    WriteFoldedExpr(&expr);
  }
}

void AstWriter::PushExpr(const Expr* expr,
                         Index operand_count,
                         Index result_count) {
  WABT_TRACE_ARGS(PushExpr, "%s, %" PRIindex ", %" PRIindex "",
                  GetExprTypeName(*expr), operand_count, result_count);

  // Try to pop operand off the expr stack to use as this expr's children. One
  // expr can have multiple return values (with the multi-value extension), so
  // we have to iterate over each in reverse.

  auto first_operand = expr_tree_stack_.end();

  Index current_count = 0;
  if (operand_count > 0) {
    for (auto iter = expr_tree_stack_.rbegin(); iter != expr_tree_stack_.rend();
         ++iter) {
      assert(iter->result_count > 0);
      current_count += iter->result_count;

      if (current_count == operand_count) {
        first_operand = iter.base() - 1;
        break;
      } else if (current_count > operand_count) {
        // We went over the number of operands this instruction wants; this can
        // only happen when there are expressions on the stack with a
        // result_count > 1. When this happens we can't fold, since any result
        // we produce will not make sense.
        break;
      }
    }
  }

  ExprTree tree(expr, result_count);

  if (current_count == operand_count && operand_count > 0) {
    auto last_operand = expr_tree_stack_.end();
    std::move(first_operand, last_operand, std::back_inserter(tree.children));
    expr_tree_stack_.erase(first_operand, last_operand);
  }

  expr_tree_stack_.emplace_back(std::move(tree));
  if (current_count > operand_count || result_count == 0) {
    FlushExprTreeStack();
  }
}

void AstWriter::FlushExprTree(const ExprTree& expr_tree) {
  WABT_TRACE_ARGS(FlushExprTree, "%s", GetExprTypeName(*expr_tree.expr));
  switch (expr_tree.expr->type()) {
    case ExprType::Block:
      WritePuts("(", NextChar::None);
      WriteBeginBlock(LabelType::Block, cast<BlockExpr>(expr_tree.expr)->block,
                      Opcode::Block_Opcode.GetName());
      WriteFoldedExprList(cast<BlockExpr>(expr_tree.expr)->block.exprs);
      FlushExprTreeStack();
      WriteCloseNewline();
      break;

    case ExprType::Loop:
      WritePuts("(", NextChar::None);
      WriteBeginBlock(LabelType::Loop, cast<LoopExpr>(expr_tree.expr)->block,
                      Opcode::Loop_Opcode.GetName());
      WriteFoldedExprList(cast<LoopExpr>(expr_tree.expr)->block.exprs);
      FlushExprTreeStack();
      WriteCloseNewline();
      break;

    case ExprType::If: {
      auto if_expr = cast<IfExpr>(expr_tree.expr);
      WritePuts("(", NextChar::None);
      WriteBeginBlock(LabelType::If, if_expr->true_,
                      Opcode::If_Opcode.GetName());
      FlushExprTreeVector(expr_tree.children);
      WriteOpenNewline("then");
      WriteFoldedExprList(if_expr->true_.exprs);
      FlushExprTreeStack();
      WriteCloseNewline();
      if (!if_expr->false_.empty()) {
        WriteOpenNewline("else");
        WriteFoldedExprList(if_expr->false_);
        FlushExprTreeStack();
        WriteCloseNewline();
      }
      WriteCloseNewline();
      break;
    }

    case ExprType::Try: {
      auto try_expr = cast<TryExpr>(expr_tree.expr);
      WritePuts("(", NextChar::None);
      WriteBeginBlock(LabelType::Try, try_expr->block,
                      Opcode::Try_Opcode.GetName());
      FlushExprTreeVector(expr_tree.children);
      WriteFoldedExprList(try_expr->block.exprs);
      FlushExprTreeStack();
      WriteOpenNewline("catch");
      WriteFoldedExprList(try_expr->catch_);
      FlushExprTreeStack();
      WriteCloseNewline();
      WriteCloseNewline();
      break;
    }

    default: {
      WritePuts("(", NextChar::None);
      WriteExpr(expr_tree.expr);
      Indent();
      FlushExprTreeVector(expr_tree.children);
      WriteCloseNewline();
      break;
    }
  }
}

void AstWriter::FlushExprTreeVector(const std::vector<ExprTree>& expr_trees) {
  WABT_TRACE_ARGS(FlushExprTreeVector, "%zu", expr_trees.size());
  for (auto expr_tree : expr_trees) {
    FlushExprTree(expr_tree);
  }
}

void AstWriter::FlushExprTreeStack() {
  std::vector<ExprTree> stack_copy(std::move(expr_tree_stack_));
  expr_tree_stack_.clear();
  FlushExprTreeVector(stack_copy);
}

void AstWriter::WriteInitExpr(const ExprList& expr) {
  if (!expr.empty()) {
    WritePuts("(", NextChar::None);
    WriteExprList(expr);
    /* clear the next char, so we don't write a newline after the expr */
    next_char_ = NextChar::None;
    WritePuts(")", NextChar::Space);
  }
}

template <typename T>
void AstWriter::WriteTypeBindings(const char* root_prefix,
                                  const char* prefix,
                                  const T& types,
                                  const std::vector<std::string>& index_to_name,
                                  Index binding_index_offset) {
  /* named params/locals must be specified by themselves, but nameless
   * params/locals can be compressed, e.g.:
   *   (param $foo i32)
   *   (param i32 i64 f32)
   */
  WriteOpenTag(root_prefix);
  size_t index = 0;
  for (Type type : types) {
    WriteOpenTagSemi(prefix);
    const std::string& name = index_to_name[binding_index_offset + index];
    if (name.empty()) {
      WriteOpenAttribute("index");
      WritePuts(std::to_string(index).c_str(), NextChar::None);
      WriteCloseAttribute(NextChar::Space);
    } else {
      WriteOpenAttribute("name");
      WriteString(name, NextChar::Space);
      WriteCloseAttribute(NextChar::Space);
    }

    WriteOpenAttribute("type");
    WriteType(type, NextChar::Space);
    WriteCloseAttribute(NextChar::None);

    ++index;
    WriteClose(true);
  }
  WriteCloseTag(root_prefix);
}

void AstWriter::WriteBeginFunc(const Func& func) {
  WriteOpenTagSemi("function");
  WriteOpenAttribute("nameOrIndex");
  WriteNameOrIndex(func.name, func_index_, NextChar::Space);
  WriteCloseAttribute(NextChar::Space);

  WriteInlineExports(ExternalKind::Func, func_index_);
  WriteInlineImport(ExternalKind::Func, func_index_);
  WriteClose(false);
  if (func.decl.has_func_type) {
    WriteOpenTag("typeIndex");
    WriteVar(func.decl.type_var, NextChar::Newline);
    WriteCloseTag("typeIndex");
  }

  // if (module.IsImport(ExternalKind::Func, Var(func_index_))) {
  //   // Imported functions can be written a few ways:
  //   //
  //   //   1. (import "module" "field" (func (type 0)))
  //   //   2. (import "module" "field" (func (param i32) (result i32)))
  //   //   3. (func (import "module" "field") (type 0))
  //   //   4. (func (import "module" "field") (param i32) (result i32))
  //   //   5. (func (import "module" "field") (type 0) (param i32) (result
  //   i32))
  //   //
  //   // Note that the text format does not allow including the param/result
  //   // explicitly when using the "(import..." syntax (#1 and #2).
  //   if (options_.inline_import || !func.decl.has_func_type) {
  //     WriteFuncSigSpace(func.decl.sig);
  //   }
  // }
  func_index_++;
}

void AstWriter::WriteFunc(const Func& func) {
  WriteBeginFunc(func);

  std::vector<std::string> index_to_name;
  MakeTypeBindingReverseMapping(func.GetNumParamsAndLocals(), func.bindings,
                                &index_to_name);

  WriteOpenTag("functionSignature");
  WriteTypeBindings("parameters", "parameter", func.decl.sig.param_types,
                    index_to_name);

  WriteTypes(func.decl.sig.result_types, "result");
  WriteCloseTag("functionSignature");

  if (func.local_types.size()) {
    WriteTypeBindings("locals", "local", func.local_types, index_to_name,
                      func.GetNumParams());
  }

  BeginFunc(func);
  WriteOpenTag("instructions");
  if (options_.fold_exprs) {
    WriteFoldedExprList(func.exprs);
    FlushExprTreeStack();
  } else {
    WriteExprList(func.exprs);
  }
  WriteCloseTag("instructions");
  EndFunc();
  WriteCloseTag("function");
}

void AstWriter::WriteBeginGlobal(const Global& global) {
  WriteOpenSpace("global");
  WriteNameOrIndex(global.name, global_index_, NextChar::Space);
  WriteInlineExports(ExternalKind::Global, global_index_);
  WriteInlineImport(ExternalKind::Global, global_index_);
  if (global.mutable_) {
    WriteOpenSpace("mut");
    WriteType(global.type, NextChar::Space);
    WriteCloseSpace();
  } else {
    WriteType(global.type, NextChar::Space);
  }
  global_index_++;
}

void AstWriter::WriteGlobal(const Global& global) {
  WriteBeginGlobal(global);
  WriteInitExpr(global.init_expr);
  WriteCloseNewline();
}

void AstWriter::WriteEvent(const Event& event) {
  WriteOpenSpace("event");
  WriteNameOrIndex(event.name, event_index_, NextChar::Space);
  WriteInlineExports(ExternalKind::Event, event_index_);
  WriteInlineImport(ExternalKind::Event, event_index_);
  if (event.decl.has_func_type) {
    WriteOpenSpace("type");
    WriteVar(event.decl.type_var, NextChar::None);
    WriteCloseSpace();
  }
  WriteTypes(event.decl.sig.param_types, "param");
  ++event_index_;
  WriteCloseNewline();
}

void AstWriter::WriteLimits(const Limits& limits) {
  Writef("%" PRIu64, limits.initial);
  if (limits.has_max) {
    Writef("%" PRIu64, limits.max);
  }
  if (limits.is_shared) {
    Writef("shared");
  }
}

void AstWriter::WriteTable(const Table& table) {
  WriteOpenSpace("table");
  WriteNameOrIndex(table.name, table_index_, NextChar::Space);
  WriteInlineExports(ExternalKind::Table, table_index_);
  WriteInlineImport(ExternalKind::Table, table_index_);
  WriteLimits(table.elem_limits);
  WriteType(table.elem_type, NextChar::None);
  WriteCloseNewline();
  table_index_++;
}

void AstWriter::WriteElemSegment(const ElemSegment& segment) {
  WriteOpenSpace("elem");
  WriteNameOrIndex(segment.name, elem_segment_index_, NextChar::Space);

  uint8_t flags = segment.GetFlags(&module);

  if (!(flags & SegPassive)) {
    WriteInitExpr(segment.offset);
  }

  if (flags == SegDeclared) {
    WritePuts("declare", NextChar::Space);
  }

  if (flags & SegUseElemExprs) {
    WriteType(segment.elem_type, NextChar::Space);
  } else {
    assert(segment.elem_type == Type::Funcref);
    WritePuts("func", NextChar::Space);
  }

  for (const ElemExpr& expr : segment.elem_exprs) {
    if (flags & SegUseElemExprs) {
      if (expr.kind == ElemExprKind::RefNull) {
        WriteOpenSpace("ref.null");
        WriteCloseSpace();
      } else {
        WriteOpenSpace("ref.func");
        WriteVar(expr.var, NextChar::Space);
        WriteCloseSpace();
      }
    } else {
      assert(expr.kind == ElemExprKind::RefFunc);
      WriteVar(expr.var, NextChar::Space);
    }
  }
  WriteCloseNewline();
  elem_segment_index_++;
}

void AstWriter::WriteMemory(const Memory& memory) {
  WriteOpenSpace("memory");
  WriteNameOrIndex(memory.name, memory_index_, NextChar::Space);
  WriteInlineExports(ExternalKind::Memory, memory_index_);
  WriteInlineImport(ExternalKind::Memory, memory_index_);
  WriteLimits(memory.page_limits);
  WriteCloseNewline();
  memory_index_++;
}

void AstWriter::WriteDataSegment(const DataSegment& segment) {
  WriteOpenSpace("data");
  WriteNameOrIndex(segment.name, data_segment_index_, NextChar::Space);
  if (segment.kind != SegmentKind::Passive) {
    WriteInitExpr(segment.offset);
  }
  WriteQuotedData(segment.data.data(), segment.data.size());
  WriteCloseNewline();
  data_segment_index_++;
}

void AstWriter::WriteImport(const Import& import) {
  if (!options_.inline_import) {
    WriteOpenSpace("import");
    WriteQuotedString(import.module_name, NextChar::Space);
    WriteQuotedString(import.field_name, NextChar::Space);
  }

  switch (import.kind()) {
    case ExternalKind::Func:
      WriteBeginFunc(cast<FuncImport>(&import)->func);
      WriteCloseSpace();
      break;

    case ExternalKind::Table:
      WriteTable(cast<TableImport>(&import)->table);
      break;

    case ExternalKind::Memory:
      WriteMemory(cast<MemoryImport>(&import)->memory);
      break;

    case ExternalKind::Global:
      WriteBeginGlobal(cast<GlobalImport>(&import)->global);
      WriteCloseSpace();
      break;

    case ExternalKind::Event:
      WriteEvent(cast<EventImport>(&import)->event);
      break;
  }

  if (options_.inline_import) {
    WriteNewline(NO_FORCE_NEWLINE);
  } else {
    WriteCloseNewline();
  }
}

void AstWriter::WriteExport(const Export& export_) {
  if (options_.inline_export && IsInlineExport(export_)) {
    return;
  }
  WriteOpenSpace("export");
  WriteQuotedString(export_.name, NextChar::Space);
  WriteOpenSpace(GetKindName(export_.kind));
  WriteVar(export_.var, NextChar::Space);
  WriteCloseSpace();
  WriteCloseNewline();
}

void AstWriter::WriteFuncType(const FuncType& func_type) {
  WriteOpenSpace("type");
  WriteNameOrIndex(func_type.name, func_type_index_++, NextChar::Space);
  WriteOpenSpace("func");
  WriteFuncSigSpace(func_type.sig);
  WriteCloseSpace();
  WriteCloseNewline();
}

void AstWriter::WriteStartFunction(const Var& start) {
  WriteOpenSpace("start");
  WriteVar(start, NextChar::None);
  WriteCloseNewline();
}

Result AstWriter::WriteModule() {
  BuildInlineExportMap();
  BuildInlineImportMap();
  if (module.name.empty()) {
    WriteOpenTag("module");
  } else {
    WriteOpenTag("module", module.name);
  }
  for (const ModuleField& field : module.fields) {
    switch (field.type()) {
      case ModuleFieldType::Func:
        WriteFunc(cast<FuncModuleField>(&field)->func);
        break;
      // case ModuleFieldType::Global:
      //   WriteGlobal(cast<GlobalModuleField>(&field)->global);
      //   break;
      // case ModuleFieldType::Import:
      //   WriteImport(*cast<ImportModuleField>(&field)->import);
      //   break;
      // case ModuleFieldType::Event:
      //   WriteEvent(cast<EventModuleField>(&field)->event);
      //   break;
      // case ModuleFieldType::Export:
      //   WriteExport(cast<ExportModuleField>(&field)->export_);
      //   break;
      // case ModuleFieldType::Table:
      //   WriteTable(cast<TableModuleField>(&field)->table);
      //   break;
      // case ModuleFieldType::ElemSegment:
      //   WriteElemSegment(cast<ElemSegmentModuleField>(&field)->elem_segment);
      //   break;
      // case ModuleFieldType::Memory:
      //   WriteMemory(cast<MemoryModuleField>(&field)->memory);
      //   break;
      // case ModuleFieldType::DataSegment:
      //   WriteDataSegment(cast<DataSegmentModuleField>(&field)->data_segment);
      //   break;
      // case ModuleFieldType::FuncType:
      //   WriteFuncType(cast<FuncTypeModuleField>(&field)->func_type);
      //   break;
      // case ModuleFieldType::Start:
      //   WriteStartFunction(cast<StartModuleField>(&field)->start);
      //   break;
      default:
        break;
    }
  }
  WriteCloseTag("module");
  /* force the newline to be written */
  WriteNextChar();
  return result_;
}

void AstWriter::BuildInlineExportMap() {
  if (!options_.inline_export) {
    return;
  }

  for (Export* export_ : module.exports) {
    Index index = kInvalidIndex;

    // Exported imports can't be written with inline exports, unless the
    // imports are also inline. For example, the following is invalid:
    //
    //   (import "module" "field" (func (export "e")))
    //
    // But this is valid:
    //
    //   (func (export "e") (import "module" "field"))
    //
    if (!options_.inline_import && module.IsImport(*export_)) {
      continue;
    }

    switch (export_->kind) {
      case ExternalKind::Func:
        index = module.GetFuncIndex(export_->var);
        break;

      case ExternalKind::Table:
        index = module.GetTableIndex(export_->var);
        break;

      case ExternalKind::Memory:
        index = module.GetMemoryIndex(export_->var);
        break;

      case ExternalKind::Global:
        index = module.GetGlobalIndex(export_->var);
        break;

      case ExternalKind::Event:
        index = module.GetEventIndex(export_->var);
        break;
    }

    if (index != kInvalidIndex) {
      auto key = std::make_pair(export_->kind, index);
      inline_export_map_.insert(std::make_pair(key, export_));
    }
  }
}

void AstWriter::WriteInlineExports(ExternalKind kind, Index index) {
  if (!options_.inline_export) {
    return;
  }

  auto iter_pair = inline_export_map_.equal_range(std::make_pair(kind, index));
  for (auto iter = iter_pair.first; iter != iter_pair.second; ++iter) {
    const Export* export_ = iter->second;
    WriteOpenAttribute("exportName");
    WritePuts(export_->name.c_str(), NextChar::None);
    WriteCloseAttribute(NextChar::Space);
  }
}

bool AstWriter::IsInlineExport(const Export& export_) {
  Index index;
  switch (export_.kind) {
    case ExternalKind::Func:
      index = module.GetFuncIndex(export_.var);
      break;

    case ExternalKind::Table:
      index = module.GetTableIndex(export_.var);
      break;

    case ExternalKind::Memory:
      index = module.GetMemoryIndex(export_.var);
      break;

    case ExternalKind::Global:
      index = module.GetGlobalIndex(export_.var);
      break;

    case ExternalKind::Event:
      index = module.GetEventIndex(export_.var);
      break;
  }

  return inline_export_map_.find(std::make_pair(export_.kind, index)) !=
         inline_export_map_.end();
}

void AstWriter::BuildInlineImportMap() {
  if (!options_.inline_import) {
    return;
  }

  for (const Import* import : module.imports) {
    inline_import_map_[static_cast<size_t>(import->kind())].push_back(import);
  }
}

void AstWriter::WriteInlineImport(ExternalKind kind, Index index) {
  if (!options_.inline_import) {
    return;
  }

  size_t kind_index = static_cast<size_t>(kind);

  if (index >= inline_import_map_[kind_index].size()) {
    return;
  }

  const Import* import = inline_import_map_[kind_index][index];
  WriteOpenSpace("import");
  WriteQuotedString(import->module_name, NextChar::Space);
  WriteQuotedString(import->field_name, NextChar::Space);
  WriteCloseSpace();
}

}  // end anonymous namespace

Result WriteAst(wabt::Stream* stream,
                const wabt::Module* module,
                const WriteAstOptions& options) {
  AstWriter ast_writer(stream, options, *module);
  return ast_writer.WriteModule();
}

}  // namespace wasmati 
