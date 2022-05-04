#ifndef CONTEXT_H
#define CONTEXT_H

#include "error.h"
#include <stdbool.h>
#include <stdio.h>

#define MAX_TEMP 256

struct error;

typedef enum SymbolType
{
    VARIABLE,
    LABEL,
    FUNCTION,
} SymbolType;

typedef struct Symbol
{
    SymbolType type;
    char *name;
    int parameters;
    int depth;
    struct Symbol *next;
} Symbol;

typedef struct Function
{
    char *name;
    Symbol *symbols;
    int expr_temps;
    int max_temps;
    bool temps[MAX_TEMP];
} Function;

typedef struct Ctx
{
    int label_count;
    FILE *output;
    Symbol *globals;
    Function funct;
    int depth;
} Ctx;

void declare_variable(Ctx *ctx, char *name);
void print_symbol(Ctx *ctx, char *name);
void enter_scope(Ctx *ctx);
void leave_scope(Ctx *ctx);
bool in_function(Ctx *ctx);
Ctx create_context(FILE *output_file);
int anon_label(Ctx *ctx);

#endif

