/* C++ code produced by gperf version 3.1 */
/* Command-line: gperf -m 50 -L C++ -N InWordSet -E -t -c --output-file=src/prebuilt/lexer-keywords.cc src/lexer-keywords.txt  */
/* Computed positions: -k'1,3,5-9,11-14,16-19,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "src/lexer-keywords.txt"
struct TokenInfo {
  TokenInfo(const char* name) : name(name) {}
  TokenInfo(const char* name, TokenType token_type)
      : name(name), token_type(token_type) {}
  TokenInfo(const char* name, Type value_type)
      : name(name), token_type(TokenType::ValueType), value_type(value_type) {}
  TokenInfo(const char* name, TokenType token_type, Opcode opcode)
      : name(name), token_type(token_type), opcode(opcode) {}

  const char* name;
  TokenType token_type;
  union {
    Type value_type;
    Opcode opcode;
  };
};
/* maximum key range = 1747, duplicates = 0 */

class Perfect_Hash
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static struct TokenInfo *InWordSet (const char *str, size_t len);
};

inline unsigned int
Perfect_Hash::hash (const char *str, size_t len)
{
  static unsigned short asso_values[] =
    {
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778,   22,   81, 1778,   51,
        14,   85,   13,  435,  207,  421,  294,  423,   78, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778,   13,   16,   52,   16,   70,
        30,   19,   13,  269,  645,   16,   29,   13,   66,   14,
        34,   16,  414,  458,   19,   13,   13,   15,  112,   50,
       117,  539,   60, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778, 1778,
      1778, 1778, 1778, 1778, 1778, 1778, 1778
    };
  unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[static_cast<unsigned char>(str[18])];
      /*FALLTHROUGH*/
      case 18:
        hval += asso_values[static_cast<unsigned char>(str[17])];
      /*FALLTHROUGH*/
      case 17:
        hval += asso_values[static_cast<unsigned char>(str[16]+1)];
      /*FALLTHROUGH*/
      case 16:
        hval += asso_values[static_cast<unsigned char>(str[15])];
      /*FALLTHROUGH*/
      case 15:
      case 14:
        hval += asso_values[static_cast<unsigned char>(str[13])];
      /*FALLTHROUGH*/
      case 13:
        hval += asso_values[static_cast<unsigned char>(str[12])];
      /*FALLTHROUGH*/
      case 12:
        hval += asso_values[static_cast<unsigned char>(str[11])];
      /*FALLTHROUGH*/
      case 11:
        hval += asso_values[static_cast<unsigned char>(str[10])];
      /*FALLTHROUGH*/
      case 10:
      case 9:
        hval += asso_values[static_cast<unsigned char>(str[8])];
      /*FALLTHROUGH*/
      case 8:
        hval += asso_values[static_cast<unsigned char>(str[7])];
      /*FALLTHROUGH*/
      case 7:
        hval += asso_values[static_cast<unsigned char>(str[6])];
      /*FALLTHROUGH*/
      case 6:
        hval += asso_values[static_cast<unsigned char>(str[5])];
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[static_cast<unsigned char>(str[4]+1)];
      /*FALLTHROUGH*/
      case 4:
      case 3:
        hval += asso_values[static_cast<unsigned char>(str[2])];
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[static_cast<unsigned char>(str[0])];
        break;
    }
  return hval + asso_values[static_cast<unsigned char>(str[len - 1])];
}

struct TokenInfo *
Perfect_Hash::InWordSet (const char *str, size_t len)
{
  enum
    {
      TOTAL_KEYWORDS = 504,
      MIN_WORD_LENGTH = 2,
      MAX_WORD_LENGTH = 28,
      MIN_HASH_VALUE = 31,
      MAX_HASH_VALUE = 1777
    };

  static struct TokenInfo wordlist[] =
    {
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 427 "src/lexer-keywords.txt"
      {"if", TokenType::If, Opcode::If},
      {""}, {""}, {""}, {""}, {""},
#line 33 "src/lexer-keywords.txt"
      {"br", TokenType::Br, Opcode::Br},
      {""}, {""}, {""}, {""},
#line 127 "src/lexer-keywords.txt"
      {"f64", Type::F64},
#line 442 "src/lexer-keywords.txt"
      {"mut", TokenType::Mut},
#line 76 "src/lexer-keywords.txt"
      {"f32", Type::F32},
#line 381 "src/lexer-keywords.txt"
      {"i64", Type::I64},
      {""},
#line 261 "src/lexer-keywords.txt"
      {"i32", Type::I32},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 43 "src/lexer-keywords.txt"
      {"else", TokenType::Else, Opcode::Else},
#line 42 "src/lexer-keywords.txt"
      {"elem", TokenType::Elem},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 466 "src/lexer-keywords.txt"
      {"table", TokenType::Table},
      {""}, {""}, {""},
#line 467 "src/lexer-keywords.txt"
      {"then", TokenType::Then},
#line 45 "src/lexer-keywords.txt"
      {"event", TokenType::Event},
#line 114 "src/lexer-keywords.txt"
      {"f64.lt", TokenType::Compare, Opcode::F64Lt},
#line 64 "src/lexer-keywords.txt"
      {"f32.lt", TokenType::Compare, Opcode::F32Lt},
#line 444 "src/lexer-keywords.txt"
      {"offset", TokenType::Offset},
      {""}, {""}, {""},
#line 452 "src/lexer-keywords.txt"
      {"result", TokenType::Result},
      {""}, {""}, {""},
#line 44 "src/lexer-keywords.txt"
      {"end", TokenType::End, Opcode::End},
      {""},
#line 112 "src/lexer-keywords.txt"
      {"f64.le", TokenType::Compare, Opcode::F64Le},
#line 62 "src/lexer-keywords.txt"
      {"f32.le", TokenType::Compare, Opcode::F32Le},
#line 120 "src/lexer-keywords.txt"
      {"f64.ne", TokenType::Compare, Opcode::F64Ne},
#line 70 "src/lexer-keywords.txt"
      {"f32.ne", TokenType::Compare, Opcode::F32Ne},
      {""},
#line 357 "src/lexer-keywords.txt"
      {"i64.ne", TokenType::Compare, Opcode::I64Ne},
#line 238 "src/lexer-keywords.txt"
      {"i32.ne", TokenType::Compare, Opcode::I32Ne},
#line 98 "src/lexer-keywords.txt"
      {"f64.abs", TokenType::Unary, Opcode::F64Abs},
#line 47 "src/lexer-keywords.txt"
      {"f32.abs", TokenType::Unary, Opcode::F32Abs},
#line 125 "src/lexer-keywords.txt"
      {"f64.sub", TokenType::Binary, Opcode::F64Sub},
#line 74 "src/lexer-keywords.txt"
      {"f32.sub", TokenType::Binary, Opcode::F32Sub},
      {""},
#line 372 "src/lexer-keywords.txt"
      {"i64.sub", TokenType::Binary, Opcode::I64Sub},
#line 252 "src/lexer-keywords.txt"
      {"i32.sub", TokenType::Binary, Opcode::I32Sub},
#line 459 "src/lexer-keywords.txt"
      {"start", TokenType::Start},
#line 39 "src/lexer-keywords.txt"
      {"data", TokenType::Data},
      {""}, {""},
#line 441 "src/lexer-keywords.txt"
      {"module", TokenType::Module},
#line 354 "src/lexer-keywords.txt"
      {"i64.lt_s", TokenType::Compare, Opcode::I64LtS},
#line 235 "src/lexer-keywords.txt"
      {"i32.lt_s", TokenType::Compare, Opcode::I32LtS},
      {""}, {""},
#line 355 "src/lexer-keywords.txt"
      {"i64.lt_u", TokenType::Compare, Opcode::I64LtU},
#line 236 "src/lexer-keywords.txt"
      {"i32.lt_u", TokenType::Compare, Opcode::I32LtU},
#line 345 "src/lexer-keywords.txt"
      {"i64.le_s", TokenType::Compare, Opcode::I64LeS},
#line 228 "src/lexer-keywords.txt"
      {"i32.le_s", TokenType::Compare, Opcode::I32LeS},
#line 450 "src/lexer-keywords.txt"
      {"ref.host", TokenType::RefHost},
#line 149 "src/lexer-keywords.txt"
      {"funcref", Type::Funcref},
#line 346 "src/lexer-keywords.txt"
      {"i64.le_u", TokenType::Compare, Opcode::I64LeU},
#line 229 "src/lexer-keywords.txt"
      {"i32.le_u", TokenType::Compare, Opcode::I32LeU},
      {""},
#line 29 "src/lexer-keywords.txt"
      {"block", TokenType::Block, Opcode::Block},
#line 364 "src/lexer-keywords.txt"
      {"i64.rotr", TokenType::Binary, Opcode::I64Rotr},
#line 245 "src/lexer-keywords.txt"
      {"i32.rotr", TokenType::Binary, Opcode::I32Rotr},
#line 456 "src/lexer-keywords.txt"
      {"return", TokenType::Return, Opcode::Return},
      {""},
#line 150 "src/lexer-keywords.txt"
      {"func", TokenType::Func},
      {""},
#line 361 "src/lexer-keywords.txt"
      {"i64.rem_s", TokenType::Binary, Opcode::I64RemS},
#line 242 "src/lexer-keywords.txt"
      {"i32.rem_s", TokenType::Binary, Opcode::I32RemS},
      {""}, {""},
#line 362 "src/lexer-keywords.txt"
      {"i64.rem_u", TokenType::Binary, Opcode::I64RemU},
#line 243 "src/lexer-keywords.txt"
      {"i32.rem_u", TokenType::Binary, Opcode::I32RemU},
      {""},
#line 148 "src/lexer-keywords.txt"
      {"f64x2", TokenType::F64X2},
#line 464 "src/lexer-keywords.txt"
      {"table.set", TokenType::TableSet, Opcode::TableSet},
      {""},
#line 394 "src/lexer-keywords.txt"
      {"i64x2", TokenType::I64X2},
#line 124 "src/lexer-keywords.txt"
      {"f64.store", TokenType::Store, Opcode::F64Store},
#line 73 "src/lexer-keywords.txt"
      {"f32.store", TokenType::Store, Opcode::F32Store},
      {""},
#line 371 "src/lexer-keywords.txt"
      {"i64.store", TokenType::Store, Opcode::I64Store},
#line 251 "src/lexer-keywords.txt"
      {"i32.store", TokenType::Store, Opcode::I32Store},
#line 99 "src/lexer-keywords.txt"
      {"f64.add", TokenType::Binary, Opcode::F64Add},
#line 48 "src/lexer-keywords.txt"
      {"f32.add", TokenType::Binary, Opcode::F32Add},
#line 457 "src/lexer-keywords.txt"
      {"select", TokenType::Select, Opcode::Select},
#line 290 "src/lexer-keywords.txt"
      {"i64.add", TokenType::Binary, Opcode::I64Add},
#line 185 "src/lexer-keywords.txt"
      {"i32.add", TokenType::Binary, Opcode::I32Add},
#line 458 "src/lexer-keywords.txt"
      {"shared", TokenType::Shared},
      {""},
#line 291 "src/lexer-keywords.txt"
      {"i64.and", TokenType::Binary, Opcode::I64And},
#line 186 "src/lexer-keywords.txt"
      {"i32.and", TokenType::Binary, Opcode::I32And},
#line 369 "src/lexer-keywords.txt"
      {"i64.store32", TokenType::Store, Opcode::I64Store32},
      {""}, {""},
#line 116 "src/lexer-keywords.txt"
      {"f64.min", TokenType::Binary, Opcode::F64Min},
#line 66 "src/lexer-keywords.txt"
      {"f32.min", TokenType::Binary, Opcode::F32Min},
#line 463 "src/lexer-keywords.txt"
      {"table.init", TokenType::TableInit, Opcode::TableInit},
#line 101 "src/lexer-keywords.txt"
      {"f64.const", TokenType::Const, Opcode::F64Const},
#line 50 "src/lexer-keywords.txt"
      {"f32.const", TokenType::Const, Opcode::F32Const},
      {""},
#line 330 "src/lexer-keywords.txt"
      {"i64.const", TokenType::Const, Opcode::I64Const},
#line 216 "src/lexer-keywords.txt"
      {"i32.const", TokenType::Const, Opcode::I32Const},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 453 "src/lexer-keywords.txt"
      {"rethrow", TokenType::Rethrow, Opcode::Rethrow},
      {""}, {""}, {""}, {""},
#line 32 "src/lexer-keywords.txt"
      {"br_table", TokenType::BrTable, Opcode::BrTable},
      {""}, {""},
#line 113 "src/lexer-keywords.txt"
      {"f64.load", TokenType::Load, Opcode::F64Load},
#line 63 "src/lexer-keywords.txt"
      {"f32.load", TokenType::Load, Opcode::F32Load},
      {""},
#line 353 "src/lexer-keywords.txt"
      {"i64.load", TokenType::Load, Opcode::I64Load},
#line 234 "src/lexer-keywords.txt"
      {"i32.load", TokenType::Load, Opcode::I32Load},
      {""},
#line 465 "src/lexer-keywords.txt"
      {"table.size", TokenType::TableSize, Opcode::TableSize},
      {""}, {""}, {""}, {""}, {""},
#line 118 "src/lexer-keywords.txt"
      {"f64.nearest", TokenType::Unary, Opcode::F64Nearest},
#line 68 "src/lexer-keywords.txt"
      {"f32.nearest", TokenType::Unary, Opcode::F32Nearest},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 331 "src/lexer-keywords.txt"
      {"i64.ctz", TokenType::Unary, Opcode::I64Ctz},
#line 217 "src/lexer-keywords.txt"
      {"i32.ctz", TokenType::Unary, Opcode::I32Ctz},
      {""},
#line 147 "src/lexer-keywords.txt"
      {"f64x2.sub", TokenType::Binary, Opcode::F64X2Sub},
      {""},
#line 468 "src/lexer-keywords.txt"
      {"throw", TokenType::Throw, Opcode::Throw},
#line 393 "src/lexer-keywords.txt"
      {"i64x2.sub", TokenType::Binary, Opcode::I64X2Sub},
#line 35 "src/lexer-keywords.txt"
      {"call", TokenType::Call, Opcode::Call},
      {""}, {""}, {""}, {""},
#line 363 "src/lexer-keywords.txt"
      {"i64.rotl", TokenType::Binary, Opcode::I64Rotl},
#line 244 "src/lexer-keywords.txt"
      {"i32.rotl", TokenType::Binary, Opcode::I32Rotl},
#line 143 "src/lexer-keywords.txt"
      {"f64x2.ne", TokenType::Compare, Opcode::F64X2Ne},
#line 117 "src/lexer-keywords.txt"
      {"f64.mul", TokenType::Binary, Opcode::F64Mul},
#line 67 "src/lexer-keywords.txt"
      {"f32.mul", TokenType::Binary, Opcode::F32Mul},
      {""},
#line 356 "src/lexer-keywords.txt"
      {"i64.mul", TokenType::Binary, Opcode::I64Mul},
#line 237 "src/lexer-keywords.txt"
      {"i32.mul", TokenType::Binary, Opcode::I32Mul},
      {""}, {""},
#line 433 "src/lexer-keywords.txt"
      {"local", TokenType::Local},
#line 327 "src/lexer-keywords.txt"
      {"i64.atomic.store", TokenType::AtomicStore, Opcode::I64AtomicStore},
#line 213 "src/lexer-keywords.txt"
      {"i32.atomic.store", TokenType::AtomicStore, Opcode::I32AtomicStore},
#line 332 "src/lexer-keywords.txt"
      {"i64.div_s", TokenType::Binary, Opcode::I64DivS},
#line 218 "src/lexer-keywords.txt"
      {"i32.div_s", TokenType::Binary, Opcode::I32DivS},
      {""}, {""},
#line 333 "src/lexer-keywords.txt"
      {"i64.div_u", TokenType::Binary, Opcode::I64DivU},
#line 219 "src/lexer-keywords.txt"
      {"i32.div_u", TokenType::Binary, Opcode::I32DivU},
      {""},
#line 100 "src/lexer-keywords.txt"
      {"f64.ceil", TokenType::Unary, Opcode::F64Ceil},
#line 49 "src/lexer-keywords.txt"
      {"f32.ceil", TokenType::Unary, Opcode::F32Ceil},
#line 138 "src/lexer-keywords.txt"
      {"f64x2.lt", TokenType::Compare, Opcode::F64X2Lt},
      {""}, {""},
#line 128 "src/lexer-keywords.txt"
      {"f64x2.abs", TokenType::Unary, Opcode::F64X2Abs},
      {""},
#line 429 "src/lexer-keywords.txt"
      {"invoke", TokenType::Invoke},
#line 431 "src/lexer-keywords.txt"
      {"local.set", TokenType::LocalSet, Opcode::LocalSet},
#line 140 "src/lexer-keywords.txt"
      {"f64x2.min", TokenType::Binary, Opcode::F64X2Min},
      {""}, {""},
#line 347 "src/lexer-keywords.txt"
      {"i64.load16_s", TokenType::Load, Opcode::I64Load16S},
#line 230 "src/lexer-keywords.txt"
      {"i32.load16_s", TokenType::Load, Opcode::I32Load16S},
#line 137 "src/lexer-keywords.txt"
      {"f64x2.le", TokenType::Compare, Opcode::F64X2Le},
#line 325 "src/lexer-keywords.txt"
      {"i64.atomic.store32", TokenType::AtomicStore, Opcode::I64AtomicStore32},
#line 348 "src/lexer-keywords.txt"
      {"i64.load16_u", TokenType::Load, Opcode::I64Load16U},
#line 231 "src/lexer-keywords.txt"
      {"i32.load16_u", TokenType::Load, Opcode::I32Load16U},
      {""}, {""},
#line 432 "src/lexer-keywords.txt"
      {"local.tee", TokenType::LocalTee, Opcode::LocalTee},
#line 329 "src/lexer-keywords.txt"
      {"i64.clz", TokenType::Unary, Opcode::I64Clz},
#line 215 "src/lexer-keywords.txt"
      {"i32.clz", TokenType::Unary, Opcode::I32Clz},
      {""}, {""}, {""},
#line 24 "src/lexer-keywords.txt"
      {"assert_return", TokenType::AssertReturn},
#line 126 "src/lexer-keywords.txt"
      {"f64.trunc", TokenType::Unary, Opcode::F64Trunc},
#line 75 "src/lexer-keywords.txt"
      {"f32.trunc", TokenType::Unary, Opcode::F32Trunc},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 477 "src/lexer-keywords.txt"
      {"v128.or", TokenType::Binary, Opcode::V128Or},
#line 449 "src/lexer-keywords.txt"
      {"ref.null", TokenType::RefNull, Opcode::RefNull},
      {""},
#line 31 "src/lexer-keywords.txt"
      {"br_on_exn", TokenType::BrOnExn, Opcode::BrOnExn},
#line 328 "src/lexer-keywords.txt"
      {"i64.atomic.wait", TokenType::AtomicWait, Opcode::I64AtomicWait},
#line 214 "src/lexer-keywords.txt"
      {"i32.atomic.wait", TokenType::AtomicWait, Opcode::I32AtomicWait},
#line 320 "src/lexer-keywords.txt"
      {"i64.atomic.rmw.or", TokenType::AtomicRmw, Opcode::I64AtomicRmwOr},
#line 207 "src/lexer-keywords.txt"
      {"i32.atomic.rmw.or", TokenType::AtomicRmw, Opcode::I32AtomicRmwOr},
      {""},
#line 349 "src/lexer-keywords.txt"
      {"i64.load32_s", TokenType::Load, Opcode::I64Load32S},
      {""},
#line 448 "src/lexer-keywords.txt"
      {"ref.is_null", TokenType::RefIsNull, Opcode::RefIsNull},
      {""},
#line 350 "src/lexer-keywords.txt"
      {"i64.load32_u", TokenType::Load, Opcode::I64Load32U},
      {""}, {""},
#line 129 "src/lexer-keywords.txt"
      {"f64x2.add", TokenType::Binary, Opcode::F64X2Add},
      {""}, {""},
#line 382 "src/lexer-keywords.txt"
      {"i64x2.add", TokenType::Binary, Opcode::I64X2Add},
      {""}, {""}, {""},
#line 476 "src/lexer-keywords.txt"
      {"v128.not", TokenType::Unary, Opcode::V128Not},
#line 107 "src/lexer-keywords.txt"
      {"f64.div", TokenType::Binary, Opcode::F64Div},
#line 57 "src/lexer-keywords.txt"
      {"f32.div", TokenType::Binary, Opcode::F32Div},
      {""}, {""}, {""},
#line 478 "src/lexer-keywords.txt"
      {"v128.store", TokenType::Store, Opcode::V128Store},
#line 151 "src/lexer-keywords.txt"
      {"get", TokenType::Get},
      {""}, {""}, {""}, {""},
#line 141 "src/lexer-keywords.txt"
      {"f64x2.mul", TokenType::Binary, Opcode::F64X2Mul},
      {""},
#line 295 "src/lexer-keywords.txt"
      {"i64.atomic.load", TokenType::AtomicLoad, Opcode::I64AtomicLoad},
#line 189 "src/lexer-keywords.txt"
      {"i32.atomic.load", TokenType::AtomicLoad, Opcode::I32AtomicLoad},
      {""},
#line 21 "src/lexer-keywords.txt"
      {"assert_malformed", TokenType::AssertMalformed},
      {""}, {""},
#line 314 "src/lexer-keywords.txt"
      {"i64.atomic.rmw8.sub_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw8SubU},
#line 201 "src/lexer-keywords.txt"
      {"i32.atomic.rmw8.sub_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw8SubU},
      {""}, {""}, {""},
#line 30 "src/lexer-keywords.txt"
      {"br_if", TokenType::BrIf, Opcode::BrIf},
      {""}, {""},
#line 521 "src/lexer-keywords.txt"
      {"set_local", TokenType::LocalSet, Opcode::LocalSet},
      {""}, {""}, {""}, {""}, {""},
#line 522 "src/lexer-keywords.txt"
      {"tee_local", TokenType::LocalTee, Opcode::LocalTee},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 373 "src/lexer-keywords.txt"
      {"i64.trunc_f32_s", TokenType::Convert, Opcode::I64TruncF32S},
#line 253 "src/lexer-keywords.txt"
      {"i32.trunc_f32_s", TokenType::Convert, Opcode::I32TruncF32S},
#line 374 "src/lexer-keywords.txt"
      {"i64.trunc_f32_u", TokenType::Convert, Opcode::I64TruncF32U},
#line 254 "src/lexer-keywords.txt"
      {"i32.trunc_f32_u", TokenType::Convert, Opcode::I32TruncF32U},
      {""}, {""}, {""}, {""},
#line 310 "src/lexer-keywords.txt"
      {"i64.atomic.rmw8.add_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw8AddU},
#line 197 "src/lexer-keywords.txt"
      {"i32.atomic.rmw8.add_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw8AddU},
      {""}, {""},
#line 311 "src/lexer-keywords.txt"
      {"i64.atomic.rmw8.and_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw8AndU},
#line 198 "src/lexer-keywords.txt"
      {"i32.atomic.rmw8.and_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw8AndU},
      {""}, {""}, {""}, {""},
#line 115 "src/lexer-keywords.txt"
      {"f64.max", TokenType::Binary, Opcode::F64Max},
#line 65 "src/lexer-keywords.txt"
      {"f32.max", TokenType::Binary, Opcode::F32Max},
#line 318 "src/lexer-keywords.txt"
      {"i64.atomic.rmw.and", TokenType::AtomicRmw, Opcode::I64AtomicRmwAnd},
#line 205 "src/lexer-keywords.txt"
      {"i32.atomic.rmw.and", TokenType::AtomicRmw, Opcode::I32AtomicRmwAnd},
#line 455 "src/lexer-keywords.txt"
      {"return_call", TokenType::ReturnCall, Opcode::ReturnCall},
#line 317 "src/lexer-keywords.txt"
      {"i64.atomic.rmw.add", TokenType::AtomicRmw, Opcode::I64AtomicRmwAdd},
#line 204 "src/lexer-keywords.txt"
      {"i32.atomic.rmw.add", TokenType::AtomicRmw, Opcode::I32AtomicRmwAdd},
      {""},
#line 472 "src/lexer-keywords.txt"
      {"v128.and", TokenType::Binary, Opcode::V128And},
#line 20 "src/lexer-keywords.txt"
      {"assert_invalid", TokenType::AssertInvalid},
#line 474 "src/lexer-keywords.txt"
      {"v128.const", TokenType::Const, Opcode::V128Const},
      {""}, {""}, {""}, {""}, {""},
#line 307 "src/lexer-keywords.txt"
      {"i64.atomic.rmw32.sub_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw32SubU},
      {""}, {""}, {""}, {""}, {""},
#line 306 "src/lexer-keywords.txt"
      {"i64.atomic.rmw32.or_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw32OrU},
#line 34 "src/lexer-keywords.txt"
      {"call_indirect", TokenType::CallIndirect, Opcode::CallIndirect},
#line 337 "src/lexer-keywords.txt"
      {"i64.extend32_s", TokenType::Unary, Opcode::I64Extend32S},
      {""},
#line 451 "src/lexer-keywords.txt"
      {"register", TokenType::Register},
      {""},
#line 56 "src/lexer-keywords.txt"
      {"f32.demote_f64", TokenType::Convert, Opcode::F32DemoteF64},
      {""},
#line 26 "src/lexer-keywords.txt"
      {"assert_unlinkable", TokenType::AssertUnlinkable},
#line 321 "src/lexer-keywords.txt"
      {"i64.atomic.rmw.sub", TokenType::AtomicRmw, Opcode::I64AtomicRmwSub},
#line 208 "src/lexer-keywords.txt"
      {"i32.atomic.rmw.sub", TokenType::AtomicRmw, Opcode::I32AtomicRmwSub},
#line 480 "src/lexer-keywords.txt"
      {"v128.xor", TokenType::Binary, Opcode::V128Xor},
#line 461 "src/lexer-keywords.txt"
      {"table.get", TokenType::TableGet, Opcode::TableGet},
      {""}, {""}, {""}, {""},
#line 509 "src/lexer-keywords.txt"
      {"i64.extend_s/i32", TokenType::Convert, Opcode::I64ExtendI32S},
      {""},
#line 510 "src/lexer-keywords.txt"
      {"i64.extend_u/i32", TokenType::Convert, Opcode::I64ExtendI32U},
#line 339 "src/lexer-keywords.txt"
      {"i64.extend_i32_s", TokenType::Convert, Opcode::I64ExtendI32S},
      {""}, {""}, {""},
#line 340 "src/lexer-keywords.txt"
      {"i64.extend_i32_u", TokenType::Convert, Opcode::I64ExtendI32U},
      {""}, {""},
#line 23 "src/lexer-keywords.txt"
      {"assert_return_canonical_nan", TokenType::AssertReturnCanonicalNan},
#line 512 "src/lexer-keywords.txt"
      {"i64.trunc_s/f32", TokenType::Convert, Opcode::I64TruncF32S},
#line 500 "src/lexer-keywords.txt"
      {"i32.trunc_s/f32", TokenType::Convert, Opcode::I32TruncF32S},
#line 516 "src/lexer-keywords.txt"
      {"i64.trunc_u/f32", TokenType::Convert, Opcode::I64TruncF32U},
#line 504 "src/lexer-keywords.txt"
      {"i32.trunc_u/f32", TokenType::Convert, Opcode::I32TruncF32U},
      {""}, {""}, {""},
#line 475 "src/lexer-keywords.txt"
      {"v128.load", TokenType::Load, Opcode::V128Load},
      {""},
#line 132 "src/lexer-keywords.txt"
      {"f64x2.div", TokenType::Binary, Opcode::F64X2Div},
#line 370 "src/lexer-keywords.txt"
      {"i64.store8", TokenType::Store, Opcode::I64Store8},
#line 250 "src/lexer-keywords.txt"
      {"i32.store8", TokenType::Store, Opcode::I32Store8},
      {""},
#line 19 "src/lexer-keywords.txt"
      {"assert_exhaustion", TokenType::AssertExhaustion},
      {""}, {""},
#line 383 "src/lexer-keywords.txt"
      {"i64x2.all_true", TokenType::Unary, Opcode::I64X2AllTrue},
#line 102 "src/lexer-keywords.txt"
      {"f64.convert_i32_s", TokenType::Convert, Opcode::F64ConvertI32S},
#line 51 "src/lexer-keywords.txt"
      {"f32.convert_i32_s", TokenType::Convert, Opcode::F32ConvertI32S},
      {""},
#line 303 "src/lexer-keywords.txt"
      {"i64.atomic.rmw32.add_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw32AddU},
#line 479 "src/lexer-keywords.txt"
      {"v128", Type::V128},
      {""}, {""},
#line 304 "src/lexer-keywords.txt"
      {"i64.atomic.rmw32.and_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw32AndU},
#line 462 "src/lexer-keywords.txt"
      {"table.grow", TokenType::TableGrow, Opcode::TableGrow},
#line 305 "src/lexer-keywords.txt"
      {"i64.atomic.rmw32.cmpxchg_u", TokenType::AtomicRmwCmpxchg, Opcode::I64AtomicRmw32CmpxchgU},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 136 "src/lexer-keywords.txt"
      {"f64x2.gt", TokenType::Compare, Opcode::F64X2Gt},
      {""}, {""},
#line 154 "src/lexer-keywords.txt"
      {"global", TokenType::Global},
#line 109 "src/lexer-keywords.txt"
      {"f64.floor", TokenType::Unary, Opcode::F64Floor},
#line 59 "src/lexer-keywords.txt"
      {"f32.floor", TokenType::Unary, Opcode::F32Floor},
#line 139 "src/lexer-keywords.txt"
      {"f64x2.max", TokenType::Binary, Opcode::F64X2Max},
      {""},
#line 153 "src/lexer-keywords.txt"
      {"global.set", TokenType::GlobalSet, Opcode::GlobalSet},
      {""}, {""}, {""},
#line 135 "src/lexer-keywords.txt"
      {"f64x2.ge", TokenType::Compare, Opcode::F64X2Ge},
#line 489 "src/lexer-keywords.txt"
      {"f32.demote/f64", TokenType::Convert, Opcode::F32DemoteF64},
#line 470 "src/lexer-keywords.txt"
      {"type", TokenType::Type},
      {""}, {""}, {""}, {""}, {""},
#line 375 "src/lexer-keywords.txt"
      {"i64.trunc_f64_s", TokenType::Convert, Opcode::I64TruncF64S},
#line 255 "src/lexer-keywords.txt"
      {"i32.trunc_f64_s", TokenType::Convert, Opcode::I32TruncF64S},
#line 376 "src/lexer-keywords.txt"
      {"i64.trunc_f64_u", TokenType::Convert, Opcode::I64TruncF64U},
#line 256 "src/lexer-keywords.txt"
      {"i32.trunc_f64_u", TokenType::Convert, Opcode::I32TruncF64U},
#line 473 "src/lexer-keywords.txt"
      {"v128.bitselect", TokenType::Ternary, Opcode::V128BitSelect},
      {""}, {""}, {""},
#line 40 "src/lexer-keywords.txt"
      {"drop", TokenType::Drop, Opcode::Drop},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 351 "src/lexer-keywords.txt"
      {"i64.load8_s", TokenType::Load, Opcode::I64Load8S},
#line 232 "src/lexer-keywords.txt"
      {"i32.load8_s", TokenType::Load, Opcode::I32Load8S},
#line 309 "src/lexer-keywords.txt"
      {"i64.atomic.rmw32.xor_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw32XorU},
#line 428 "src/lexer-keywords.txt"
      {"import", TokenType::Import},
#line 352 "src/lexer-keywords.txt"
      {"i64.load8_u", TokenType::Load, Opcode::I64Load8U},
#line 233 "src/lexer-keywords.txt"
      {"i32.load8_u", TokenType::Load, Opcode::I32Load8U},
#line 46 "src/lexer-keywords.txt"
      {"export", TokenType::Export},
      {""},
#line 97 "src/lexer-keywords.txt"
      {"f32x4", TokenType::F32X4},
      {""}, {""},
#line 286 "src/lexer-keywords.txt"
      {"i32x4", TokenType::I32X4},
      {""}, {""},
#line 445 "src/lexer-keywords.txt"
      {"param", TokenType::Param},
#line 358 "src/lexer-keywords.txt"
      {"i64.or", TokenType::Binary, Opcode::I64Or},
#line 239 "src/lexer-keywords.txt"
      {"i32.or", TokenType::Binary, Opcode::I32Or},
      {""}, {""}, {""},
#line 293 "src/lexer-keywords.txt"
      {"i64.atomic.load32_u", TokenType::AtomicLoad, Opcode::I64AtomicLoad32U},
      {""},
#line 454 "src/lexer-keywords.txt"
      {"return_call_indirect", TokenType::ReturnCallIndirect, Opcode::ReturnCallIndirect},
#line 430 "src/lexer-keywords.txt"
      {"local.get", TokenType::LocalGet, Opcode::LocalGet},
      {""}, {""},
#line 447 "src/lexer-keywords.txt"
      {"ref.func", TokenType::RefFunc, Opcode::RefFunc},
      {""},
#line 434 "src/lexer-keywords.txt"
      {"loop", TokenType::Loop, Opcode::Loop},
#line 377 "src/lexer-keywords.txt"
      {"i64.trunc_sat_f32_s", TokenType::Convert, Opcode::I64TruncSatF32S},
#line 257 "src/lexer-keywords.txt"
      {"i32.trunc_sat_f32_s", TokenType::Convert, Opcode::I32TruncSatF32S},
      {""}, {""},
#line 378 "src/lexer-keywords.txt"
      {"i64.trunc_sat_f32_u", TokenType::Convert, Opcode::I64TruncSatF32U},
#line 258 "src/lexer-keywords.txt"
      {"i32.trunc_sat_f32_u", TokenType::Convert, Opcode::I32TruncSatF32U},
      {""}, {""}, {""}, {""},
#line 446 "src/lexer-keywords.txt"
      {"quote", TokenType::Quote},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 103 "src/lexer-keywords.txt"
      {"f64.convert_i32_u", TokenType::Convert, Opcode::F64ConvertI32U},
#line 52 "src/lexer-keywords.txt"
      {"f32.convert_i32_u", TokenType::Convert, Opcode::F32ConvertI32U},
      {""},
#line 513 "src/lexer-keywords.txt"
      {"i64.trunc_s/f64", TokenType::Convert, Opcode::I64TruncF64S},
#line 501 "src/lexer-keywords.txt"
      {"i32.trunc_s/f64", TokenType::Convert, Opcode::I32TruncF64S},
#line 517 "src/lexer-keywords.txt"
      {"i64.trunc_u/f64", TokenType::Convert, Opcode::I64TruncF64U},
#line 505 "src/lexer-keywords.txt"
      {"i32.trunc_u/f64", TokenType::Convert, Opcode::I32TruncF64U},
      {""},
#line 308 "src/lexer-keywords.txt"
      {"i64.atomic.rmw32.xchg_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw32XchgU},
      {""}, {""}, {""}, {""},
#line 368 "src/lexer-keywords.txt"
      {"i64.store16", TokenType::Store, Opcode::I64Store16},
#line 249 "src/lexer-keywords.txt"
      {"i32.store16", TokenType::Store, Opcode::I32Store16},
#line 336 "src/lexer-keywords.txt"
      {"i64.extend16_s", TokenType::Unary, Opcode::I64Extend16S},
#line 222 "src/lexer-keywords.txt"
      {"i32.extend16_s", TokenType::Unary, Opcode::I32Extend16S},
      {""}, {""}, {""}, {""},
#line 104 "src/lexer-keywords.txt"
      {"f64.convert_i64_s", TokenType::Convert, Opcode::F64ConvertI64S},
#line 53 "src/lexer-keywords.txt"
      {"f32.convert_i64_s", TokenType::Convert, Opcode::F32ConvertI64S},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 123 "src/lexer-keywords.txt"
      {"f64.sqrt", TokenType::Unary, Opcode::F64Sqrt},
#line 72 "src/lexer-keywords.txt"
      {"f32.sqrt", TokenType::Unary, Opcode::F32Sqrt},
      {""},
#line 96 "src/lexer-keywords.txt"
      {"f32x4.sub", TokenType::Binary, Opcode::F32X4Sub},
#line 134 "src/lexer-keywords.txt"
      {"f64x2.extract_lane", TokenType::SimdLaneOp, Opcode::F64X2ExtractLane},
      {""},
#line 285 "src/lexer-keywords.txt"
      {"i32x4.sub", TokenType::Binary, Opcode::I32X4Sub},
#line 385 "src/lexer-keywords.txt"
      {"i64x2.extract_lane", TokenType::SimdLaneOp, Opcode::I64X2ExtractLane},
#line 491 "src/lexer-keywords.txt"
      {"f64.convert_s/i32", TokenType::Convert, Opcode::F64ConvertI32S},
#line 485 "src/lexer-keywords.txt"
      {"f32.convert_s/i32", TokenType::Convert, Opcode::F32ConvertI32S},
#line 493 "src/lexer-keywords.txt"
      {"f64.convert_u/i32", TokenType::Convert, Opcode::F64ConvertI32U},
#line 487 "src/lexer-keywords.txt"
      {"f32.convert_u/i32", TokenType::Convert, Opcode::F32ConvertI32U},
#line 300 "src/lexer-keywords.txt"
      {"i64.atomic.rmw16.sub_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw16SubU},
#line 194 "src/lexer-keywords.txt"
      {"i32.atomic.rmw16.sub_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw16SubU},
#line 92 "src/lexer-keywords.txt"
      {"f32x4.ne", TokenType::Compare, Opcode::F32X4Ne},
      {""}, {""},
#line 279 "src/lexer-keywords.txt"
      {"i32x4.ne", TokenType::Compare, Opcode::I32X4Ne},
#line 299 "src/lexer-keywords.txt"
      {"i64.atomic.rmw16.or_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw16OrU},
#line 193 "src/lexer-keywords.txt"
      {"i32.atomic.rmw16.or_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw16OrU},
      {""},
#line 338 "src/lexer-keywords.txt"
      {"i64.extend8_s", TokenType::Unary, Opcode::I64Extend8S},
#line 223 "src/lexer-keywords.txt"
      {"i32.extend8_s", TokenType::Unary, Opcode::I32Extend8S},
      {""}, {""},
#line 498 "src/lexer-keywords.txt"
      {"get_local", TokenType::LocalGet, Opcode::LocalGet},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 87 "src/lexer-keywords.txt"
      {"f32x4.lt", TokenType::Compare, Opcode::F32X4Lt},
      {""}, {""},
#line 77 "src/lexer-keywords.txt"
      {"f32x4.abs", TokenType::Unary, Opcode::F32X4Abs},
      {""}, {""}, {""},
#line 89 "src/lexer-keywords.txt"
      {"f32x4.min", TokenType::Binary, Opcode::F32X4Min},
      {""}, {""}, {""}, {""},
#line 86 "src/lexer-keywords.txt"
      {"f32x4.le", TokenType::Compare, Opcode::F32X4Le},
      {""}, {""}, {""}, {""}, {""},
#line 275 "src/lexer-keywords.txt"
      {"i32x4.lt_s", TokenType::Compare, Opcode::I32X4LtS},
      {""},
#line 276 "src/lexer-keywords.txt"
      {"i32x4.lt_u", TokenType::Compare, Opcode::I32X4LtU},
      {""},
#line 119 "src/lexer-keywords.txt"
      {"f64.neg", TokenType::Unary, Opcode::F64Neg},
#line 69 "src/lexer-keywords.txt"
      {"f32.neg", TokenType::Unary, Opcode::F32Neg},
#line 272 "src/lexer-keywords.txt"
      {"i32x4.le_s", TokenType::Compare, Opcode::I32X4LeS},
      {""},
#line 273 "src/lexer-keywords.txt"
      {"i32x4.le_u", TokenType::Compare, Opcode::I32X4LeU},
      {""}, {""}, {""}, {""}, {""},
#line 296 "src/lexer-keywords.txt"
      {"i64.atomic.rmw16.add_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw16AddU},
#line 190 "src/lexer-keywords.txt"
      {"i32.atomic.rmw16.add_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw16AddU},
#line 294 "src/lexer-keywords.txt"
      {"i64.atomic.load8_u", TokenType::AtomicLoad, Opcode::I64AtomicLoad8U},
#line 188 "src/lexer-keywords.txt"
      {"i32.atomic.load8_u", TokenType::AtomicLoad, Opcode::I32AtomicLoad8U},
#line 297 "src/lexer-keywords.txt"
      {"i64.atomic.rmw16.and_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw16AndU},
#line 191 "src/lexer-keywords.txt"
      {"i32.atomic.rmw16.and_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw16AndU},
#line 298 "src/lexer-keywords.txt"
      {"i64.atomic.rmw16.cmpxchg_u", TokenType::AtomicRmwCmpxchg, Opcode::I64AtomicRmw16CmpxchgU},
#line 192 "src/lexer-keywords.txt"
      {"i32.atomic.rmw16.cmpxchg_u", TokenType::AtomicRmwCmpxchg, Opcode::I32AtomicRmw16CmpxchgU},
      {""}, {""}, {""},
#line 335 "src/lexer-keywords.txt"
      {"i64.eqz", TokenType::Convert, Opcode::I64Eqz},
#line 221 "src/lexer-keywords.txt"
      {"i32.eqz", TokenType::Convert, Opcode::I32Eqz},
#line 397 "src/lexer-keywords.txt"
      {"i64.xor", TokenType::Binary, Opcode::I64Xor},
#line 289 "src/lexer-keywords.txt"
      {"i32.xor", TokenType::Binary, Opcode::I32Xor},
      {""}, {""},
#line 324 "src/lexer-keywords.txt"
      {"i64.atomic.store16", TokenType::AtomicStore, Opcode::I64AtomicStore16},
#line 211 "src/lexer-keywords.txt"
      {"i32.atomic.store16", TokenType::AtomicStore, Opcode::I32AtomicStore16},
#line 78 "src/lexer-keywords.txt"
      {"f32x4.add", TokenType::Binary, Opcode::F32X4Add},
#line 18 "src/lexer-keywords.txt"
      {"anyref", Type::Anyref},
      {""},
#line 263 "src/lexer-keywords.txt"
      {"i32x4.add", TokenType::Binary, Opcode::I32X4Add},
      {""}, {""}, {""}, {""},
#line 105 "src/lexer-keywords.txt"
      {"f64.convert_i64_u", TokenType::Convert, Opcode::F64ConvertI64U},
#line 54 "src/lexer-keywords.txt"
      {"f32.convert_i64_u", TokenType::Convert, Opcode::F32ConvertI64U},
#line 425 "src/lexer-keywords.txt"
      {"i8x16.sub", TokenType::Binary, Opcode::I8X16Sub},
#line 146 "src/lexer-keywords.txt"
      {"f64x2.sqrt", TokenType::Unary, Opcode::F64X2Sqrt},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 90 "src/lexer-keywords.txt"
      {"f32x4.mul", TokenType::Binary, Opcode::F32X4Mul},
      {""},
#line 417 "src/lexer-keywords.txt"
      {"i8x16.ne", TokenType::Compare, Opcode::I8X16Ne},
#line 277 "src/lexer-keywords.txt"
      {"i32x4.mul", TokenType::Binary, Opcode::I32X4Mul},
      {""}, {""}, {""}, {""}, {""},
#line 145 "src/lexer-keywords.txt"
      {"f64x2.splat", TokenType::Unary, Opcode::F64X2Splat},
      {""}, {""},
#line 392 "src/lexer-keywords.txt"
      {"i64x2.splat", TokenType::Unary, Opcode::I64X2Splat},
#line 302 "src/lexer-keywords.txt"
      {"i64.atomic.rmw16.xor_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw16XorU},
#line 196 "src/lexer-keywords.txt"
      {"i32.atomic.rmw16.xor_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw16XorU},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""},
#line 439 "src/lexer-keywords.txt"
      {"memory.size", TokenType::MemorySize, Opcode::MemorySize},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 438 "src/lexer-keywords.txt"
      {"memory.init", TokenType::MemoryInit, Opcode::MemoryInit},
      {""},
#line 413 "src/lexer-keywords.txt"
      {"i8x16.lt_s", TokenType::Compare, Opcode::I8X16LtS},
      {""},
#line 414 "src/lexer-keywords.txt"
      {"i8x16.lt_u", TokenType::Compare, Opcode::I8X16LtU},
      {""}, {""}, {""},
#line 410 "src/lexer-keywords.txt"
      {"i8x16.le_s", TokenType::Compare, Opcode::I8X16LeS},
      {""},
#line 411 "src/lexer-keywords.txt"
      {"i8x16.le_u", TokenType::Compare, Opcode::I8X16LeU},
#line 152 "src/lexer-keywords.txt"
      {"global.get", TokenType::GlobalGet, Opcode::GlobalGet},
      {""}, {""},
#line 111 "src/lexer-keywords.txt"
      {"f64.gt", TokenType::Compare, Opcode::F64Gt},
#line 61 "src/lexer-keywords.txt"
      {"f32.gt", TokenType::Compare, Opcode::F32Gt},
      {""}, {""}, {""},
#line 121 "src/lexer-keywords.txt"
      {"f64.promote_f32", TokenType::Convert, Opcode::F64PromoteF32},
      {""}, {""}, {""},
#line 313 "src/lexer-keywords.txt"
      {"i64.atomic.rmw8.or_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw8OrU},
#line 200 "src/lexer-keywords.txt"
      {"i32.atomic.rmw8.or_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw8OrU},
      {""},
#line 110 "src/lexer-keywords.txt"
      {"f64.ge", TokenType::Compare, Opcode::F64Ge},
#line 60 "src/lexer-keywords.txt"
      {"f32.ge", TokenType::Compare, Opcode::F32Ge},
      {""}, {""}, {""}, {""}, {""},
#line 301 "src/lexer-keywords.txt"
      {"i64.atomic.rmw16.xchg_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw16XchgU},
#line 195 "src/lexer-keywords.txt"
      {"i32.atomic.rmw16.xchg_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw16XchgU},
      {""}, {""}, {""},
#line 400 "src/lexer-keywords.txt"
      {"i8x16.add", TokenType::Binary, Opcode::I8X16Add},
      {""},
#line 312 "src/lexer-keywords.txt"
      {"i64.atomic.rmw8.cmpxchg_u", TokenType::AtomicRmwCmpxchg, Opcode::I64AtomicRmw8CmpxchgU},
#line 199 "src/lexer-keywords.txt"
      {"i32.atomic.rmw8.cmpxchg_u", TokenType::AtomicRmwCmpxchg, Opcode::I32AtomicRmw8CmpxchgU},
      {""}, {""},
#line 142 "src/lexer-keywords.txt"
      {"f64x2.neg", TokenType::Unary, Opcode::F64X2Neg},
#line 343 "src/lexer-keywords.txt"
      {"i64.gt_s", TokenType::Compare, Opcode::I64GtS},
#line 226 "src/lexer-keywords.txt"
      {"i32.gt_s", TokenType::Compare, Opcode::I32GtS},
#line 387 "src/lexer-keywords.txt"
      {"i64x2.neg", TokenType::Unary, Opcode::I64X2Neg},
#line 183 "src/lexer-keywords.txt"
      {"i16x8.sub", TokenType::Binary, Opcode::I16X8Sub},
#line 344 "src/lexer-keywords.txt"
      {"i64.gt_u", TokenType::Compare, Opcode::I64GtU},
#line 227 "src/lexer-keywords.txt"
      {"i32.gt_u", TokenType::Compare, Opcode::I32GtU},
#line 341 "src/lexer-keywords.txt"
      {"i64.ge_s", TokenType::Compare, Opcode::I64GeS},
#line 224 "src/lexer-keywords.txt"
      {"i32.ge_s", TokenType::Compare, Opcode::I32GeS},
      {""}, {""},
#line 342 "src/lexer-keywords.txt"
      {"i64.ge_u", TokenType::Compare, Opcode::I64GeU},
#line 225 "src/lexer-keywords.txt"
      {"i32.ge_u", TokenType::Compare, Opcode::I32GeU},
#line 415 "src/lexer-keywords.txt"
      {"i8x16.mul", TokenType::Binary, Opcode::I8X16Mul},
      {""},
#line 175 "src/lexer-keywords.txt"
      {"i16x8.ne", TokenType::Compare, Opcode::I16X8Ne},
#line 36 "src/lexer-keywords.txt"
      {"catch", TokenType::Catch, Opcode::Catch},
      {""}, {""}, {""}, {""},
#line 366 "src/lexer-keywords.txt"
      {"i64.shr_s", TokenType::Binary, Opcode::I64ShrS},
#line 247 "src/lexer-keywords.txt"
      {"i32.shr_s", TokenType::Binary, Opcode::I32ShrS},
      {""}, {""},
#line 367 "src/lexer-keywords.txt"
      {"i64.shr_u", TokenType::Binary, Opcode::I64ShrU},
#line 248 "src/lexer-keywords.txt"
      {"i32.shr_u", TokenType::Binary, Opcode::I32ShrU},
      {""}, {""}, {""},
#line 81 "src/lexer-keywords.txt"
      {"f32x4.div", TokenType::Binary, Opcode::F32X4Div},
      {""}, {""},
#line 426 "src/lexer-keywords.txt"
      {"i8x16", TokenType::I8X16},
#line 71 "src/lexer-keywords.txt"
      {"f32.reinterpret_i32", TokenType::Convert, Opcode::F32ReinterpretI32},
      {""}, {""},
#line 264 "src/lexer-keywords.txt"
      {"i32x4.all_true", TokenType::Unary, Opcode::I32X4AllTrue},
      {""}, {""}, {""},
#line 436 "src/lexer-keywords.txt"
      {"memory.fill", TokenType::MemoryFill, Opcode::MemoryFill},
      {""},
#line 495 "src/lexer-keywords.txt"
      {"f64.promote/f32", TokenType::Convert, Opcode::F64PromoteF32},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 171 "src/lexer-keywords.txt"
      {"i16x8.lt_s", TokenType::Compare, Opcode::I16X8LtS},
      {""},
#line 172 "src/lexer-keywords.txt"
      {"i16x8.lt_u", TokenType::Compare, Opcode::I16X8LtU},
      {""},
#line 85 "src/lexer-keywords.txt"
      {"f32x4.gt", TokenType::Compare, Opcode::F32X4Gt},
      {""},
#line 168 "src/lexer-keywords.txt"
      {"i16x8.le_s", TokenType::Compare, Opcode::I16X8LeS},
      {""},
#line 169 "src/lexer-keywords.txt"
      {"i16x8.le_u", TokenType::Compare, Opcode::I16X8LeU},
      {""},
#line 88 "src/lexer-keywords.txt"
      {"f32x4.max", TokenType::Binary, Opcode::F32X4Max},
#line 292 "src/lexer-keywords.txt"
      {"i64.atomic.load16_u", TokenType::AtomicLoad, Opcode::I64AtomicLoad16U},
#line 187 "src/lexer-keywords.txt"
      {"i32.atomic.load16_u", TokenType::AtomicLoad, Opcode::I32AtomicLoad16U},
#line 323 "src/lexer-keywords.txt"
      {"i64.atomic.rmw.xor", TokenType::AtomicRmw, Opcode::I64AtomicRmwXor},
#line 210 "src/lexer-keywords.txt"
      {"i32.atomic.rmw.xor", TokenType::AtomicRmw, Opcode::I32AtomicRmwXor},
      {""},
#line 84 "src/lexer-keywords.txt"
      {"f32x4.ge", TokenType::Compare, Opcode::F32X4Ge},
      {""}, {""}, {""}, {""}, {""},
#line 270 "src/lexer-keywords.txt"
      {"i32x4.gt_s", TokenType::Compare, Opcode::I32X4GtS},
      {""},
#line 271 "src/lexer-keywords.txt"
      {"i32x4.gt_u", TokenType::Compare, Opcode::I32X4GtU},
      {""}, {""}, {""},
#line 268 "src/lexer-keywords.txt"
      {"i32x4.ge_s", TokenType::Compare, Opcode::I32X4GeS},
      {""},
#line 269 "src/lexer-keywords.txt"
      {"i32x4.ge_u", TokenType::Compare, Opcode::I32X4GeU},
      {""}, {""}, {""}, {""},
#line 386 "src/lexer-keywords.txt"
      {"i64x2.load_splat", TokenType::Load, Opcode::I64X2LoadSplat},
#line 158 "src/lexer-keywords.txt"
      {"i16x8.add", TokenType::Binary, Opcode::I16X8Add},
#line 514 "src/lexer-keywords.txt"
      {"i64.trunc_s:sat/f32", TokenType::Convert, Opcode::I64TruncSatF32S},
#line 502 "src/lexer-keywords.txt"
      {"i32.trunc_s:sat/f32", TokenType::Convert, Opcode::I32TruncSatF32S},
#line 518 "src/lexer-keywords.txt"
      {"i64.trunc_u:sat/f32", TokenType::Convert, Opcode::I64TruncSatF32U},
#line 506 "src/lexer-keywords.txt"
      {"i32.trunc_u:sat/f32", TokenType::Convert, Opcode::I32TruncSatF32U},
      {""}, {""},
#line 365 "src/lexer-keywords.txt"
      {"i64.shl", TokenType::Binary, Opcode::I64Shl},
#line 246 "src/lexer-keywords.txt"
      {"i32.shl", TokenType::Binary, Opcode::I32Shl},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 490 "src/lexer-keywords.txt"
      {"f32.reinterpret/i32", TokenType::Convert, Opcode::F32ReinterpretI32},
      {""}, {""},
#line 173 "src/lexer-keywords.txt"
      {"i16x8.mul", TokenType::Binary, Opcode::I16X8Mul},
      {""}, {""},
#line 316 "src/lexer-keywords.txt"
      {"i64.atomic.rmw8.xor_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw8XorU},
#line 203 "src/lexer-keywords.txt"
      {"i32.atomic.rmw8.xor_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw8XorU},
      {""}, {""},
#line 423 "src/lexer-keywords.txt"
      {"i8x16.sub_saturate_s", TokenType::Binary, Opcode::I8X16SubSaturateS},
      {""},
#line 424 "src/lexer-keywords.txt"
      {"i8x16.sub_saturate_u", TokenType::Binary, Opcode::I8X16SubSaturateU},
      {""}, {""},
#line 390 "src/lexer-keywords.txt"
      {"i64x2.shr_s", TokenType::Binary, Opcode::I64X2ShrS},
      {""}, {""}, {""},
#line 391 "src/lexer-keywords.txt"
      {"i64x2.shr_u", TokenType::Binary, Opcode::I64X2ShrU},
      {""}, {""}, {""}, {""},
#line 401 "src/lexer-keywords.txt"
      {"i8x16.all_true", TokenType::Unary, Opcode::I8X16AllTrue},
#line 384 "src/lexer-keywords.txt"
      {"i64x2.any_true", TokenType::Unary, Opcode::I64X2AnyTrue},
      {""}, {""},
#line 144 "src/lexer-keywords.txt"
      {"f64x2.replace_lane", TokenType::SimdLaneOp, Opcode::F64X2ReplaceLane},
      {""},
#line 443 "src/lexer-keywords.txt"
      {"nop", TokenType::Nop, Opcode::Nop},
#line 388 "src/lexer-keywords.txt"
      {"i64x2.replace_lane", TokenType::SimdLaneOp, Opcode::I64X2ReplaceLane},
      {""}, {""},
#line 395 "src/lexer-keywords.txt"
      {"i64x2.trunc_sat_f64x2_s", TokenType::Unary, Opcode::I64X2TruncSatF64X2S},
      {""},
#line 396 "src/lexer-keywords.txt"
      {"i64x2.trunc_sat_f64x2_u", TokenType::Unary, Opcode::I64X2TruncSatF64X2U},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 471 "src/lexer-keywords.txt"
      {"unreachable", TokenType::Unreachable, Opcode::Unreachable},
      {""},
#line 484 "src/lexer-keywords.txt"
      {"anyfunc", Type::Funcref},
      {""},
#line 122 "src/lexer-keywords.txt"
      {"f64.reinterpret_i64", TokenType::Convert, Opcode::F64ReinterpretI64},
      {""}, {""}, {""},
#line 262 "src/lexer-keywords.txt"
      {"i32.wrap_i64", TokenType::Convert, Opcode::I32WrapI64},
      {""}, {""}, {""},
#line 408 "src/lexer-keywords.txt"
      {"i8x16.gt_s", TokenType::Compare, Opcode::I8X16GtS},
      {""},
#line 409 "src/lexer-keywords.txt"
      {"i8x16.gt_u", TokenType::Compare, Opcode::I8X16GtU},
#line 520 "src/lexer-keywords.txt"
      {"set_global", TokenType::GlobalSet, Opcode::GlobalSet},
      {""}, {""},
#line 406 "src/lexer-keywords.txt"
      {"i8x16.ge_s", TokenType::Compare, Opcode::I8X16GeS},
      {""},
#line 407 "src/lexer-keywords.txt"
      {"i8x16.ge_u", TokenType::Compare, Opcode::I8X16GeU},
      {""}, {""},
#line 83 "src/lexer-keywords.txt"
      {"f32x4.extract_lane", TokenType::SimdLaneOp, Opcode::F32X4ExtractLane},
      {""}, {""},
#line 267 "src/lexer-keywords.txt"
      {"i32x4.extract_lane", TokenType::SimdLaneOp, Opcode::I32X4ExtractLane},
      {""}, {""}, {""}, {""},
#line 398 "src/lexer-keywords.txt"
      {"i8x16.add_saturate_s", TokenType::Binary, Opcode::I8X16AddSaturateS},
      {""},
#line 399 "src/lexer-keywords.txt"
      {"i8x16.add_saturate_u", TokenType::Binary, Opcode::I8X16AddSaturateU},
      {""}, {""}, {""}, {""}, {""},
#line 326 "src/lexer-keywords.txt"
      {"i64.atomic.store8", TokenType::AtomicStore, Opcode::I64AtomicStore8},
#line 212 "src/lexer-keywords.txt"
      {"i32.atomic.store8", TokenType::AtomicStore, Opcode::I32AtomicStore8},
      {""}, {""}, {""}, {""}, {""},
#line 481 "src/lexer-keywords.txt"
      {"v8x16.swizzle", TokenType::Binary, Opcode::V8X16Swizzle},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 389 "src/lexer-keywords.txt"
      {"i64x2.shl", TokenType::Binary, Opcode::I64X2Shl},
      {""},
#line 181 "src/lexer-keywords.txt"
      {"i16x8.sub_saturate_s", TokenType::Binary, Opcode::I16X8SubSaturateS},
      {""},
#line 182 "src/lexer-keywords.txt"
      {"i16x8.sub_saturate_u", TokenType::Binary, Opcode::I16X8SubSaturateU},
#line 515 "src/lexer-keywords.txt"
      {"i64.trunc_s:sat/f64", TokenType::Convert, Opcode::I64TruncSatF64S},
#line 503 "src/lexer-keywords.txt"
      {"i32.trunc_s:sat/f64", TokenType::Convert, Opcode::I32TruncSatF64S},
#line 519 "src/lexer-keywords.txt"
      {"i64.trunc_u:sat/f64", TokenType::Convert, Opcode::I64TruncSatF64U},
#line 507 "src/lexer-keywords.txt"
      {"i32.trunc_u:sat/f64", TokenType::Convert, Opcode::I32TruncSatF64U},
      {""},
#line 184 "src/lexer-keywords.txt"
      {"i16x8", TokenType::I16X8},
      {""}, {""}, {""}, {""}, {""},
#line 159 "src/lexer-keywords.txt"
      {"i16x8.all_true", TokenType::Unary, Opcode::I16X8AllTrue},
      {""}, {""},
#line 496 "src/lexer-keywords.txt"
      {"f64.reinterpret/i64", TokenType::Convert, Opcode::F64ReinterpretI64},
      {""}, {""}, {""},
#line 508 "src/lexer-keywords.txt"
      {"i32.wrap/i64", TokenType::Convert, Opcode::I32WrapI64},
      {""}, {""},
#line 108 "src/lexer-keywords.txt"
      {"f64.eq", TokenType::Compare, Opcode::F64Eq},
#line 58 "src/lexer-keywords.txt"
      {"f32.eq", TokenType::Compare, Opcode::F32Eq},
      {""},
#line 334 "src/lexer-keywords.txt"
      {"i64.eq", TokenType::Compare, Opcode::I64Eq},
#line 220 "src/lexer-keywords.txt"
      {"i32.eq", TokenType::Compare, Opcode::I32Eq},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 379 "src/lexer-keywords.txt"
      {"i64.trunc_sat_f64_s", TokenType::Convert, Opcode::I64TruncSatF64S},
#line 259 "src/lexer-keywords.txt"
      {"i32.trunc_sat_f64_s", TokenType::Convert, Opcode::I32TruncSatF64S},
#line 25 "src/lexer-keywords.txt"
      {"assert_trap", TokenType::AssertTrap},
      {""},
#line 380 "src/lexer-keywords.txt"
      {"i64.trunc_sat_f64_u", TokenType::Convert, Opcode::I64TruncSatF64U},
#line 260 "src/lexer-keywords.txt"
      {"i32.trunc_sat_f64_u", TokenType::Convert, Opcode::I32TruncSatF64U},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 166 "src/lexer-keywords.txt"
      {"i16x8.gt_s", TokenType::Compare, Opcode::I16X8GtS},
      {""},
#line 167 "src/lexer-keywords.txt"
      {"i16x8.gt_u", TokenType::Compare, Opcode::I16X8GtU},
      {""}, {""},
#line 22 "src/lexer-keywords.txt"
      {"assert_return_arithmetic_nan", TokenType::AssertReturnArithmeticNan},
#line 164 "src/lexer-keywords.txt"
      {"i16x8.ge_s", TokenType::Compare, Opcode::I16X8GeS},
      {""},
#line 165 "src/lexer-keywords.txt"
      {"i16x8.ge_u", TokenType::Compare, Opcode::I16X8GeU},
      {""}, {""},
#line 95 "src/lexer-keywords.txt"
      {"f32x4.sqrt", TokenType::Unary, Opcode::F32X4Sqrt},
      {""}, {""}, {""},
#line 437 "src/lexer-keywords.txt"
      {"memory.grow", TokenType::MemoryGrow, Opcode::MemoryGrow},
#line 79 "src/lexer-keywords.txt"
      {"f32x4.convert_i32x4_s", TokenType::Unary, Opcode::F32X4ConvertI32X4S},
      {""},
#line 80 "src/lexer-keywords.txt"
      {"f32x4.convert_i32x4_u", TokenType::Unary, Opcode::F32X4ConvertI32X4U},
#line 156 "src/lexer-keywords.txt"
      {"i16x8.add_saturate_s", TokenType::Binary, Opcode::I16X8AddSaturateS},
#line 404 "src/lexer-keywords.txt"
      {"i8x16.extract_lane_s", TokenType::SimdLaneOp, Opcode::I8X16ExtractLaneS},
#line 157 "src/lexer-keywords.txt"
      {"i16x8.add_saturate_u", TokenType::Binary, Opcode::I16X8AddSaturateU},
#line 405 "src/lexer-keywords.txt"
      {"i8x16.extract_lane_u", TokenType::SimdLaneOp, Opcode::I8X16ExtractLaneU},
      {""},
#line 241 "src/lexer-keywords.txt"
      {"i32.reinterpret_f32", TokenType::Convert, Opcode::I32ReinterpretF32},
      {""}, {""}, {""},
#line 94 "src/lexer-keywords.txt"
      {"f32x4.splat", TokenType::Unary, Opcode::F32X4Splat},
      {""}, {""},
#line 284 "src/lexer-keywords.txt"
      {"i32x4.splat", TokenType::Unary, Opcode::I32X4Splat},
      {""}, {""}, {""},
#line 41 "src/lexer-keywords.txt"
      {"elem.drop", TokenType::ElemDrop, Opcode::ElemDrop},
      {""}, {""}, {""}, {""},
#line 38 "src/lexer-keywords.txt"
      {"data.drop", TokenType::DataDrop, Opcode::DataDrop},
      {""}, {""},
#line 492 "src/lexer-keywords.txt"
      {"f64.convert_s/i64", TokenType::Convert, Opcode::F64ConvertI64S},
#line 486 "src/lexer-keywords.txt"
      {"f32.convert_s/i64", TokenType::Convert, Opcode::F32ConvertI64S},
#line 494 "src/lexer-keywords.txt"
      {"f64.convert_u/i64", TokenType::Convert, Opcode::F64ConvertI64U},
#line 488 "src/lexer-keywords.txt"
      {"f32.convert_u/i64", TokenType::Convert, Opcode::F32ConvertI64U},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""},
#line 359 "src/lexer-keywords.txt"
      {"i64.popcnt", TokenType::Unary, Opcode::I64Popcnt},
#line 240 "src/lexer-keywords.txt"
      {"i32.popcnt", TokenType::Unary, Opcode::I32Popcnt},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 133 "src/lexer-keywords.txt"
      {"f64x2.eq", TokenType::Compare, Opcode::F64X2Eq},
      {""},
#line 499 "src/lexer-keywords.txt"
      {"i32.reinterpret/f32", TokenType::Convert, Opcode::I32ReinterpretF32},
      {""}, {""}, {""}, {""}, {""},
#line 91 "src/lexer-keywords.txt"
      {"f32x4.neg", TokenType::Unary, Opcode::F32X4Neg},
      {""}, {""},
#line 278 "src/lexer-keywords.txt"
      {"i32x4.neg", TokenType::Unary, Opcode::I32X4Neg},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 469 "src/lexer-keywords.txt"
      {"try", TokenType::Try, Opcode::Try},
      {""}, {""}, {""},
#line 162 "src/lexer-keywords.txt"
      {"i16x8.extract_lane_s", TokenType::SimdLaneOp, Opcode::I16X8ExtractLaneS},
#line 287 "src/lexer-keywords.txt"
      {"i32x4.trunc_sat_f32x4_s", TokenType::Unary, Opcode::I32X4TruncSatF32X4S},
#line 163 "src/lexer-keywords.txt"
      {"i16x8.extract_lane_u", TokenType::SimdLaneOp, Opcode::I16X8ExtractLaneU},
#line 288 "src/lexer-keywords.txt"
      {"i32x4.trunc_sat_f32x4_u", TokenType::Unary, Opcode::I32X4TruncSatF32X4U},
      {""}, {""}, {""}, {""},
#line 422 "src/lexer-keywords.txt"
      {"i8x16.splat", TokenType::Unary, Opcode::I8X16Splat},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 460 "src/lexer-keywords.txt"
      {"table.copy", TokenType::TableCopy, Opcode::TableCopy},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 130 "src/lexer-keywords.txt"
      {"f64x2.convert_i64x2_s", TokenType::Unary, Opcode::F64X2ConvertI64X2S},
#line 440 "src/lexer-keywords.txt"
      {"memory", TokenType::Memory},
#line 131 "src/lexer-keywords.txt"
      {"f64x2.convert_i64x2_u", TokenType::Unary, Opcode::F64X2ConvertI64X2U},
      {""}, {""},
#line 360 "src/lexer-keywords.txt"
      {"i64.reinterpret_f64", TokenType::Convert, Opcode::I64ReinterpretF64},
      {""}, {""}, {""}, {""}, {""},
#line 319 "src/lexer-keywords.txt"
      {"i64.atomic.rmw.cmpxchg", TokenType::AtomicRmwCmpxchg, Opcode::I64AtomicRmwCmpxchg},
#line 206 "src/lexer-keywords.txt"
      {"i32.atomic.rmw.cmpxchg", TokenType::AtomicRmwCmpxchg, Opcode::I32AtomicRmwCmpxchg},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 28 "src/lexer-keywords.txt"
      {"binary", TokenType::Bin},
      {""}, {""}, {""}, {""}, {""},
#line 497 "src/lexer-keywords.txt"
      {"get_global", TokenType::GlobalGet, Opcode::GlobalGet},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 274 "src/lexer-keywords.txt"
      {"i32x4.load_splat", TokenType::Load, Opcode::I32X4LoadSplat},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 416 "src/lexer-keywords.txt"
      {"i8x16.neg", TokenType::Unary, Opcode::I8X16Neg},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""},
#line 511 "src/lexer-keywords.txt"
      {"i64.reinterpret/f64", TokenType::Convert, Opcode::I64ReinterpretF64},
#line 180 "src/lexer-keywords.txt"
      {"i16x8.splat", TokenType::Unary, Opcode::I16X8Splat},
      {""}, {""},
#line 282 "src/lexer-keywords.txt"
      {"i32x4.shr_s", TokenType::Binary, Opcode::I32X4ShrS},
      {""}, {""}, {""},
#line 283 "src/lexer-keywords.txt"
      {"i32x4.shr_u", TokenType::Binary, Opcode::I32X4ShrU},
      {""}, {""}, {""}, {""}, {""},
#line 265 "src/lexer-keywords.txt"
      {"i32x4.any_true", TokenType::Unary, Opcode::I32X4AnyTrue},
      {""}, {""},
#line 93 "src/lexer-keywords.txt"
      {"f32x4.replace_lane", TokenType::SimdLaneOp, Opcode::F32X4ReplaceLane},
      {""}, {""},
#line 280 "src/lexer-keywords.txt"
      {"i32x4.replace_lane", TokenType::SimdLaneOp, Opcode::I32X4ReplaceLane},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 412 "src/lexer-keywords.txt"
      {"i8x16.load_splat", TokenType::Load, Opcode::I8X16LoadSplat},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 174 "src/lexer-keywords.txt"
      {"i16x8.neg", TokenType::Unary, Opcode::I16X8Neg},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 281 "src/lexer-keywords.txt"
      {"i32x4.shl", TokenType::Binary, Opcode::I32X4Shl},
      {""}, {""}, {""},
#line 420 "src/lexer-keywords.txt"
      {"i8x16.shr_s", TokenType::Binary, Opcode::I8X16ShrS},
      {""}, {""}, {""},
#line 421 "src/lexer-keywords.txt"
      {"i8x16.shr_u", TokenType::Binary, Opcode::I8X16ShrU},
      {""}, {""}, {""}, {""}, {""},
#line 402 "src/lexer-keywords.txt"
      {"i8x16.any_true", TokenType::Unary, Opcode::I8X16AnyTrue},
      {""}, {""}, {""}, {""}, {""},
#line 418 "src/lexer-keywords.txt"
      {"i8x16.replace_lane", TokenType::SimdLaneOp, Opcode::I8X16ReplaceLane},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 37 "src/lexer-keywords.txt"
      {"current_memory", TokenType::MemorySize, Opcode::MemorySize},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""},
#line 170 "src/lexer-keywords.txt"
      {"i16x8.load_splat", TokenType::Load, Opcode::I16X8LoadSplat},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 27 "src/lexer-keywords.txt"
      {"atomic.notify", TokenType::AtomicNotify, Opcode::AtomicNotify},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 419 "src/lexer-keywords.txt"
      {"i8x16.shl", TokenType::Binary, Opcode::I8X16Shl},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 178 "src/lexer-keywords.txt"
      {"i16x8.shr_s", TokenType::Binary, Opcode::I16X8ShrS},
      {""}, {""}, {""},
#line 179 "src/lexer-keywords.txt"
      {"i16x8.shr_u", TokenType::Binary, Opcode::I16X8ShrU},
#line 106 "src/lexer-keywords.txt"
      {"f64.copysign", TokenType::Binary, Opcode::F64Copysign},
#line 55 "src/lexer-keywords.txt"
      {"f32.copysign", TokenType::Binary, Opcode::F32Copysign},
      {""}, {""}, {""},
#line 160 "src/lexer-keywords.txt"
      {"i16x8.any_true", TokenType::Unary, Opcode::I16X8AnyTrue},
      {""}, {""}, {""}, {""}, {""},
#line 176 "src/lexer-keywords.txt"
      {"i16x8.replace_lane", TokenType::SimdLaneOp, Opcode::I16X8ReplaceLane},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 82 "src/lexer-keywords.txt"
      {"f32x4.eq", TokenType::Compare, Opcode::F32X4Eq},
      {""}, {""},
#line 266 "src/lexer-keywords.txt"
      {"i32x4.eq", TokenType::Compare, Opcode::I32X4Eq},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 155 "src/lexer-keywords.txt"
      {"grow_memory", TokenType::MemoryGrow, Opcode::MemoryGrow},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 177 "src/lexer-keywords.txt"
      {"i16x8.shl", TokenType::Binary, Opcode::I16X8Shl},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 482 "src/lexer-keywords.txt"
      {"v8x16.shuffle", TokenType::SimdShuffleOp, Opcode::V8X16Shuffle},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 403 "src/lexer-keywords.txt"
      {"i8x16.eq", TokenType::Compare, Opcode::I8X16Eq},
      {""}, {""},
#line 315 "src/lexer-keywords.txt"
      {"i64.atomic.rmw8.xchg_u", TokenType::AtomicRmw, Opcode::I64AtomicRmw8XchgU},
#line 202 "src/lexer-keywords.txt"
      {"i32.atomic.rmw8.xchg_u", TokenType::AtomicRmw, Opcode::I32AtomicRmw8XchgU},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 322 "src/lexer-keywords.txt"
      {"i64.atomic.rmw.xchg", TokenType::AtomicRmw, Opcode::I64AtomicRmwXchg},
#line 209 "src/lexer-keywords.txt"
      {"i32.atomic.rmw.xchg", TokenType::AtomicRmw, Opcode::I32AtomicRmwXchg},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 161 "src/lexer-keywords.txt"
      {"i16x8.eq", TokenType::Compare, Opcode::I16X8Eq},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""},
#line 435 "src/lexer-keywords.txt"
      {"memory.copy", TokenType::MemoryCopy, Opcode::MemoryCopy}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          const char *s = wordlist[key].name;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return &wordlist[key];
        }
    }
  return 0;
}
