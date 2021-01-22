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
    wasmati::BasicNode             *node;	/* node pointer */  
    wasmati::SequenceNode          *sequence;
    wasmati::SequenceExprNode      *sequenceExpr;
    wasmati::SequenceLiteralNode   *sequenceLiteral;
    wasmati::ExpressionNode        *expression; /* expression nodes */
    wasmati::LiteralNode           *literal; /* expression nodes */
    wasmati::StringNode            *string;
    wasmati::IntNode               *integerNode;
    wasmati::FloatNode             *doubleNode;
    wasmati::ListNode              *listNode;
    wasmati::LValueNode            *lvalue;
    wasmati::BlockNode             *block;
}

%token <i> INT
%token <r> DOUBLE
%token <s> IDENTIFIER STRING
%token END 0 "end of file"
%token OR EQUAL NEQUAL ASSIGN AND IN NOT FALSE TRUE
%token ELSE FOREACH NIL
%token IF
%token LBRACE RBRACE LBRACKET RBRACKET CLBRACKET CRBRACKET DOT COMMA COLON SEMICOLON VERT

%nonassoc IF 
%nonassoc ELSE
%left OR
%left AND
%nonassoc IN DOT
%nonassoc NOT INT DOUBLE STRING IDENTIFIER
%left EQUAL NEQUAL
%left '<' '>'
%left '+' '-'
%left '*' '/' '%'
%nonassoc tUNARY
%nonassoc tCALL
%nonassoc tINDEXING

%type <node> stmt 
%type <sequence> stmts
%type <sequenceExpr> exprs
%type <sequenceLiteral> literals
%type <literal> literal
%type <expression> expr
%type <lvalue> lval
%type <integerNode> integer
%type <doubleNode> double
%type <string> string
%type <listNode> list

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

prog_target : END           {driver.ast(new SequenceNode(LINE));}
            | stmts END     {driver.ast($1);}
            | expr END      {driver.ast($1);}
            ;


stmts : stmt            {$$ = new SequenceNode(LINE, $1);}   
      | stmts stmt      {$$ = new SequenceNode(LINE, $2, $1);}

stmt : FOREACH IDENTIFIER IN expr stmts             {$$ = new Foreach(LINE, $2, $4, $5);}
     | lval ASSIGN expr SEMICOLON                   {$$ = new AssignNode(LINE, $1, $3);}
     | IF LBRACE expr RBRACE stmts                  {$$ = new IfNode(LINE, $3, $5);}
     | IF LBRACE expr RBRACE stmts ELSE stmts       {$$ = new IfElseNode(LINE, $3, $5, $7);}
     | CLBRACKET stmts CRBRACKET                    {$$ = new BlockNode(LINE, $2);}
     | CLBRACKET CRBRACKET                          {$$ = new BlockNode(LINE, new SequenceNode(LINE));}
     | expr SEMICOLON                               {$$ = $1;};   
     ;

expr : LBRACKET expr VERT IDENTIFIER IN expr COLON expr RBRACKET    {$$ = new RangeExprNode(LINE, $2, $4, $6, $8);}
     | expr DOT IDENTIFIER %prec tCALL                              {$$ = new FunctionCall(LINE, $3, $1);}
     | expr EQUAL expr                                              {$$ = new EqualNode(LINE, $1, $3);}
     | expr NEQUAL expr                                             {$$ = new NotEqualNode(LINE, $1, $3);}
     | expr AND expr                                                {$$ = new AndNode(LINE, $1, $3);}
     | expr OR expr                                                 {$$ = new OrNode(LINE, $1, $3);}
     | expr IN expr                                                 {$$ = new InNode(LINE, $1, $3);}
     | NOT expr %prec tUNARY                                        {$$ = new NotNode(LINE, $2);}
     | IDENTIFIER                                                   {$$ = new RValueNode(LINE, new IdentifierNode(LINE, $1)); delete $1;}
     | literal                                                      {$$ = $1;}
     | IDENTIFIER LBRACE exprs RBRACE %prec tCALL                   {$$ = new FunctionCall(LINE, $1, $3);}
     | IDENTIFIER LBRACE RBRACE %prec tCALL                         {$$ = new FunctionCall(LINE, $1, new SequenceExprNode(LINE));}
     | LBRACE expr RBRACE                                           {$$ = $2;}
     | FALSE                                                        {$$ = new BoolNode(LINE, false);}
     | TRUE                                                         {$$ = new BoolNode(LINE, true);}
     | expr LBRACKET expr RBRACKET %prec tINDEXING                  {$$ = new AtNode(LINE, $1, $3);}
     ;

lval : IDENTIFIER                       { $$ = new IdentifierNode(LINE, $1); delete $1; }
     ;

exprs : expr                              { $$ = new SequenceExprNode(LINE, $1); }
      | exprs COMMA expr                  { $$ = new SequenceExprNode(LINE, $3, $1); }
      ; 

literals : literal                                { $$ = new SequenceLiteralNode(LINE, $1); }
         | literals COMMA literal                 { $$ = new SequenceLiteralNode(LINE, $3, $1); }
         ; 

literal : integer                       { $$ = $1; }
        | double                        { $$ = $1; }
        | string                        { $$ = $1; }
        | list                          { $$ = $1; }
        | NIL                           { $$ = new NilNode(LINE); }
        ;

list : LBRACKET literals RBRACKET       {$$ = new ListNode(LINE, $2);}
     | LBRACKET RBRACKET                {$$ = new ListNode(LINE, new SequenceLiteralNode(LINE));}
     ;

integer : INT                          { $$ = new IntNode(LINE, $1); }
        ;

double : DOUBLE                        { $$ = new FloatNode(LINE, $1); }
       ;

string : STRING                          { $$ = new StringNode(LINE, *$1); delete $1;}
       ;
 /*** END EXAMPLE - Change the example grammar rules above ***/

%% /*** Additional Code ***/

void wasmati::Parser::error(const Parser::location_type& l,
			                const std::string& m) {
    driver.error(l, m);
}
