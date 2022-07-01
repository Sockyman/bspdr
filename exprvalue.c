#include "exprvalue.h"
#include "expression.h"

Exval exval_from_target(Ctx *ctx, ExTarget target)
{
    switch (target.type)
    {
        case DISCARD:
            return exval_none();
        case ANY:
            return exval_temp(ctx);
        case CONDITION:
            return exval_condition();
        case EXVAL:
            return target.target;
    }
    return exval_none();
}

void release_temp(Ctx *ctx, Exval exval)
{

}

Exval exval_direct(Expression *expr)
{
    Exval exval = { EX_DIRECT, EX_EXPRESSION, { expr } };
    return exval;
}

Exval exval_direct_index(int index)
{
    Exval exval = { EX_DIRECT, EX_INDEX };
    exval.value.index = index;
    return exval;
}

Exval exval_immediate(Expression *expr)
{
    Exval exval = { EX_IMMEDIATE, EX_EXPRESSION, { expr } };
    return exval;
}

Exval exval_section(int index)
{
    Exval exval = { EX_IMMEDIATE, EX_INDEX };
    exval.value.index = index;
    return exval;
}

Exval exval_temp(Ctx *ctx)
{
    Exval exval = { EX_TEMP };
    exval.value.index = ctx->funct.temps++;
    return exval;
}

Exval exval_none()
{
    Exval exval = { EX_NONE };
    return exval;
}

Exval exval_condition()
{
    Exval exval = { EX_CONDITION };
    return exval;
}

Exval exval_param(char *function, int index)
{
    Exval exval = { EX_PARAMETER };
    exval.value.param.function = function;
    exval.value.param.index = index;
    return exval;
}

Exval exval_return()
{
    Exval exval = { EX_RETURN };
    return exval;
}

ExTarget target_any()
{
    ExTarget target = { ANY };
    return target;
}

ExTarget target_discard()
{
    ExTarget target = { DISCARD };
    return target;
}

ExTarget target_exval(Exval target)
{
    ExTarget extarget = { EXVAL, target };
    return extarget;
}

ExTarget target_condition()
{
    ExTarget extarget = { CONDITION };
    return extarget;
}

bool is_constexpr(Exval *x, Exval *y, Exval *condition, Operation operation)
{
    if (condition)
    {
        return (x->type == EX_IMMEDIATE && x->data_type == EX_EXPRESSION)
                && (y->type == EX_IMMEDIATE && y->data_type == EX_EXPRESSION)
                && (condition->type == EX_IMMEDIATE && condition->data_type == EX_EXPRESSION);
    }
    return (x->type == EX_IMMEDIATE && x->data_type == EX_EXPRESSION
            && (!y || (y->type == EX_IMMEDIATE && y->data_type == EX_EXPRESSION))
            && operation_is_constexpr(operation));
}

