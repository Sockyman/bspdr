#ifndef TOKEN_H
#define TOKEN_H

#include "expression.h"
#include "trace.h"
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
    TOK_ASSEMBLY,
    TOK_COMPOUND,
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
} Token;

void free_token(Token *token);
Token *new_token(Trace trace, TokenType type, char *name, Expression *expr,
        Token *statements, Token *statements_other, StringList *ids);

Token *token_compound(Trace trace, Token *statements);
Token *token_expression(Trace trace, Expression *expr);
Token *token_variable(Trace trace, StringList *ids, Expression *length);
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

Token *cat_token(Token *token_1, Token *token_2);

StringList *new_stringlist(char *str);
StringList *cat_stringlist(StringList *list, char *str);
void free_stringlist(StringList *list);

#endif

