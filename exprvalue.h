#ifndef EXPRVALUE_H
#define EXPRVALUE_H

#include "expression.h"
#include "context.h"

typedef enum {
    FLAG_ZERO,
    FLAG_NOT_ZERO,
    FLAG_CARRY,
    FLAG_NOT_CARRY,
} Flag;

typedef enum ExvalType
{
    EX_DIRECT,
    EX_IMMEDIATE,
    EX_TEMP,
    EX_CONDITION,
    EX_NONE,
    EX_RETURN,
    EX_PARAMETER,
    EX_FLAG,
} ExvalType;

struct ExvalParamValue
{
    char *function;
    int index;
};

union ExvalValue
{
    Expression *expression;
    int index;
    struct ExvalParamValue param;
    Flag flag;
};

typedef enum ExvalDataType
{
    EX_INDEX,
    EX_EXPRESSION,
    EX_DATA_FLAG,
} ExvalDataType;

typedef struct Exval
{
    ExvalType type;
    ExvalDataType data_type;
    union ExvalValue value;
} Exval;

typedef enum TargetType
{
    DISCARD,
    ANY,
    EXVAL,
    CONDITION,
} TargetType;

typedef struct ExTarget
{
    TargetType type;
    Exval target;
} ExTarget;

Exval exval_from_target(Ctx *ctx, ExTarget target);
void release_temp(Ctx *ctx, Exval exval);

Exval exval_direct(Expression *expr);
Exval exval_immediate(Expression *expr);
Exval exval_condition();
Exval exval_temp(Ctx *ctx);
Exval exval_none();
Exval exval_param(char *function, int index);
Exval exval_return();
Exval exval_section(int index);
Exval exval_flag(Flag flag);

bool is_constexpr(Exval *x, Exval *y, Exval *condition, Operation operation);

ExTarget target_any();
ExTarget target_discard();
ExTarget target_exval(Exval target);
ExTarget target_condition();

#endif

