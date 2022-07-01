#ifndef RESOLVE_H
#define RESOLVE_H

#include "expression.h"
#include "exprvalue.h"
#include "context.h"
#include <stdbool.h>

Exval resolve(Ctx *ctx, Expression *expr, ExTarget target);
Exval resolve_binary(Ctx *ctx, Expression *expr, ExTarget target);
Exval resolve_binary_generic(Ctx *ctx, Expression *expr, ExTarget target);
Exval resolve_unary(Ctx *ctx, Expression *expr, ExTarget target);
Exval resolve_unary_generic(Ctx *ctx, Expression *expr, ExTarget target);
Exval resolve_symbolic(Ctx *ctx, Expression *expr, ExTarget target);
Exval resolve_literal(Ctx *ctx, Expression *expr, ExTarget target);
Exval resolve_function_call(Ctx *ctx, Expression *expr, ExTarget target);
Exval resolve_string_literal(Ctx *ctx, Expression *expr, ExTarget target);
Exval resolve_array_alloc(Ctx *ctx, Expression *expr, ExTarget target);
Exval resolve_literal_array(Ctx *ctx, Expression *expr, ExTarget target);

Exval call_function(Trace trace, Ctx *ctx, ExTarget target, char *name, int param);
Exval do_complex(Trace trace, Ctx *ctx, Exval x, Exval y, Exval t, char *op);
Exval do_move(Ctx *ctx, Exval source, Exval destination);
Exval do_additive(Ctx *ctx, Exval x, Exval y, Exval t, char *working_ins, char *working_ins_carry);
Exval do_relational(Ctx *ctx, Exval x, Exval y, Exval t, bool equal);
Exval do_bitwise(Ctx *ctx, Exval x, Exval y, Exval t, char *working_ins);
Exval do_logical(Ctx *ctx, Expression *first, Expression *second, ExTarget target, char *working_ins);
Exval do_logical_not(Ctx *ctx, Expression *expr, ExTarget target);
Exval do_equality(Ctx *ctx, Exval x, Exval y, Exval t, bool equal);
Exval do_shift(Ctx *ctx, Exval x, Exval y, Exval t, bool right);
Exval do_bitwise_not(Ctx *ctx, Exval x, Exval t);
Exval do_negate(Ctx *ctx, Exval x, Exval t);
Exval do_address_of(Ctx *ctx, Expression *expr, ExTarget target);
Exval do_deref(Ctx *ctx, Expression *expr, ExTarget target);
Exval do_assign(Ctx *ctx, Expression *destination, Expression *source, ExTarget target);
Exval do_assign_by(Ctx *ctx, Expression *expr, ExTarget target);
Exval do_ternary(Ctx *ctx, Expression *expr, ExTarget target);

#endif

