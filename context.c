#include "context.h"
#include <stdlib.h>

Ctx create_context(FILE *output)
{
    Ctx ctx;
    ctx.label_count = 0;
    ctx.output = output;
    ctx.globals = NULL;
    ctx.depth = 0;
    return ctx;
}

int anon_label(Ctx *ctx)
{
    return ctx->label_count++;
}

