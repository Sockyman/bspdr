#ifndef CONTEXT_H
#define CONTEXT_H

#include "error.h"
#include "trace.h"
#include <stdbool.h>
#include <stdio.h>

#define MAX_TEMP 256

typedef enum ScopeDepth
{
    CURRENT_SCOPE = -1,
    GLOBAL_SCOPE = 0,
    FUNCTION_SCOPE = 1,
} ScopeDepth;

typedef enum SymbolType
{
    VARIABLE,
    LABEL,
    FUNCTION,
} SymbolType;

typedef enum OutputFile
{
    OUT_TEXT,
    OUT_DATA,
    OUT_CONST,
} OutputFile;

typedef enum ComplexOperator
{
    COMP_MULTIPLY,
    COMP_DIVIDE,
    COMP_MODULO,
} ComplexOperator;

typedef struct Symbol
{
    SymbolType type;
    char *name;
    char *function;
    int parameters;
    int depth;
    bool is_defined;
    struct Symbol *next;
} Symbol;

typedef struct Function
{
    char *name;
    Symbol *symbols;
    int expr_temps;
    int max_temps;
    int temps;
} Function;

typedef enum SectionType
{
    SEC_STRING,
    SEC_ARRAY,
    SEC_ARRAY_LITERAL,
} SectionType;

union SectionValue
{
    char *str;
    int *arr;
};

typedef struct Section
{
    SectionType type;
    int index;
    int size;
    union SectionValue value;
    struct Section *next;
} Section;

typedef struct Ctx
{
    OutputFile file;
    int label_count;
    FILE *output[3];
    Symbol *globals;
    Section *sections;
    Function funct;
    int depth;
    bool complex_operators[3];
} Ctx;

Symbol *declare_symbol(Ctx *ctx, SymbolType type, char *name, int parameters, int depth, bool is_defined);
void redeclare_symbol(Ctx *ctx, Symbol *symbol);

void enter_scope(Ctx *ctx);
void leave_scope(Ctx *ctx);
void enter_function(Ctx *ctx, char *name);
char *function_in(Ctx *ctx);
int anon_label(Ctx *ctx);
Ctx create_context(FILE *output_text, FILE *output_data, FILE *output_const);
FILE *output(Ctx *ctx);
void set_output(Ctx *ctx, OutputFile file);

Symbol *resolve_symbol_map(Symbol *map, char *name);
Symbol *resolve_symbol(Ctx *ctx, char *name);
void fprint_symbol(FILE *file, Symbol *symbol);

Section *get_section(Ctx *ctx);
Section *put_section(Ctx *ctx);
int alloc_string_literal(Ctx *ctx, char *str);
int alloc_array(Ctx *ctx, int size);
int alloc_array_literal(Ctx *ctx, int *arr, int size);

void free_symbols(Symbol *symbols);
void free_sections(Section *sections);

#endif

