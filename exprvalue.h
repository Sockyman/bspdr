#ifndef EXPRVALUE_H
#define EXPRVALUE_H

#include "expression.h"

typedef enum ExvalType
{
    EX_DIRECT,
    EX_IMMEDIATE,
    EX_TEMP,
    EX_CONDITION,
    EX_NONE,
} ExvalType;

union ExvalValue
{
    Expression *expression;
    int index;
};

typedef struct Exval
{
    ExvalType type;
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

Exval exval_from_target(ExTarget target);
void release_temp(Exval exval);

Exval exval_direct(Expression *expr);
Exval exval_immediate(Expression *expr);
Exval exval_condition();
Exval exval_temp();
Exval exval_none();

ExTarget target_any();
ExTarget target_discard();
ExTarget target_exval(Exval target);
ExTarget target_condition();

#endif

