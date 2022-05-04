#ifndef TOKEN_H
#define TOKEN_H

#include "expression.h"
#include "trace.h"
#include <stdbool.h>

typedef enum TokenType
{
    TOK_EXPR,
    TOK_VARIABLE,
    TOK_FUNCTION,
    TOK_CONDITION,
    TOK_LOOP,
    TOK_GOTO,
    TOK_RETURN,
} TokenType;

typedef struct Token
{
    Trace trace;
    TokenType type;
    char *name;
    Expression *expr;
    struct Token *statements;
    struct Token *statements_other;
    struct Token *next;
} Token;

void free_token(Token *token);
Token *new_token(Trace trace, TokenType type, char *name, Expression *expr, Token *statements, Token *statements_other);

Token *token_expression(Trace trace, Expression *expr);
Token *token_variable(Trace trace, char *name, Expression *expr);
Token *token_conditon(Trace trace, Expression *expr, Token *tokens, Token *tokens_else);
Token *token_loop(Trace trace, Expression *expr, Token *tokens, bool reverse_order);
Token *token_goto(Trace trace, char *name);
Token *token_return(Trace trace, Expression *expr);

#endif

