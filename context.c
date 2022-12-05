#include "context.h"
#include "assembly.h"
#include "stringliteral.h"
#include <string.h>
#include <stdlib.h>

Ctx create_context(FILE *output_text, FILE *output_data, FILE *output_const)
{
    Ctx ctx;
    ctx.label_count = 0;
    ctx.file = OUT_TEXT;
    ctx.output[OUT_TEXT] = output_text;
    ctx.output[OUT_DATA] = output_data;
    ctx.output[OUT_CONST] = output_const;
    ctx.globals = NULL;
    ctx.sections = NULL;
    ctx.depth = 0;
    memset(ctx.complex_operators, 0, sizeof(bool) * 3);
    return ctx;
}

FILE *output(Ctx *ctx)
{
    return ctx->output[ctx->file];
}

void set_output(Ctx *ctx, OutputFile file)
{
    ctx->file = file;
}

int anon_label(Ctx *ctx)
{
    return ctx->label_count++;
}

void enter_scope(Ctx *ctx)
{
    ++ctx->depth;
}

void leave_scope(Ctx *ctx)
{
    --ctx->depth;
    if (ctx->depth == 0)
    {
        free_symbols(ctx->funct.symbols);
    }
    else
    {
        Symbol *current = ctx->funct.symbols;
        while (current)
        {
            if (current->depth > ctx->depth
                    && current->is_defined
                    && current->type == VARIABLE)
            {
                current->is_defined = false;
            }
            current = current->next;
        }
    }
}

void enter_function(Ctx *ctx, char *name)
{
    enter_scope(ctx);
    ctx->funct.name = name;
    ctx->funct.symbols = NULL;
    ctx->funct.max_temps = 0;
}

char *function_in(Ctx *ctx)
{
    if (ctx->depth > 0)
        return ctx->funct.name;
    return NULL;
}

Symbol *declare_symbol(Ctx *ctx, SymbolType type, char *name, int parameters,
        int depth, bool is_defined)
{
    Symbol *symbol = malloc(sizeof(Symbol));
    symbol->type = type;
    symbol->name = name;
    symbol->parameters = parameters;
    symbol->depth = (depth >= 0) ? depth : ctx->depth;
    symbol->function = (symbol->depth > 0) ? ctx->funct.name : NULL;
    symbol->is_defined = is_defined;
    symbol->next = NULL;

    Symbol **list = (symbol->depth > 0) ? &ctx->funct.symbols : &ctx->globals;
    if (!(*list))
    {
        *list = symbol;
    }
    else
    {
        Symbol *current = *list;
        while (current->next)
            current = current->next;
        current->next = symbol;
    }
    return symbol;
}

void redeclare_symbol(Ctx *ctx, Symbol *symbol)
{
    symbol->depth = ctx->depth;
    symbol->is_defined = true;
}

Symbol *resolve_symbol_map(Symbol *map, char *name)
{
    Symbol *current = map;
    while (current)
    {
        if (!strcmp(current->name, name))
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

Symbol *resolve_symbol(Ctx *ctx, char *name)
{
    Symbol *symbol;
    if (function_in(ctx)
            && (symbol = resolve_symbol_map(ctx->funct.symbols, name)))
    {
        return symbol;
    }
    return resolve_symbol_map(ctx->globals, name);
}

int alloc_string_literal(Ctx *ctx, char *str)
{
    Section **cur = &ctx->sections;
    while (*cur)
    {
        if ((*cur)->type == SEC_STRING
                && !strcmp((*cur)->value.str, str))
        {
            return (*cur)->index;
        }
        cur = &(*cur)->next;
    }

    int lbl = anon_label(ctx);
    Section *section = malloc(sizeof(Section));
    section->index = lbl;
    section->value.str = str;
    section->type = SEC_STRING;
    section->next = NULL;

    *cur = section;
    set_output(ctx, OUT_CONST);
    putlabeln_global(ctx, lbl);
    //fprintf(output(ctx), "; \"%s\"\n", str);
    fprintf(output(ctx), "byte ");
    fprint_string_bytes(output(ctx), str);
    set_output(ctx, OUT_TEXT);
    return lbl;
}

int alloc_array_literal(Ctx *ctx, int *arr, int size)
{
    Section **cur = &ctx->sections;
    while (*cur)
    {
        if ((*cur)->type == SEC_ARRAY_LITERAL
                && (*cur)->size == size)
        {
            bool match = true;
            for (int i = 0; i < size; ++i)
            {
                if ((*cur)->value.arr[i] != arr[i])
                {
                    match = false;
                    break;
                }
            }
            if (match)
            {
                free(arr);
                return (*cur)->index;
            }
        }
        cur = &(*cur)->next;
    }

    int lbl = anon_label(ctx);
    Section *section = malloc(sizeof(Section));
    section->index = lbl;
    section->size = size;
    section->value.arr = arr;
    section->type = SEC_ARRAY_LITERAL;
    section->next = NULL;

    *cur = section;
    set_output(ctx, OUT_CONST);
    putlabeln_global(ctx, lbl);
    fprintf(output(ctx), "word ");
    for (int i = 0; i < size; ++i)
    {
        if (i != 0)
            fprintf(output(ctx), ", ");
        fprintf(output(ctx), "%d", arr[i]);
    }
    fprintf(output(ctx), "\n");
    set_output(ctx, OUT_TEXT);
    return lbl;
}

int alloc_array(Ctx *ctx, int size)
{
    int lbl = anon_label(ctx);
    set_output(ctx, OUT_DATA);
    putlabeln_global(ctx, lbl);
    putreserve(ctx, size);
    set_output(ctx, OUT_TEXT);
    return lbl;
}

void fprint_symbol(FILE *file, Symbol *symbol)
{
    if (symbol->function)
    {
        putfunctionname(file, symbol->function);
        fprintf(file, ".");
    }
    fprintf(file, "%s", symbol->name);
}

void free_symbols(Symbol *symbols)
{
    if (!symbols)
        return;
    if (symbols->next)
        free_symbols(symbols->next);
    free(symbols);
}

