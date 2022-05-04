
%code requires
{
    #include "expression.h"
}

%debug
%define parse.error verbose

%{

#include "expression.h"
#include "context.h"
#include "resolve.h"
#include <stdio.h>
#include <string.h>

Ctx g_ctx;
int yylex();
int yyerror(const char *s);

%}

%token IDENTIFIER "identifier" STRING "string" INTEGER "integer"
%token IF "if" ELSE "else" WHILE "while" DO "do" GOTO "goto"
%token LET "let" RETURN "return"
%token EQUAL "==" LESS_EQ "<=" GREATER_EQ ">=" NOT_EQ "!="
%token SHIFT_R ">>" SHIFT_L "<<" AND "&&" OR "||"
%token INC
%token INLINE_ASM

%union
{
    Expression *expr;
    char *str;
    int integer;
}

%type <expr> expression
%type <str> IDENTIFIER STRING INLINE_ASM
%type <integer> INTEGER

%right '='
%left OR
%left AND
%left '|'
%left '^'
%left '&'
%left EQUAL NOT_EQ
%left '>' '<' GREATER_EQ LESS_EQ
%left SHIFT_R SHIFT_L
%left '+' '-'
%left '*' '/' '%'
%right '!' '~'
%left '['

%%

program
    : statement_list
    ;

statement_list
    : statement_list statement
    | statement
    ;

statement
    : expression ';' { fprint_trace(stderr, $1->trace); fputs("; ", stdout); fprint_expression(stdout, $1); putchar('\n'); resolve(&g_ctx, $1, target_discard()); }
    | inline_assembly
    ;

inline_assembly
    : INLINE_ASM { char *s = strndup($1 + 2, strlen($1) - 4); printf("%s\n", s); }
    ;

expression
    : expression '=' expression         { $$ = expression_binary(trace_get(), OPER_ASSIGN, $1, $3); }
    | expression OR expression          { $$ = expression_binary(trace_get(), OPER_LOG_OR, $1, $3); }
    | expression AND expression         { $$ = expression_binary(trace_get(), OPER_LOG_AND, $1, $3); }
    | expression '|' expression         { $$ = expression_binary(trace_get(), OPER_BIT_OR, $1, $3); }
    | expression '^' expression         { $$ = expression_binary(trace_get(), OPER_BIT_XOR, $1, $3); }
    | expression '&' expression         { $$ = expression_binary(trace_get(), OPER_BIT_AND, $1, $3); }
    | expression EQUAL expression       { $$ = expression_binary(trace_get(), OPER_EQUAL, $1, $3); }
    | expression NOT_EQ expression      { $$ = expression_binary(trace_get(), OPER_NOT_EQUAL, $1, $3); }
    | expression '>' expression         { $$ = expression_binary(trace_get(), OPER_GREATER, $1, $3); }
    | expression GREATER_EQ expression  { $$ = expression_binary(trace_get(), OPER_GREATER_EQ, $1, $3); }
    | expression '<' expression         { $$ = expression_binary(trace_get(), OPER_LESS, $1, $3); }
    | expression LESS_EQ expression     { $$ = expression_binary(trace_get(), OPER_LESS_EQ, $1, $3); }
    | expression SHIFT_L expression     { $$ = expression_binary(trace_get(), OPER_SHIFT_LEFT, $1, $3); }
    | expression SHIFT_R expression     { $$ = expression_binary(trace_get(), OPER_SHIFT_RIGHT, $1, $3); }
    | expression '+' expression         { $$ = expression_binary(trace_get(), OPER_ADD, $1, $3); }
    | expression '-' expression         { $$ = expression_binary(trace_get(), OPER_SUBTRACT, $1, $3); }
    | '+' expression %prec '~'      { $$ = $2; }
    | '-' expression %prec '~'      { $$ = expression_unary(trace_get(), OPER_NEGATE, $2); }
    | '!' expression                { $$ = expression_unary(trace_get(), OPER_LOG_NOT, $2); }
    | '~' expression                { $$ = expression_unary(trace_get(), OPER_BIT_NOT, $2); }
    | '*' expression %prec '~'      { $$ = expression_unary(trace_get(), OPER_DEREF, $2); }
    | '&' expression %prec '~'      { $$ = expression_unary(trace_get(), OPER_ADDRESS, $2); }
    | IDENTIFIER '(' ')' %prec '['
    | expression '[' expression ']' { $$ = expression_array(trace_get(), $1, $3);  }
    | INTEGER                       { $$ = expression_literal(trace_get(), $1); }
    | IDENTIFIER                    { $$ = expression_symbolic(trace_get(), $1); }
    | STRING
    | '(' expression ')'            { $$ = $2; }
    ;

%%

int yyerror(const char *s)
{
    printf("error: %s\n", s);
    return 0;
}

