%{ /*** C/C++ Declarations ***/

#include <stdio.h>
#include <string>
#include <vector>

#include "src/interpreter/nodes.h"

%}

/*** yacc/bison Declarations ***/

/* Require bison 2.3 or later */
%require "2.3"

/* add debug output code to generated parser. disable this for release
 * versions. */
%debug

/* start symbol is named "start" */
%start constant

/* write out a header file containing the token defines */
%defines

/* use newer C++ skeleton file */
%skeleton "lalr1.cc"

/* namespace to enclose parser in */
%define api.namespace {wasmati}

/* set the parser's class identifier */
%define parser_class_name {Parser}

/* keep track of the current position within the input */
%locations
%initial-action
{
    // initialize the initial location object
    @$.begin.filename = @$.end.filename = &driver.streamname;
};

/* The interpreter is passed by reference to the parser and to the scanner. This
 * provides a simple but effective pure interface, not relying on global
 * variables. */
%parse-param { class Interpreter& driver }

/* verbose error messages */
%error-verbose

 /*** BEGIN EXAMPLE - Change the example grammar's tokens below ***/

%union {
	int                   i;	/* integer value */
	double                r;      /* double value */
    std::string          *s;	/* symbol name or string literal */;

}

%token <i> INT
%token <r> DOUBLE
%token <s> IDENTIFIER STRING
%token EOL
%token OR EQUAL ASSIGN AND IN NOT
%token ELSE END THEN FOREACH NIL DO
%token IF
%token LBRACE RBRACE LBRACKET RBRACKET CLBRACKET CRBRACKET DOT COMMA COLON SEMICOLON VERT

%nonassoc IF 
%nonassoc ELSE
%left OR
%left AND
%nonassoc NOT
%left EQUAL
%left '<' '>'
%left '+' '-'
%left '*' '/' '%'

 /*** END EXAMPLE - Change the example grammar's tokens above ***/

%{

#include "src/interpreter/interpreter.h"
#include "src/interpreter/scanner.h"

/* this "connects" the bison parser in the interp to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the interp context. */
#undef yylex
#define yylex driver.lexer->lex

%}

%% /*** Grammar Rules ***/

 /*** BEGIN EXAMPLE - Change the example grammar rules below ***/

constant : INT {}
         | DOUBLE {}



 /*** END EXAMPLE - Change the example grammar rules above ***/

%% /*** Additional Code ***/

void wasmati::Parser::error(const Parser::location_type& l,
			                const std::string& m) {
    driver.error(l, m);
}
