
%code requires
{
    #include "expression.h"
    #include "trace.h"
    #include "token.h"
    #include "external.h"
}

%debug
%define parse.error verbose

%{

#include "expression.h"
#include "context.h"
#include "resolve.h"
#include "token.h"
#include "trace.h"
#include "error.h"
#include "external.h"
#include "stringliteral.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

# define YYLLOC_DEFAULT(Cur, Rhs, N) \
    do         \
        if (N) \
        {      \
            (Cur).first_line   = YYRHSLOC(Rhs, 1).first_line;   \
            (Cur).first_column = YYRHSLOC(Rhs, 1).first_column; \
            (Cur).last_line    = YYRHSLOC(Rhs, N).last_line;    \
            (Cur).last_column  = YYRHSLOC(Rhs, N).last_column;  \
            (Cur).filename = YYRHSLOC(Rhs, 1).filename;         \
        }    \
        else \
        {    \
            (Cur).first_line   = (Cur).last_line   = \
                YYRHSLOC(Rhs, 0).last_line;          \
            (Cur).first_column = (Cur).last_column = \
                YYRHSLOC(Rhs, 0).last_column; \
            (Cur).filename = \
                YYRHSLOC(Rhs, 0).filename;    \
        }                                     \
    while (0)

Token *g_token;
int yylex();
int yyerror(const char *s);

%}

%token IDENTIFIER "identifier" STRING "string" INTEGER "integer" REGISTER EXTERN INCLUDE_ASM
%token IF "if" ELSE "else" WHILE "while" DO "do" GOTO "goto" FOR "for"
%token LET "let" RETURN "return" ARRAY_DEC "array"
%token EQUAL "==" LESS_EQ "<=" GREATER_EQ ">=" NOT_EQ "!="
%token SHIFT_R ">>" SHIFT_L "<<" AND "&&" OR "||"
%token INLINE_ASM "assembly"

%union
{
    Expression *expr;
    char *str;
    char character;
    int integer;
    Token *token;
    StringList *str_list;
    AsmPlace *aplace;
    AsmLocation alocation;
}

%type <expr> expression expression_list expression_list_body paren_expression symbol_expression
%type <str> IDENTIFIER STRING INLINE_ASM strings
%type <integer> INTEGER
%type <token> statement_list statement condition_statement
%type <token> non_condition_statement compound_statement inline_assembly
%type <token> variable_initialize variable_declaration loop_statement function
%type <token> declaration declaration_list asm_include extern_variable extern_function
%type <str_list> string_list string_list_nullable
%type <character> REGISTER
%type <alocation> asm_location
%type <aplace> asm_place asm_place_list


%right '='
%right '?'
%left "||"
%left "&&"
%left '|'
%left '^'
%left '&'
%left "==" "!="
%left '>' '<' ">=" "<="
%left ">>" "<<"
%left '+' '-'
%left '*' '/' '%'
%right '!' '~'
%left '['

%%

program
    : declaration_list  { g_token = $1; }
    | %empty  { g_token = NULL; }
    ;

declaration_list
    : declaration_list declaration  { $$ = cat_token($1, $2); }
    | declaration
    ;

declaration
    : function
    | variable_declaration ';'
    | extern_variable ';'
    | inline_assembly
    | extern_function
    | asm_include
    ;

asm_include
    : INCLUDE_ASM strings ';' { $$ = token_include_asm(@$, $2); free($2); }
    ;

function
    : IDENTIFIER '(' string_list_nullable ')' statement  { $$ = token_function(@$, $1, $3, $5); free($1); }
    ;

extern_function
    : EXTERN IDENTIFIER '(' strings ',' '{' asm_place_list '}' ',' asm_place ')' ';'    { $$ = token_extern_function(@$, $2, $4, $7, $10); }
    | EXTERN IDENTIFIER '(' strings ',' '{' asm_place_list '}' ')' ';'                  { $$ = token_extern_function(@$, $2, $4, $7, NULL); }
    ;

asm_place_list
    : %empty                        { $$ = NULL; }
    | asm_place
    | asm_place_list ',' asm_place  { $$ = place_append_list($1, $3); }
    ;

asm_place
    : asm_location ':' asm_location     { $$ = place_double($1, $3); }
    | strings                           { $$ = place_single($1); }
    ;

asm_location
    : REGISTER      { $$ = location_register($1); }
    | strings       { $$ = location_string($1); }
    | %empty        { $$ = location_none(); }
    ;


statement_list
    : statement_list statement  { $$ = cat_token($1, $2); }
    | statement
    ;

statement
    : non_condition_statement
    | condition_statement
    ;

non_condition_statement
    : expression ';'         { $$ = token_expression(@$, $1); }
    | "goto" IDENTIFIER ';'  { $$ = token_goto(@$, $2); free($2); }
    | IDENTIFIER ':'         { $$ = token_label(@$, $1); free($1); }
    | inline_assembly
    | compound_statement
    | variable_initialize ';'
    | ';'                      { $$ = NULL; }
    | "return" expression ';'  { $$ = token_return(@$, $2); }
    | "return" ';'             { $$ = token_return(@$, NULL); }
    | error ';'                { $$ = NULL; }
    ;

condition_statement
    : "if" paren_expression statement  { $$ = token_condition(@$, $2, $3, NULL); }
    | "if" paren_expression non_condition_statement "else" statement  { $$ = token_condition(@$, $2, $3, $5); }
    | loop_statement
    ;

loop_statement
    : "while" paren_expression statement  { $$ = token_loop(@$, $2, $3, false); }
    | "do" statement "while" paren_expression ';'  { $$ = token_loop(@$, $4, $2, true); }
    | "for" '(' statement expression ';' expression ')' statement { $$ = token_for(@$, $3, $4, $6, $8); }
    ;

variable_initialize
    : variable_declaration
    | variable_declaration '=' expression { $$->expr = $3; }
    ;

variable_declaration
    : "let" string_list  { $$ = token_variable(@$, $2, NULL, false); }
    | "let" string_list '[' expression ']' { token_variable(@$, $2, $4, false); }
    ;

extern_variable
    : EXTERN variable_declaration { $2->is_extern = true; $$ = $2; }
    ;

string_list_nullable
    : string_list
    | %empty  { $$ = NULL; }
    ;

string_list
    : string_list ',' IDENTIFIER  { cat_stringlist($1, $3); $$ = $1; free($3); }
    | IDENTIFIER  { $$ = new_stringlist($1); free($1); }
    ;

compound_statement
    : '{' statement_list '}'  { $$ = token_compound(@$, $2); }
    | '{' '}'  { $$ = token_compound(@$, NULL); }
    | '{' error '}'  { $$ = token_compound(@$, NULL); }
    ;

inline_assembly
    : INLINE_ASM  { $$ = token_assembly(@$, strndup($1 + 2, strlen($1) - 4)); free($1); }
    ;

expression
    : expression '=' expression   { $$ = expression_binary(@1, OPER_ASSIGN, $1, $3); }
    | expression "||" expression  { $$ = expression_binary(@1, OPER_LOG_OR, $1, $3); }
    | expression "&&" expression  { $$ = expression_binary(@1, OPER_LOG_AND, $1, $3); }
    | expression '|' expression   { $$ = expression_binary(@1, OPER_BIT_OR, $1, $3); }
    | expression '^' expression   { $$ = expression_binary(@1, OPER_BIT_XOR, $1, $3); }
    | expression '&' expression   { $$ = expression_binary(@1, OPER_BIT_AND, $1, $3); }
    | expression "==" expression  { $$ = expression_binary(@1, OPER_EQUAL, $1, $3); }
    | expression "!=" expression  { $$ = expression_binary(@1, OPER_NOT_EQUAL, $1, $3); }
    | expression '>' expression   { $$ = expression_binary(@1, OPER_GREATER, $1, $3); }
    | expression ">=" expression  { $$ = expression_binary(@1, OPER_GREATER_EQ, $1, $3); }
    | expression '<' expression   { $$ = expression_binary(@1, OPER_LESS, $1, $3); }
    | expression "<=" expression  { $$ = expression_binary(@1, OPER_LESS_EQ, $1, $3); }
    | expression "<<" expression  { $$ = expression_binary(@1, OPER_SHIFT_LEFT, $1, $3); }
    | expression ">>" expression  { $$ = expression_binary(@1, OPER_SHIFT_RIGHT, $1, $3); }
    | expression '+' expression   { $$ = expression_binary(@1, OPER_ADD, $1, $3); }
    | expression '-' expression   { $$ = expression_binary(@1, OPER_SUBTRACT, $1, $3); }
    | expression '*' expression   { $$ = expression_binary(@1, OPER_MULTIPLY, $1, $3); }
    | expression '/' expression   { $$ = expression_binary(@1, OPER_DIVIDE, $1, $3); }
    | expression '%' expression   { $$ = expression_binary(@1, OPER_MODULO, $1, $3); }
    | '+' expression %prec '~'    { $$ = $2; }
    | '-' expression %prec '~'    { $$ = expression_unary(@1, OPER_NEGATE, $2); }
    | '!' expression              { $$ = expression_unary(@1, OPER_LOG_NOT, $2); }
    | '~' expression              { $$ = expression_unary(@1, OPER_BIT_NOT, $2); }
    | '*' expression %prec '~'    { $$ = expression_unary(@1, OPER_DEREF, $2); }
    | '&' expression %prec '~'    { $$ = expression_unary(@1, OPER_ADDRESS, $2); }
    | IDENTIFIER '(' expression_list ')' %prec '['  { $$ = expression_function(@$, $1, $3); free($1); }
    | expression '[' expression ']' { $$ = expression_array(@$, $1, $3); }
    | INTEGER                       { $$ = expression_literal(@1, $1); }
    | symbol_expression
    | strings                       { $$ = expression_string(@1, $1); }
    | ARRAY_DEC '[' expression ']'        { $$ = expression_alloc_array(@$, $3); }
    | ARRAY_DEC '(' expression_list ')'   { $$ = expression_literal_array(@$, $3); }
    | paren_expression
    | expression '+' '=' expression %prec '='  { $$ = expression_assign_by(@$, OPER_ADD, $1, $4); }
    | expression '-' '=' expression %prec '='  { $$ = expression_assign_by(@$, OPER_SUBTRACT, $1, $4); }
    | expression '*' '=' expression %prec '='  { $$ = expression_assign_by(@$, OPER_MULTIPLY, $1, $4); }
    | expression '/' '=' expression %prec '='  { $$ = expression_assign_by(@$, OPER_DIVIDE, $1, $4); }
    | expression '|' '=' expression %prec '='  { $$ = expression_assign_by(@$, OPER_BIT_OR, $1, $4); }
    | expression '&' '=' expression %prec '='  { $$ = expression_assign_by(@$, OPER_BIT_AND, $1, $4); }
    | expression '?' expression ':' expression %prec '?' { $$ = expression_ternary(@$, $1, $3, $5); }
    ;

symbol_expression
    : IDENTIFIER  { $$ = expression_symbolic(@1, $1); free($1); }
    ;

paren_expression
    : '(' expression ')'  { $$ = $2; }
    | '(' error ')'       { $$ = expression_literal(@$, 1); }
    ;

strings
    : STRING          { $$ = $1; }
    | strings STRING  { $$ = combine_string($1, $2); free($2); }
    ;

expression_list
    : expression_list_body
    | %empty               { $$ = NULL; }
    ;

expression_list_body
    : expression ',' expression_list  { $1->next = $3; $$ = $1; }
    | expression
    ;

%%

int yyerror(const char *s)
{
    error(&yylloc, "%s.", s);
    return 0;
}

