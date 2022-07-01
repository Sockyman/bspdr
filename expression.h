#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "trace.h"
#include "context.h"
#include <stdio.h>
#include <stdbool.h>

typedef enum ExprType
{
    BINARY_OPERATION,
    UNARY_OPERATION,
    TERNARY_CONDITIONAL,
    SYMBOLIC,
    LITERAL,
    FUNCTION_CALL,
    STRING_LITERAL,
    ARRAY_ALLOC,
    ARRAY_LITERAL,
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
    OPER_MULTIPLY,
    OPER_DIVIDE,
    OPER_MODULO,
    OPER_ASSIGN_BY,
} Operation;

struct ExprTernary
{
    struct Expression *condition;
    struct Expression *a;
    struct Expression *b;
};

struct ExprOperation
{
    Operation operation;
    struct Expression *x;
    struct Expression *y;
};

struct ExprFunction
{
    char *name;
    struct Expression *arguments;
};

union ExprContents
{
    char *symbol;
    struct ExprOperation operative;
    struct ExprFunction function;
    struct ExprTernary ternary;
    int literal;
};

typedef struct Expression
{
    Trace trace;
    ExprType type;
    union ExprContents value;
    struct Expression *next;
} Expression;

Expression *expression_symbolic(Trace trace, char *symbol);
Expression *expression_literal(Trace trace, int value);
Expression *expression_binary(Trace trace, enum Operation operation, Expression *x, Expression *y);
Expression *expression_unary(Trace trace, enum Operation operation, Expression *x);
Expression *expression_ternary(Trace trace, Expression *condition, Expression *a, Expression *b);
Expression *expression_array(Trace trace, Expression *x, Expression *y);
Expression *expression_assign_by(Trace trace, enum Operation operation, Expression *x, Expression *y);
Expression *expression_function(Trace trace, char *name, Expression *arguments);
Expression *expression_string(Trace trace, char *str);
Expression *expression_alloc_array(Trace trace, Expression *size);
Expression *expression_literal_array(Trace trace, Expression *contents);
void free_expression(Expression *expr);

int operation(Operation operation, int x, int y);
bool try_reduce(Expression *expr, int *output);
bool operation_is_constexpr(enum Operation operation);
void fprint_expression(FILE *file, Ctx *ctx, Expression *expr);

#endif

