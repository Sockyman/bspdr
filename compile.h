#ifndef COMPILE_H
#define COMPILE_H

#include "token.h"
#include "trace.h"
#include "context.h"
#include "expression.h"
#include "exprvalue.h"

void declare_variable(Ctx *ctx, Trace trace, char *name, bool is_extern);
void compile_expression(Ctx *ctx, Expression *expr, ExTarget target);
void compile_program(Ctx *ctx, Token *token);
void compile(Ctx *ctx, Token *token);
void compile_compound(Ctx *ctx, Token *token);
void compile_expression_token(Ctx *ctx, Token *token);
void compile_variable(Ctx *ctx, Token *token);
void compile_condition(Ctx *ctx, Token *token);
void compile_loop(Ctx *ctx, Token *token);
void compile_for(Ctx *ctx, Token *token);
void compile_goto(Ctx *ctx, Token *token);
void compile_label(Ctx *ctx, Token *token);
void compile_return(Ctx *ctx, Token *token);
void compile_switch(Ctx *ctx, Token *token);
void compile_assembly(Ctx *ctx, Token *token);
void compile_function(Ctx *ctx, Token *token);
void compile_extern_function(Ctx *ctx, Token *token);
void compile_include_asm(Ctx *ctx, Token *token);

#endif

