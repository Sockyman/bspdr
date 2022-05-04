#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "trace.h"
#include <stdio.h>
#include <stdbool.h>

typedef enum ExprType
{
    BINARY_OPERATION,
    UNARY_OPERATION,
    SYMBOLIC,
    LITERAL,
} ExprType;

typedef enum Operation
{
    OPER_ADD,
    OPER_SUBTRACT,
    OPER_ASSIGN,
    OPER_BIT_AND,
    OPER_BIT_OR,
    OPER_BIT_XOR,
    OPER_BIT_NOT,
    OPER_NEGATE,
    OPER_LOG_AND,
    OPER_LOG_OR,
    OPER_LOG_NOT,
    OPER_EQUAL,
    OPER_NOT_EQUAL,
    OPER_LESS,
    OPER_GREATER,
    OPER_LESS_EQ,
    OPER_GREATER_EQ,
    OPER_SHIFT_LEFT,
    OPER_SHIFT_RIGHT,
    OPER_ADDRESS,
    OPER_DEREF,
} Operation;

struct ExprOperation
{
    Operation operation;
    struct Expression *x;
    struct Expression *y;
};

union ExprContents
{
    struct ExprOperation operative;
    char *symbol;
    int literal;
};

typedef struct Expression
{
    Trace trace;
    ExprType type;
    union ExprContents value;
} Expression;

Expression *expression_symbolic(Trace trace, char *symbol);
Expression *expression_literal(Trace trace, int value);
Expression *expression_binary(Trace trace, enum Operation operation, Expression *x, Expression *y);
Expression *expression_unary(Trace trace, enum Operation operation, Expression *x);
Expression *expression_array(Trace trace, Expression *x, Expression *y);
void free_expression(Expression *expr);

bool operation_is_constexpr(enum Operation operation);
//bool expression_is_assignable(Expression *expr);
void fprint_expression(FILE *file, Expression *expr);

#endif

