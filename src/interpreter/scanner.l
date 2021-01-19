%{ /*** C/C++ Declarations ***/

#include <string>

#include "src/interpreter/scanner.h"

/* import the parser's token type into a local typedef */
typedef wasmati::Parser::token token;
typedef wasmati::Parser::token_type token_type;

/* By default yylex returns int, we use token_type. Unfortunately yyterminate
 * by default returns 0, which is not of token_type. */
#define yyterminate() return token::END

/* This disables inclusion of unistd.h, which is not available under Visual C++
 * on Win32. The C++ scanner uses STL streams instead. */
#define YY_NO_UNISTD_H

// don't change this
#define yyerror LexerError

extern "C" int yylex();

%}

/*** Flex Declarations and Options ***/
%option yylineno 8bit

/* enable c++ scanner class generation */
%option c++

/* change the name of the scanner class. results in "WasmatiFlexLexer" */
%option prefix="Wasmati"

/* the manual says "somewhat more optimized" */
%option batch

/* enable scanner to generate debug output. disable this for release
 * versions. */
%option debug

/* no support for include files is planned */
%option yywrap nounput 

/* enables the use of start condition stacks */
%option stack

/* The following paragraph suffices to track locations accurately. Each time
 * yylex is invoked, the begin position is moved onto the end position. */
%{
#define YY_USER_ACTION  yylloc->columns(yyleng);
%}

%x X_COMMENT X_STRING X_STRING_ESCAPECHAR X_STRING_IGNORE X_NEW_LINE
IDENTIFIER                                  [A-Za-z][A-Za-z0-9\-]*
SPACE                                       [ \t]

INT                                         [1-9]+[0-9]*|0
EXPONENT                                    [eE][-+]?[0-9]+
REAL                                        [0-9]*\.[0-9]+{EXPONENT}?|[0-9]+\.{EXPONENT}?|[0-9]+{EXPONENT}

EMPTY_LINE                                  ^{SPACE}*\n
%% /*** Regular Expressions Part ***/

 /* code to place at the beginning of yylex() */
%{
    // reset location
    yylloc->step();
%}

 /*** BEGIN REGEX - Change the example lexer rules below ***/
"="                  { return token::EQUAL;         }
"in"                 { return token::IN;            }
"&&"                 { return token::AND;           }
"||"                 { return token::OR;            }
"!"                  { return token::NOT;           }
"foreach"            { return token::FOREACH;       }
":="                 { return token::ASSIGN;        }
"if"                 { return token::IF;            }
"do"                 { return token::DO;            }
"then"               { return token::THEN;          }
"else"               { return token::ELSE;          }
"end"                { return token::END;           }
"nil"                { return token::NIL;           }

"("                  { return token::LBRACE;         }
")"                  { return token::RBRACE;         }
"["                  { return token::LBRACKET;       }
"]"                  { return token::RBRACKET;       }
"{"                  { return token::CLBRACKET;      }
"}"                  { return token::CRBRACKET;      }
"."                  { return token::DOT;            }
","                  { return token::COMMA;          }
":"                  { return token::COLON;          }
";"                  { return token::SEMICOLON;      }
"|"                  { return token::VERT;           }


<*>{EMPTY_LINE}                             ; /*ignore*/

{IDENTIFIER}        {yylval->s = new std::string(yytext); return token::IDENTIFIER; }

{INT}               {
                        yylval->i = strtol(yytext, nullptr, 10); 
                        if(errno == ERANGE) 
                        yyerror("ERROR: Overflow! The value cannot be stored in 64bit");
                        return token::INT;
                    }

{REAL}              {
                        yylval->r = strtod(yytext, nullptr);
                        if(errno == ERANGE) 
                        yyerror("ERROR: Overflow! The value can not be represented in IEEE754.");
                        return token::DOUBLE;
                    }                    

"//".*\n            ; /*ignore line comment*/

"/*"                                        yy_push_state(X_COMMENT);
<X_COMMENT>"*/"                             yy_pop_state();
<X_COMMENT>"*/"\n                           yy_pop_state();
<X_COMMENT>.|\n                             ; /*ignore: belongs to comment*/

\"                                          yy_push_state(X_STRING); yylval->s = new std::string("");
<X_STRING>\"                                yy_pop_state(); return token::STRING;
<X_STRING>\\                                yy_push_state(X_STRING_ESCAPECHAR);
<X_STRING>\0                                yyerror("null char in string");
<X_STRING>.                                 *yylval->s += yytext;
<X_STRING>\n                                yyerror("newline in string");

<X_STRING_ESCAPECHAR>n                      *yylval->s += "\n"; yy_pop_state();
<X_STRING_ESCAPECHAR>r                      *yylval->s += "\r"; yy_pop_state();
<X_STRING_ESCAPECHAR>t                      *yylval->s += "\t"; yy_pop_state();
<X_STRING_ESCAPECHAR>\"                     *yylval->s += "\""; yy_pop_state();
<X_STRING_ESCAPECHAR>.                      yy_pop_state(); *yylval->s +=std::string(yytext);

[ \t]+                                     ; /* ignore whitespace */

.                                          yyerror("LEXERROR: Unknown char");


 /*** END REGEX - Change the example lexer rules above ***/

%% /*** Additional Code ***/

namespace wasmati {

Scanner::Scanner(std::istream* in,
		         std::ostream* out)
        : WasmatiFlexLexer(in, out){}

Scanner::~Scanner(){}

void Scanner::set_debug(bool b){
    yy_flex_debug = b;
}

}

/* This implementation of WasmatiFlexLexer::yylex() is required to fill the
 * vtable of the class ExampleFlexLexer. We define the scanner's main yylex
 * function via YY_DECL to reside in the Scanner class instead. */

#ifdef yylex
#undef yylex
#endif

int WasmatiFlexLexer::yylex(){
    std::cerr << "in Lexer::yylex() !" << std::endl;
    return 0;
}

/* When the scanner receives an end-of-file indication from YY_INPUT, it then
 * checks the yywrap() function. If yywrap() returns false (zero), then it is
 * assumed that the function has gone ahead and set up `yyin' to point to
 * another input file, and scanning continues. If it returns true (non-zero),
 * then the scanner terminates, returning 0 to its caller. */

int WasmatiFlexLexer::yywrap(){
    return 1;
}