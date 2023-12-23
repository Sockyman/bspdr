#include "token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Token *new_token(Trace trace, TokenType type, char *name, Expression *expr,
    Token *statements, Token *statements_other, StringList *ids)
{
    Token *token = malloc(sizeof(Token)); token->trace = trace;
    token->type = type;
    if (name)
        token->name = strdup(name);
    else
        token->name = NULL;
    token->expr = expr;
    token->expr_other = NULL;
    token->statements = statements;
    token->statements_other = statements_other;
    token->ids = ids;
    token->is_extern = false;
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
    if (token->ids)
        free_stringlist(token->ids);
    free(token);
}

Token *cat_token(Token *token_1, Token *token_2)
{
    if (!token_1 && !token_2)
        return NULL;
    else if (!token_1)
        return token_2;
    if (token_2)
    {
        Token *current = token_1;
        while (current->next)
            current = current->next;
        current->next = token_2;
    }
    return token_1;
}

Token *token_compound(Trace trace, Token *statements)
{
    return new_token(trace, TOK_COMPOUND, NULL, NULL, statements, NULL, NULL);
}

Token *token_expression(Trace trace, Expression *expr)
{
    return new_token(trace, TOK_EXPR, NULL, expr, NULL, NULL, NULL);
}

Token *token_variable(Trace trace, StringList *ids, Expression *length, bool is_extern)
{
    Token *token = new_token(trace, TOK_VARIABLE, NULL, NULL, NULL, NULL, ids);
    token->expr_other = length;
    token->is_extern = is_extern;
    return token;
}

Token *token_condition(Trace trace, Expression *expr, Token *tokens, Token *tokens_else)
{
    return new_token(trace, TOK_CONDITION, NULL, expr, tokens, tokens_else, NULL);
}

Token *token_loop(Trace trace, Expression *expr, Token *tokens, bool reverse_order)
{
    return new_token(trace, TOK_LOOP, NULL, expr,
        reverse_order ? NULL : tokens, reverse_order ? tokens : NULL, NULL);
}

Token *token_for(Trace trace, Token *init_statement, Expression *expr_check, Expression *expr_end, Token *loop_statement)
{
    Token *token = new_token(trace, TOK_FOR, NULL, expr_check, init_statement, loop_statement, NULL);
    token->expr_other = expr_end;
    return token;

}

Token *token_goto(Trace trace, char *name)
{
    return new_token(trace, TOK_GOTO, name, NULL, NULL, NULL, NULL);
}

Token *token_label(Trace trace, char *name)
{
    return new_token(trace, TOK_LABEL, name, NULL, NULL, NULL, NULL);
}

Token *token_return(Trace trace, Expression *expr)
{
    return new_token(trace, TOK_RETURN, NULL, expr, NULL, NULL, NULL);
}

Token *token_assembly(Trace trace, char *text)
{
    return new_token(trace, TOK_ASSEMBLY, text, NULL, NULL, NULL, NULL);
}

Token *token_function(Trace trace, char *name, StringList *parameters, Token *statements)
{
    return new_token(trace, TOK_FUNCTION, name, NULL, statements, NULL, parameters);
}

Token *token_extern_function(Trace trace, char *name, char *extern_name, AsmPlace *params, AsmPlace *return_place)
{
    Token *token = new_token(trace, TOK_EXTERN_FUNCTION, name, NULL, NULL, NULL, NULL);
    token->extern_name = extern_name;
    token->asm_params = params;
    token->asm_return = return_place;
    return token;
}

Token *token_include_asm(Trace trace, char *file_name)
{
    return new_token(trace, TOK_INCLUDE_ASM, file_name, NULL, NULL, NULL, NULL);
}

Token *token_switch(Trace trace, Expression *expr, Token *statements)
{
    return new_token(trace, TOK_SWITCH, NULL, expr, statements, NULL, NULL);
}

Token *token_case(Trace trace, Expression *expr, Token *statements, bool continue_after)
{
    Token *token = new_token(trace, TOK_CASE, NULL, expr, statements, NULL, NULL);
    token->continue_after = continue_after;
    return token;
}

StringList *new_stringlist(char *str)
{
    StringList *list = malloc(sizeof(StringList));
    list->str = strdup(str);
    list->next = NULL;
    return list;
}

StringList *cat_stringlist(StringList *list, char *str)
{
    StringList *to_append = new_stringlist(str);
    StringList *current = list;
    while (current->next)
    {
        current = current->next;
    }
    current->next = to_append;
    return list;
}

void free_stringlist(StringList *list)
{
    while(list)
    {
        free(list->str);
        StringList *next = list->next;
        free(list);
        list = next;
    }
}

