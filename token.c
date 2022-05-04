#include "token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Token *new_token(Trace trace, TokenType type, char *name, Expression *expr, Token *statements, Token *statements_other)
{
    Token *token = malloc(sizeof(Token));
    token->trace = trace;
    token->type = type;
    if (name)
        token->name = strdup(name);
    else
        token->name = NULL;
    token->expr = expr;
    token->statements = statements;
    token->statements_other = statements_other;
    return token;
}

void free_token(Token *token)
{
    if (token->name)
        free(token->name);
    if (token->expr)
        free_expression(token->expr);
    if (token->statements)
        free_token(token->statements);
    if (token->statements_other)
        free_token(token->statements_other);
    if (token->next)
        free_token(token->next);
    free(token);
}

Token *token_expression(Trace trace, Expression *expr)
{
    return new_token(trace, TOK_EXPR, NULL, expr, NULL, NULL);
}

Token *token_variable(Trace trace, char *name, Expression *expr)
{
    return new_token(trace, TOK_VARIABLE, name, expr, NULL, NULL);
}

Token *token_conditon(Trace trace, Expression *expr, Token *tokens, Token *tokens_else)
{
    return new_token(trace, TOK_CONDITION, NULL, expr, tokens, tokens_else);
}

Token *token_loop(Trace trace, Expression *expr, Token *tokens, bool reverse_order)
{
    return new_token(trace, TOK_LOOP, NULL, expr,
            reverse_order ? NULL : tokens, reverse_order ? tokens : NULL);
}

Token *token_goto(Trace trace, char *name)
{
    return new_token(trace, TOK_GOTO, name, NULL, NULL, NULL);
}

Token *token_return(Trace trace, Expression *expr)
{
    return new_token(trace, TOK_RETURN, NULL, expr, NULL, NULL);
}

