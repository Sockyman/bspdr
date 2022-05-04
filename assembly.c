#include "assembly.h"
#include <stdio.h>

void putins_imp(Ctx *ctx, char * name)
{
    fprintf(ctx->output, "\t%s\n", name);
}

void putins_ind(Ctx *ctx, char *name)
{
    fprintf(ctx->output, "\t%s x, y\n", name);
}

void putins_exval(Ctx *ctx, char *name, Exval exval, int offset)
{
    switch (exval.type)
    {
        case EX_DIRECT:
            putins_dir(ctx, name, exval.value.expression, offset);
            break;
        case EX_IMMEDIATE:
            putins_imm(ctx, name, exval.value.expression, offset);
            break;
        case EX_TEMP:
            putins_dir_temp(ctx, name, exval.value.index, offset);
            break;
        default:
            break;
    }
}

void putins_imm(Ctx *ctx, char *name, Expression *expr, int offset)
{
    fprintf(ctx->output, "\t%s #", name);
    fprint_expression(stdout, expr);
    fprintf(ctx->output, " >> %d\n", offset * 8);
}

void putins_dir_str(Ctx *ctx, char *name, char *value, int offset)
{
    fprintf(ctx->output, "\t%s ", name);
    fprintf(ctx->output, "%s", value);
    fprintf(ctx->output, " + %d\n", offset);

}

void putins_dir_anon_label(Ctx *ctx, char *name, int label)
{
    fprintf(ctx->output, "\t%s _l_%d\n", name, label);
}

void putins_dir(Ctx *ctx, char *name, Expression *expr, int offset)
{
    fprintf(ctx->output, "\t%s ", name);
    fprint_expression(stdout, expr);
    fprintf(ctx->output, " + %d\n", offset);
}

void putins_dir_temp(Ctx *ctx, char *name, int temp_index, int offset)
{
    fprintf(ctx->output, "\t%s ", name);
    fprintf(ctx->output, "%s_%d", "_t", temp_index);
    fprintf(ctx->output, " + %d\n", offset);
}

void putins_imm_int(Ctx *ctx, char *name, int value)
{
    fprintf(ctx->output, "\t%s #", name);
    fprintf(ctx->output, "%d\n", value);
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
                    putins_dir_str(ctx, "ora", "_condition", 0);
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
    fprintf(ctx->output, "%s:\n", name);
}

void putlabeln(Ctx *ctx, int n)
{
    fprintf(ctx->output, "_l_%d:\n", n);
}

