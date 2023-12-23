#ifndef TOKEN_H
#define TOKEN_H

#include "expression.h"
#include "trace.h"
#include "external.h"
#include <stdbool.h>

typedef struct StringList
{
    char *str;
    struct StringList *next;
} StringList;

typedef enum TokenType
{
    TOK_EXPR,
    TOK_VARIABLE,
    TOK_FUNCTION,
    TOK_CONDITION,
    TOK_LOOP,
    TOK_FOR,
    TOK_GOTO,
    TOK_LABEL,
    TOK_RETURN,
    TOK_SWITCH,
    TOK_CASE,
    TOK_ASSEMBLY,
    TOK_COMPOUND,
    TOK_INCLUDE_ASM,
    TOK_EXTERN_FUNCTION,
} TokenType;

typedef struct Token
{
    Trace trace;
    TokenType type;
    char *name;
    Expression *expr;
    Expression *expr_other;
    StringList *ids;
    struct Token *statements;
    struct Token *statements_other;
    struct Token *next;
    bool is_extern;
    bool continue_after;
    char *extern_name;
    AsmPlace *asm_params;
    AsmPlace *asm_return;

} Token;

void free_token(Token *token);
Token *new_token(Trace trace, TokenType type, char *name, Expression *expr,
        Token *statements, Token *statements_other, StringList *ids);

Token *token_compound(Trace trace, Token *statements);
Token *token_expression(Trace trace, Expression *expr);
Token *token_variable(Trace trace, StringList *ids, Expression *length, bool is_extern);
Token *token_condition(Trace trace, Expression *expr,
        Token *tokens, Token *tokens_else);
Token *token_loop(Trace trace, Expression *expr,
        Token *tokens, bool reverse_order);
Token *token_for(Trace trace, Token *init_statement, Expression *expr_check, Expression *expr_end, Token *loop_statement);
Token *token_goto(Trace trace, char *name);
Token *token_label(Trace trace, char *name);
Token *token_return(Trace trace, Expression *expr);
Token *token_assembly(Trace trace, char *text);
Token *token_function(Trace trace, char *name, StringList *parameters, Token *statements);

Token *token_extern_function(Trace trace, char *name, char *extern_name, AsmPlace *params, AsmPlace *return_place);

Token *token_include_asm(Trace trace, char *file_name);

Token *token_switch(Trace trace, Expression *expr, Token *statements);
Token *token_case(Trace trace, Expression *expr, Token *statements, bool continue_after);

Token *cat_token(Token *token_1, Token *token_2);

StringList *new_stringlist(char *str);
StringList *cat_stringlist(StringList *list, char *str);
void free_stringlist(StringList *list);

#endif

