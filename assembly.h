#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include "expression.h"
#include "exprvalue.h"
#include "context.h"
#include <stdbool.h>

#define ZERO_FLAG 0b0100
#define CARRY_FLAG 0b1000

void putins_imp(Ctx *ctx, char *name);
void putins_ind(Ctx *ctx, char *name);
void putins_exval(Ctx *ctx, char *name, Exval exval, int offset);
void putins_imm(Ctx *ctx, char *name, Expression *expr, int offset);
void putins_dir(Ctx *ctx, char *name, Expression *expr, int offset);
void putins_dir_temp(Ctx *ctx, char *name, int temp_index, int offset);
void putins_imm_int(Ctx *ctx, char *name, int value);
void putins_dir_str(Ctx *ctx, char *name, char *value, int offset);
void putins_dir_symbol(Ctx *ctx, char *name, Symbol *symbol, int offset);
void putins_dir_anon_label(Ctx *ctx, char *name, int label);
void putins_dir_anon_label_global(Ctx *ctx, char *name, int label);
void putins_dir_sec(Ctx *ctx, char *name, int index, int offset);
void putins_imm_sec(Ctx *ctx, char *name, int index, int offset);

// void putins_reg_dir(Ctx *ctx, char *prefix, char reg, char *value, int offset);
// void putins_reg_imm(Ctx *ctx, char *prefix, char reg, char *value, int offset);

void putins_param(Ctx *ctx, char *name, char *function, int index, int offset);
void putins_return(Ctx *ctx, char *name, int offset);

void loada(Ctx *ctx, Exval exval, int offset);
void storea(Ctx *ctx, Exval exval, int offset);

void putlabel(Ctx *ctx, char *name);
void putlabel_function(Ctx *ctx, char *name);
void putfunctionname(FILE *file, char *name);
void putlabel_symbol(Ctx *ctx, Symbol *symbol);
void putlabeln(Ctx *ctx, int n);
void putlabeln_global(Ctx *ctx, int n);
void putreserve(Ctx *ctx, int words);

#endif

