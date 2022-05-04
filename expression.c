#include "expression.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Expression *alloc_expression(Trace trace, enum ExprType type)
{
    Expression *expr = malloc(sizeof(Expression));
    expr->trace = trace;
    expr->type = type;
    return expr;
}

Expression *expression_symbolic(Trace trace, char *symbol)
{
    Expression *expr = alloc_expression(trace, SYMBOLIC);
    expr->value.symbol = strdup(symbol);
    return expr;
}

Expression *expression_literal(Trace trace, int value)
{
    Expression *expr = alloc_expression(trace, LITERAL);
    expr->value.literal = value;
    return expr;
}

Expression *expression_binary(Trace trace, enum Operation operation, Expression *x, Expression *y)
{
    Expression *expr = alloc_expression(trace, BINARY_OPERATION);
    expr->value.operative.operation = operation;
    expr->value.operative.x = x;
    expr->value.operative.y = y;
    return expr;
}

Expression *expression_unary(Trace trace, enum Operation operation, Expression *x)
{
    Expression *expr = alloc_expression(trace, UNARY_OPERATION);
    expr->value.operative.operation = operation;
    expr->value.operative.x = x;
    return expr;
}

void free_expression(Expression *expr)
{
    if (expr->type == BINARY_OPERATION)
    {
        free_expression(expr->value.operative.x);
        free_expression(expr->value.operative.y);
    }
    else if (expr->type == UNARY_OPERATION)
    {
        free_expression(expr->value.operative.x);
    }
    else if (expr->type == SYMBOLIC)
    {
        free(expr->value.symbol);
    }
    free(expr);
}

bool operation_is_constexpr(enum Operation operation)
{
    switch(operation)
    {
        case OPER_ASSIGN:
        case OPER_ADDRESS:
        case OPER_DEREF:
            return false;
        default:
            return true;
    }
    fprintf(stderr, "Unknown operation.");
    return false;
}

char *get_operator(enum Operation operation)
{
    switch (operation)
    {
        case OPER_ADD:
            return "+";
        case OPER_SUBTRACT:
            return "-";
        case OPER_ASSIGN:
            return "=";
        case OPER_BIT_AND:
            return "&";
        case OPER_BIT_OR:
            return "|";
        case OPER_BIT_XOR:
            return "^";
        case OPER_BIT_NOT:
            return "~";
        case OPER_NEGATE:
            return "-";
        case OPER_LOG_NOT:
            return "!";
        case OPER_LOG_AND:
            return "&&";
        case OPER_LOG_OR:
            return "||";
        case OPER_LESS:
            return "<";
        case OPER_GREATER:
            return ">";
        case OPER_LESS_EQ:
            return "<=";
        case OPER_GREATER_EQ:
            return ">=";
        case OPER_EQUAL:
            return "==";
        case OPER_NOT_EQUAL:
            return "!=";
        case OPER_SHIFT_LEFT:
            return "<<";
        case OPER_SHIFT_RIGHT:
            return ">>";
        case OPER_ADDRESS:
            return "-1&"; // Wacky so it can be handled by assembler.
        case OPER_DEREF:
            return "1*"; // Similar reason.
    }
    fprintf(stderr, "Invalid operator");
    return "";
}

void fprint_expression(FILE *file, Expression *expr)
{
    fprintf(file, "(");
    switch (expr->type)
    {
        case SYMBOLIC:
            fprintf(file, "%s", expr->value.symbol);
            break;
        case LITERAL:
            fprintf(file, "%d", expr->value.literal);
            break;
        case BINARY_OPERATION:
            fprint_expression(file, expr->value.operative.x);
            fprintf(file, " %s ", get_operator(expr->value.operative.operation));
            fprint_expression(file, expr->value.operative.y);
            break;
        case UNARY_OPERATION:
            fprintf(file, "%s", get_operator(expr->value.operative.operation));
            fprint_expression(file, expr->value.operative.x);
            break;
    }
    fprintf(file, ")");
}

Expression *expression_array(Trace trace, Expression *x, Expression *y)
{
    return expression_unary(trace, OPER_DEREF,
        expression_binary(trace, OPER_ADD,
            x,
            expression_binary(trace, OPER_SHIFT_LEFT,
                y,
                expression_literal(trace, 1)
            )));
}

