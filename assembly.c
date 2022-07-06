#include "assembly.h"
#include <stdio.h>

void putins_imp(Ctx *ctx, char * name)
{
    fprintf(output(ctx), "\t%s\n", name);
}

void putins_ind(Ctx *ctx, char *name)
{
    fprintf(output(ctx), "\t%s x, y\n", name);
}

void putins_exval(Ctx *ctx, char *name, Exval exval, int offset)
{
    switch (exval.type)
    {
        case EX_DIRECT:
            if (exval.data_type == EX_EXPRESSION)
                putins_dir(ctx, name, exval.value.expression, offset);
            else
                putins_dir_sec(ctx, name, exval.value.index, offset);
            break;
        case EX_IMMEDIATE:
            if (exval.data_type == EX_EXPRESSION)
                putins_imm(ctx, name, exval.value.expression, offset);
            else
                putins_imm_sec(ctx, name, exval.value.index, offset);
            break;
        case EX_TEMP:
            putins_dir_temp(ctx, name, exval.value.index, offset);
            break;
        case EX_PARAMETER:
            putins_param(ctx, name, exval.value.param.function,
                    exval.value.param.index, offset);
            break;
        case EX_RETURN:
            putins_return(ctx, name, offset);
            break;
        default:
            break;
    }
}

void putins_imm(Ctx *ctx, char *name, Expression *expr, int offset)
{
    fprintf(output(ctx), "\t%s #", name);
    fprint_expression(output(ctx), ctx, expr);
    fprintf(output(ctx), " >> %d\n", offset * 8);
}

void putins_dir_str(Ctx *ctx, char *name, char *value, int offset)
{
    fprintf(output(ctx), "\t%s ", name);
    fprintf(output(ctx), "%s", value);
    fprintf(output(ctx), " + %d\n", offset);
}

void putins_dir_sec(Ctx *ctx, char *name, int index, int offset)
{
    fprintf(output(ctx), "\t%s ", name);
    fprintf(output(ctx), "_lbl_%d", index);
    fprintf(output(ctx), " + %d\n", offset);
}

void putins_imm_sec(Ctx *ctx, char *name, int index, int offset)
{
    fprintf(output(ctx), "\t%s #", name);
    fprintf(output(ctx), "_lbl_%d", index);
    fprintf(output(ctx), " >> %d\n", offset * 8);
}

void putins_dir_symbol(Ctx *ctx, char *name, Symbol *symbol, int offset)
{
    fprintf(output(ctx), "\t%s ", name);
    fprint_symbol(output(ctx), symbol);
    fprintf(output(ctx), " + %d\n", offset);
}

void putins_dir_anon_label(Ctx *ctx, char *name, int label)
{
    char *funct = function_in(ctx);
    fprintf(output(ctx), "\t%s ", name);
    if (funct)
        fprintf(output(ctx), "%s.", funct);
    fprintf(output(ctx), "_lbl_%d\n", label);
}

void putins_dir_anon_label_global(Ctx *ctx, char *name, int label)
{
    fprintf(output(ctx), "\t%s ", name);
    fprintf(output(ctx), "_lbl_%d\n", label);
}

void putins_dir(Ctx *ctx, char *name, Expression *expr, int offset)
{
    fprintf(output(ctx), "\t%s ", name);
    fprint_expression(output(ctx), ctx, expr);
    fprintf(output(ctx), " + %d\n", offset);
}

void putins_dir_temp(Ctx *ctx, char *name, int temp_index, int offset)
{
    fprintf(output(ctx), "\t%s ", name);
    fprintf(output(ctx), "%s.%s_%d", ctx->funct.name, "_tmp", temp_index);
    fprintf(output(ctx), " + %d\n", offset);
}

void putins_imm_int(Ctx *ctx, char *name, int value)
{
    fprintf(output(ctx), "\t%s #", name);
    fprintf(output(ctx), "%d\n", value);
}

void putins_param(Ctx *ctx, char *name, char *function, int index, int offset)
{
    fprintf(output(ctx), "\t%s %s._param_%d + %d\n", name, function, index, offset);

}

void putins_return(Ctx *ctx, char *name, int offset)
{
    fprintf(output(ctx), "\t%s _return + %d\n", name, offset);
}

void loada(Ctx *ctx, Exval exval, int offset)
{
    switch (exval.type)
    {
        case EX_NONE:
            break;
        default:
            putins_exval(ctx, "lda", exval, offset);
            break;
    }
}

void storea(Ctx *ctx, Exval exval, int offset)
{
    switch (exval.type)
    {
        case EX_CONDITION:
            {
                if (!offset)
                {
                    putins_dir_str(ctx, "sta", "_condition", 0);
                }
                else
                {
                    putins_dir_str(ctx, "or", "_condition", 0);
                }
            }
            break;
        case EX_IMMEDIATE:
            fprintf(stderr, "Illegal store.\n");
            break;
        case EX_NONE:
            break;
        default:
            putins_exval(ctx, "sta", exval, offset);
            break;
    }
}

void putlabel(Ctx *ctx, char *name)
{
    fprintf(output(ctx), "%s:\n", name);
}

void putlabeln(Ctx *ctx, int n)
{
    char *funct = function_in(ctx);
    if (funct)
        fprintf(output(ctx), "%s.", funct);
    fprintf(output(ctx), "_lbl_%d:\n", n);
}

void putlabeln_global(Ctx *ctx, int n)
{
    fprintf(output(ctx), "_lbl_%d:\n", n);
}

void putlabel_symbol(Ctx *ctx, Symbol *symbol)
{
    fprint_symbol(output(ctx), symbol);
    fprintf(output(ctx), ":\n");
}

void putreserve(Ctx *ctx, int words)
{
    fprintf(output(ctx), "\t%%res %d\n", words * 2);
}

