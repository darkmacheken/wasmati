%code requires {
/*** C/C++ Declarations ***/

#include <stdio.h>
#include <string>
#include <vector>
#include "src/interpreter/nodes.h"

#define LINE driver.lineno()

}

/*** yacc/bison Declarations ***/
%locations

/* Require bison 2.3 or later */
%require "2.3"

/* add debug output code to generated parser. disable this for release
 * versions. */
%debug

/* start symbol is named "start" */
%start prog_target

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
    BasicNode             *node;	/* node pointer */  
    BlockNode             *block;	
    SequenceNode          *sequence;
    SequenceExprNode      *sequenceExpr;
    SequenceLiteralNode   *sequenceLiteral;
    ExpressionNode        *expression; /* expression nodes */
    std::vector<std::string> *identifiers;
}

%token <i> INT
%token <r> FLOAT
%token <s> IDENTIFIER STRING
%token END 0 "End of file"
%token LBRACE "(" RBRACE ")" LBRACKET "[" RBRACKET "]" CLBRACKET "{" CRBRACKET "}"
%token DOT "." COMMA "," COLON ":" SEMICOLON ";"
%token NOT "!"
%token EQ_OP "=" NE_OP "!="
%token AND_OP "&&" OR_OP "||"
%token ASSIGN ":=" IN 
%token LT_OP "<" LE_OP "<=" GT_OP ">" GE_OP ">="
%token PLUS "+" SUB "-"
%token MUL "*" DIV "/" MOD "%"
%token FALSE TRUE
%token ELSE FOREACH NIL
%token IF
%token FUNCTION RETURN TIME IMPORT

%precedence THEN
%precedence ELSE
%nonassoc NOT INT STRING IDENTIFIER
%left OR_OP 
%left AND_OP
%left IN 
%left EQ_OP NE_OP
%left LT_OP LE_OP GT_OP GE_OP
%left PLUS SUB
%left MUL DIV MOD
%left LBRACE LBRACKET DOT

%type <node> stmt if_stmt iteration_stmt function
%type <block> block
%type <sequence> stmts
%type <sequenceExpr> exprs
%type <expression> expr binop unop boolean literal integer float string call
%type <identifiers> identifiers

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

prog_target : END           {/* ignore */}
            | stmts END     {driver.ast($1);}
            | expr END      {driver.ast($1);}
            ;

stmts 
     : stmt                                       {$$ = new SequenceNode(LINE, $1);}   
     | stmts stmt                                 {$$ = new SequenceNode(LINE, $2, $1);}

stmt 
     : iteration_stmt                             {$$ = $1;}
     | if_stmt                                    {$$ = $1;}
     | block                                      {$$ = $1;}
     | function                                   {$$ = $1;}
     | expr ";"                                   {$$ = $1;}   
     | ";"                                        {/* ignore */}
     | RETURN ";"                                 {$$ = new ReturnNode(LINE);}
     | RETURN expr ";"                            {$$ = new ReturnNode(LINE, $2);}
     | IMPORT STRING                              {$$ = new ImportNode(LINE, $2);}
     ;

block
     : "{" "}"                                    {$$ = new BlockNode(LINE, new SequenceNode(LINE));}
     | "{" stmts "}"                              {$$ = new BlockNode(LINE, $2);}
     ;

if_stmt
     : IF "(" expr ")" stmt %prec THEN            {$$ = new IfNode(LINE, $3, $5);}
     | IF "(" expr ")" stmt ELSE stmt             {$$ = new IfElseNode(LINE, $3, $5, $7);}
     ;

iteration_stmt
     : FOREACH IDENTIFIER IN expr stmt            {$$ = new Foreach(LINE, $2, $4, $5);}
     ;

function
     : FUNCTION IDENTIFIER "(" ")" block               {$$ = new FunctionNode(LINE, $2, $5);}
     | FUNCTION IDENTIFIER "(" identifiers ")" block   {$$ = new FunctionNode(LINE, $2, $6, $4);}
     ;

identifiers
     : IDENTIFIER                                 {$$ = new std::vector<std::string>(); $$->push_back(*$1);}
     | identifiers "," IDENTIFIER                 {$1->push_back(*$3); $$ = $1;}
     ;

expr : "[" IDENTIFIER IN expr ":" expr "]"        {$$ = new FilterExprNode(LINE, $2, $4, $6);}
     | IDENTIFIER ":=" expr                       {$$ = new AssignExpr(LINE, $1, $3);}
     | binop                                      {$$ = $1;}
     | unop                                       {$$ = $1;}
     | IDENTIFIER                                 {$$ = new RValueNode(LINE, $1); delete $1;}
     | literal                                    {$$ = $1;}
     | call                                       {$$ = $1;}
     | "(" expr ")"                               {$$ = $2;}
     | expr "[" expr "]"                          {$$ = new AtNode(LINE, $1, $3);}
     | TIME "(" expr ")"                          {$$ = new TimeNode(LINE, $3);}
     ;

binop 
     : expr "=" expr                              {$$ = new EqualNode(LINE, $1, $3);}
     | expr "!=" expr                             {$$ = new NotEqualNode(LINE, $1, $3);}
     | expr "&&" expr                             {$$ = new AndNode(LINE, $1, $3);}
     | expr "||" expr                             {$$ = new OrNode(LINE, $1, $3);}
     | expr IN expr                               {$$ = new InNode(LINE, $1, $3);}
     | expr "<" expr                              {$$ = new LessNode(LINE, $1, $3);}
     | expr "<=" expr                             {$$ = new LessEqualNode(LINE, $1, $3);}
     | expr ">" expr                              {$$ = new GreaterNode(LINE, $1, $3);}
     | expr ">=" expr                             {$$ = new GreaterEqualNode(LINE, $1, $3);}
     | expr "+" expr                              {$$ = new AddNode(LINE, $1, $3);}
     | expr "-" expr                              {$$ = new SubNode(LINE, $1, $3);}
     | expr "*" expr                              {$$ = new MulNode(LINE, $1, $3);}
     | expr "/" expr                              {$$ = new DivNode(LINE, $1, $3);}
     | expr "%" expr                              {$$ = new ModNode(LINE, $1, $3);}
     ;

unop
     : NOT expr                                   {$$ = new NotNode(LINE, $2);}
     ;

call 
     : IDENTIFIER "(" ")"                         {$$ = new FunctionCall(LINE, $1);}
     | IDENTIFIER "(" exprs ")"                   {$$ = new FunctionCall(LINE, $1, $3);}
     | expr "." IDENTIFIER                        {$$ = new AttributeCall(LINE, $3, $1);}
     | expr "." IDENTIFIER "(" ")"                {$$ = new MemberFunctionCall(LINE, $3, $1);}
     | expr "." IDENTIFIER "(" exprs ")"          {$$ = new MemberFunctionCall(LINE, $3, $1, $5);}
     ;

exprs : expr                                      { $$ = new SequenceExprNode(LINE, $1); }
      | exprs "," expr                            { $$ = new SequenceExprNode(LINE, $3, $1); }
      ; 

literal : integer                                 { $$ = $1; }
        | float                                   { $$ = $1; }
        | string                                  { $$ = $1; }
        | boolean                                 { $$ = $1; }
        | NIL                                     { $$ = new NilNode(LINE); }
        ;

integer : INT                                     { $$ = new IntNode(LINE, $1); }
        ;

float : 
     FLOAT                                        { $$ = new FloatNode(LINE, $1); }
     ;

string : STRING                                   { $$ = new StringNode(LINE, *$1); delete $1;}
       ;

boolean 
     : FALSE                                      {$$ = new BoolNode(LINE, false);}
     | TRUE                                       {$$ = new BoolNode(LINE, true);}
     ;

%% /*** Additional Code ***/

void wasmati::Parser::error(const Parser::location_type& l,
			                const std::string& m) {
    driver.error(l, m);
}
