#include "compile.h"
#include "expression.h"
#include "resolve.h"
#include "error.h"
#include "exprvalue.h"
#include "assembly.h"
#include <stdio.h>

void compile_program(Ctx *ctx, Token *token)
{
    set_output(ctx, OUT_DATA);
    fprintf(output(ctx), "section data\n");
    putlabel(ctx, "_condition");
    putreserve(ctx, 1);
    putlabel(ctx, "_return");
    putreserve(ctx, 1);
    set_output(ctx, OUT_TEXT);

    putlabel(ctx, "_reset");
    fprintf(output(ctx), "\tcall ");
    putfunctionname(output(ctx), "main");
    fprintf(output(ctx), "\n");
    putins_imp(ctx, "hlt");

    declare_symbol(ctx, FUNCTION, "main", 0, GLOBAL_SCOPE, false);

    compile(ctx, token);

    Symbol *current = ctx->globals;
    while (current)
    {
        if (current->type == FUNCTION && !current->is_defined)
        {
            error(NULL, "function \'%s\' is not defined.", current->name);
        }
        current = current->next;
    }
}

void compile(Ctx *ctx, Token *token)
{
    Token *t = token;
    while (t)
    {
        switch (t->type)
        {
            case TOK_COMPOUND:
                compile_compound(ctx, t);
                break;
            case TOK_EXPR:
                compile_expression_token(ctx, t);
                break;
            case TOK_VARIABLE:
                compile_variable(ctx, t);
                break;
            case TOK_CONDITION:
                compile_condition(ctx, t);
                break;
            case TOK_LOOP:
                compile_loop(ctx, t);
                break;
            case TOK_FOR:
                compile_for(ctx, t);
                break;
            case TOK_GOTO:
                compile_goto(ctx, t);
                break;
            case TOK_LABEL:
                compile_label(ctx, t);
                break;
            case TOK_RETURN:
                compile_return(ctx, t);
                break;
            case TOK_ASSEMBLY:
                compile_assembly(ctx, t);
                break;
            case TOK_FUNCTION:
                compile_function(ctx, t);
                break;
            case TOK_INCLUDE_ASM:
                compile_include_asm(ctx, t);
                break;
            case TOK_EXTERN_FUNCTION:
                compile_extern_function(ctx, t);
                break;
            case TOK_SWITCH:
                compile_switch(ctx, t);
                break;
            case TOK_CASE:
                fprintf(stderr, "Token case only valid within token switch.\n");
                break;
        }
        t = t->next;
    }
}

void compile_include_asm(Ctx *ctx, Token *token)
{
    fprintf(output(ctx), "include_once \"%s\"\n", token->name);
}

void begin_function(Ctx *ctx, Token *token, int pcount)
{
    Symbol *symbol = resolve_symbol(ctx, token->name);
    if (symbol && symbol->is_defined)
    {
        error(&token->trace, "\'%s\' is already defined.", token->name);
    }
    else if (symbol && symbol->parameters != pcount)
    {
        error(&token->trace, "\'%s\' used with different parameters.", token->name);
    }
    else if (symbol)
    {
        symbol->is_defined = true;
    }
    else
    {
        symbol = declare_symbol(ctx, FUNCTION, token->name,
                pcount, GLOBAL_SCOPE, true);
    }

    putlabel_function(ctx, token->name);
}

void compile_function(Ctx *ctx, Token *token)
{
    enter_function(ctx, token->name);

    int pcount = 0;
    StringList *param = token->ids;
    while (param)
    {
        set_output(ctx, OUT_DATA);
        putfunctionname(output(ctx), token->name);
        fprintf(output(ctx), "._param_%d:\n", pcount++);
        set_output(ctx, OUT_TEXT);
        declare_variable(ctx, token->trace, param->str, false);
        param = param->next;
    }
    begin_function(ctx, token, pcount);

    compile(ctx, token->statements);
    putins_imp(ctx, "ret");

    set_output(ctx, OUT_DATA);
    for (int i = 0; i < ctx->funct.max_temps; ++i)
    {
        putfunctionname(output(ctx), token->name);
        fprintf(output(ctx), "._tmp_%d:\n", i);
        putreserve(ctx, 1);
    }
    set_output(ctx, OUT_TEXT);

    Symbol *lbl_check = ctx->funct.symbols;
    while (lbl_check)
    {
        if (lbl_check->type == LABEL && !lbl_check->is_defined)
        {
            error(&token->trace, "label \'%s\' is not defined in \'%s\'.",
                    lbl_check->name, token->name);
        }
        lbl_check = lbl_check->next;
    }

    leave_scope(ctx);
}


void compile_extern_function(Ctx *ctx, Token *token)
{
    enter_function(ctx, token->name);

    int pcount = 0;
    AsmPlace *param_place = token->asm_params;
    while (param_place)
    {
        set_output(ctx, OUT_DATA);
        putfunctionname(output(ctx), token->name);
        fprintf(output(ctx), "._param_%d", pcount++);

        if (param_place->one_location)
        {
            fprintf(output(ctx), " = %s\n", param_place->locations[0].str);
        }
        else
        {
            fprintf(output(ctx), ":\n");
            putreserve(ctx, 1);
        }

        set_output(ctx, OUT_TEXT);
        param_place = param_place->next;
    }
    begin_function(ctx, token, pcount);

    int p_number = 0;
    AsmPlace *param = token->asm_params;
    while (param)
    {
        if (param->one_location)
        {
            putfunctionname(output(ctx), token->name);
            fprintf(output(ctx), "._param_%d = %s\n", p_number, param->locations[0].str);
        }
        else
        {
            for (int i = 0; i < 2; ++i)
            {
                AsmLocation *loc = &param->locations[i];
                if (loc->type == LOCATION_STR)
                {
                    fprintf(output(ctx), "\tlda ");
                    putfunctionname(output(ctx), token->name);
                    fprintf(output(ctx), "._param_%d + %d\n", p_number, i);
                    fprintf(output(ctx), "\tsta %s\n", loc->str);
                }
            }
        }
        
        param = param->next;
        ++p_number;
    }

    p_number = 0;
    param = token->asm_params;
    while (param)
    {
        for (int i = 0; i < 2; ++i)
        {
            AsmLocation *loc = &param->locations[i];
            if (loc->type == LOCATION_REG)
            {
                fprintf(output(ctx), "\tld%c ", loc->reg);
                putfunctionname(output(ctx), token->name);
                fprintf(output(ctx), "._param_%d + %d\n", p_number, i);
            }
        }
        
        param = param->next;
        ++p_number;
    }

    fprintf(output(ctx), "\tcall %s\n", token->extern_name);

    if (token->asm_return)
    {
        if (token->asm_return->one_location)
        {
            for (int i = 0; i < 2; ++i)
            {
                putins_dir_str(ctx, "lda", token->asm_return->locations[0].str, i);
                putins_dir_str(ctx, "sta", "_return", i);
            }
        }
        else
        {
            for (int i = 0; i < 2; ++i)
            {
                AsmLocation *loc = &token->asm_return->locations[i];
                if (loc->type == LOCATION_REG)
                {
                    fprintf(output(ctx), "\tst%c _return + %d\n", loc->reg, i);
                }
            }
            for (int i = 0; i < 2; ++i)
            {
                AsmLocation *loc = &token->asm_return->locations[i];
                if (loc->type == LOCATION_STR)
                {
                    putins_dir_str(ctx, "lda", loc->str, 0);
                    putins_dir_str(ctx, "sta", "_return", i);
                }
                else if (loc->type == LOCATION_NONE)
                {
                    putins_imm_int(ctx, "lda", 0);
                    putins_dir_str(ctx, "sta", "_return", i);
                }
            }
        }
    }

    putins_imp(ctx, "ret");
    leave_scope(ctx);
}


void compile_compound(Ctx *ctx, Token *token)
{
    enter_scope(ctx);
    if (token->statements)
    {
        compile(ctx, token->statements);
    }
    leave_scope(ctx);
}

void compile_expression(Ctx *ctx, Expression *expr, ExTarget target)
{
    ctx->funct.temps = 0;
    resolve(ctx, expr, target);
    ctx->funct.max_temps = (ctx->funct.temps > ctx->funct.max_temps)
        ? ctx->funct.temps : ctx->funct.max_temps;
}

void compile_expression_token(Ctx *ctx, Token *token)
{
    compile_expression(ctx, token->expr, target_discard());
}

void declare_variable(Ctx *ctx, Trace trace, char *name, bool is_extern)
{
    Symbol *symbol = resolve_symbol(ctx, name);
    if (symbol)
    {
        if (symbol->is_defined)
        {
            error(&trace,
                    "redeclaration of \'%s\'.", name);
        }
        else
        {
            redeclare_symbol(ctx, symbol);
        }
    }
    else
    {
        symbol = declare_symbol(ctx, VARIABLE, name, 0, CURRENT_SCOPE, true);
        if (!is_extern)
        {
            set_output(ctx, OUT_DATA);
            putlabel_symbol(ctx, symbol);
            putreserve(ctx, 1);
            set_output(ctx, OUT_TEXT);
        }
    }
}

void compile_variable(Ctx *ctx, Token *token)
{
    StringList *current = token->ids;
    while (current)
    {
        declare_variable(ctx, token->trace, current->str, token->is_extern);
        current = current->next;
    }
    if (token->expr)
    {
        current = token->ids;
        while (current)
        {
            Expression sym = { token->trace, SYMBOLIC, {current->str}, NULL };
            Expression assign_symbol = { token->trace, BINARY_OPERATION };
            assign_symbol.value.operative.operation = OPER_ASSIGN;
            assign_symbol.value.operative.x = &sym;
            assign_symbol.value.operative.y = token->expr;
            compile_expression(ctx, &assign_symbol, target_discard());
            current = current->next;
        }
    }
}

void compile_condition(Ctx *ctx, Token *token)
{
    int end_lbl = anon_label(ctx);
    int else_lbl = anon_label(ctx);
    compile_expression(ctx, token->expr, target_condition());
    putins_dir_anon_label(ctx, "jz", token->statements_other ? else_lbl : end_lbl);
    compile(ctx, token->statements);
    if (token->statements_other)
    {
        putins_dir_anon_label(ctx, "jmp", end_lbl);
        putlabeln(ctx, else_lbl);
        compile(ctx, token->statements_other);
    }
    putlabeln(ctx, end_lbl);
}

void compile_loop(Ctx *ctx, Token *token)
{
    enter_scope(ctx);
    int loop_label = anon_label(ctx);
    putlabeln(ctx, loop_label);

    if (token->statements)
    {
        int end_label = anon_label(ctx);
        compile_expression(ctx, token->expr, target_condition());
        putins_dir_anon_label(ctx, "jz", end_label);
        compile(ctx, token->statements);
        putins_dir_anon_label(ctx, "jmp", loop_label);
        putlabeln(ctx, end_label);
    }
    else
    {
        compile(ctx, token->statements_other);
        compile_expression(ctx, token->expr, target_condition());
        putins_dir_anon_label(ctx, "jnz", loop_label);
    }
    leave_scope(ctx);
}

void compile_for(Ctx *ctx, Token *token)
{
    enter_scope(ctx);

    int end_label = anon_label(ctx);
    int loop_label = anon_label(ctx);
    // for (int i = 0; i < 100; i += 1) foo();
    compile(ctx, token->statements);
    putlabeln(ctx, loop_label);
    compile_expression(ctx, token->expr, target_condition());
    putins_dir_anon_label(ctx, "jz", end_label);
    compile(ctx, token->statements_other);
    compile_expression(ctx, token->expr_other, target_discard());
    putins_dir_anon_label(ctx, "jmp", loop_label);
    putlabeln(ctx, end_label);

    leave_scope(ctx);
}

void compile_goto(Ctx *ctx, Token *token)
{
    Symbol *symbol = resolve_symbol(ctx, token->name);
    if (!symbol)
    {
        symbol = declare_symbol(ctx, LABEL, token->name, 0, FUNCTION_SCOPE, false);
    }
    putins_dir_symbol(ctx, "jmp", symbol, 0);
}

void compile_label(Ctx *ctx, Token *token)
{
    Symbol *symbol = resolve_symbol(ctx, token->name);
    if (symbol && symbol->type != LABEL)
    {
        error(&token->trace, "symbol \'%s\' already exists.", token->name);
    }
    else if (symbol && symbol->type == LABEL)
    {
        if (symbol->is_defined)
        {
            error(&token->trace, "redefinition of label \'%s\'.", token->name);
        }
        else
        {
            symbol->is_defined = true;
        }
    }
    else
    {
        symbol = declare_symbol(ctx, LABEL, token->name, 0, FUNCTION_SCOPE, true);
    }
    putlabel_symbol(ctx, symbol);
}

void compile_return(Ctx *ctx, Token *token)
{
    if (token->expr)
        compile_expression(ctx, token->expr, target_exval(exval_return()));

    putins_imp(ctx, "ret");
}

void compile_switch(Ctx *ctx, Token *token)
{
    int end_lbl = anon_label(ctx);
    int default_lbl = anon_label(ctx);
    int label_i0 = default_lbl + 1;

    Exval goal = resolve(ctx, token->expr, target_any());
    putins_exval(ctx, "ldx", goal, 0);
    putins_exval(ctx, "ldy", goal, 1);

    Token *current = token->statements;
    while (current && current->type == TOK_CASE)
    {
        int comp_lbl = anon_label(ctx);
        int skip_lbl = anon_label(ctx);

        Exval comparison = resolve(ctx, current->expr, target_any());
        if (comparison.type != EX_IMMEDIATE)
        {
            error(&current->trace, "case is not assemble time constant.");
            current = current->next;
            continue;
        }

        putins_imp(ctx, "txa");
        putins_exval(ctx, "cmp", comparison, 0);
        putins_dir_anon_label(ctx, "jnz", skip_lbl);
        putins_imp(ctx, "tya");
        putins_exval(ctx, "cmpc", comparison, 1);
        putins_dir_anon_label(ctx, "jz", comp_lbl);
        putlabeln(ctx, skip_lbl);

        current = current->next;
    }
    putins_dir_anon_label(ctx, "jmp", default_lbl);

    current = token->statements;
    int case_count = 0;
    while (current && current->type == TOK_CASE)
    {
        int comp_lbl = case_count++ * 2 + label_i0;
        putlabeln(ctx, comp_lbl);
        compile(ctx, current->statements);
        if (!current->continue_after)
        {
            putins_dir_anon_label(ctx, "jmp", end_lbl);
        }
        current = current->next;
    }

    putlabeln(ctx, default_lbl);
    if (current)
    {
        compile(ctx, current->statements);
    }
    putlabeln(ctx, end_lbl);
}

void compile_assembly(Ctx *ctx, Token *token)
{
    fprintf(output(ctx), "; (inline asm) %s:%d\n", token->trace.filename, token->trace.first_line);
    fprintf(output(ctx), "%s\n", token->name);
    fprintf(output(ctx), "; (end inline asm) %s:%d\n", token->trace.filename, token->trace.last_line);
}
