#include "expression.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Expression *alloc_expression(Trace trace, enum ExprType type)
{
    Expression *expr = malloc(sizeof(Expression));
    expr->trace = trace;
    expr->type = type;
    expr->next = NULL;
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

Expression *expression_ternary(Trace trace, Expression *condition, Expression *a, Expression *b)
{
    Expression *expr = alloc_expression(trace, TERNARY_CONDITIONAL);
    expr->value.ternary.condition = condition;
    expr->value.ternary.a = a;
    expr->value.ternary.b = b;
    return expr;
}

void free_expression(Expression *expr)
{
    switch (expr->type)
    {
        case BINARY_OPERATION:
            free_expression(expr->value.operative.x);
            free_expression(expr->value.operative.y);
            break;
        case UNARY_OPERATION:
        case ARRAY_ALLOC:
            free_expression(expr->value.operative.x);
            break;
        case SYMBOLIC:
        case STRING_LITERAL:
            free(expr->value.symbol);
            break;
        case FUNCTION_CALL:
            free(expr->value.function.name);
            if (expr->value.function.arguments)
                free_expression(expr->value.function.arguments);
            break;
        case TERNARY_CONDITIONAL:
            free_expression(expr->value.ternary.condition);
            free_expression(expr->value.ternary.a);
            free_expression(expr->value.ternary.b);
            break;
        default:
            break;
    }
    if (expr->next)
    {
        free_expression(expr->next);
    }
    free(expr);
}

bool try_reduce(Expression *expr, int *output)
{
    int x;
    int y;
    int condition;
    switch (expr->type)
    {
        case STRING_LITERAL:
        case ARRAY_ALLOC:
        case FUNCTION_CALL:
        case ARRAY_LITERAL:
        case SYMBOLIC:
            return false;
        case LITERAL:
            *output = expr->value.literal;
            return true;
        case BINARY_OPERATION:
            if (operation_is_constexpr(expr->value.operative.operation)
                    && try_reduce(expr->value.operative.x, &x)
                    && try_reduce(expr->value.operative.y, &y))
            {
                *output = operation(expr->value.operative.operation, x, y);
                return true;
            }
            break;
        case UNARY_OPERATION:
            if (operation_is_constexpr(expr->value.operative.operation)
                    && try_reduce(expr->value.operative.x, &x))
            {
                *output = operation(expr->value.operative.operation, x, 0);
                return true;
            }
            break;
        case TERNARY_CONDITIONAL:
            if (try_reduce(expr->value.ternary.condition, &condition)
                    && try_reduce(expr->value.ternary.a, &x)
                    && try_reduce(expr->value.ternary.b, &y))
            {
                *output = condition ? x : y; 
                return true;
            }
            break;
    }
    return false;
}

int operation(Operation operation, int x, int y)
{
    switch (operation)
    {
        case OPER_ADD:
            return x + y;
        case OPER_SUBTRACT:
            return x - y;
        case OPER_BIT_AND:
            return x & y;
        case OPER_BIT_OR:
            return x | y;
        case OPER_BIT_XOR:
            return x ^ y;
        case OPER_BIT_NOT:
            return ~x;
        case OPER_LOG_AND:
            return x && y;
        case OPER_LOG_OR:
            return x || y;
        case OPER_LOG_NOT:
            return !x;
        case OPER_NEGATE:
            return -x;
        case OPER_EQUAL:
            return x == y;
        case OPER_NOT_EQUAL:
            return x != y;
        case OPER_GREATER:
            return x > y;
        case OPER_LESS:
            return x < y;
        case OPER_GREATER_EQ:
            return x >= y;
        case OPER_LESS_EQ:
            return x <= y;
        case OPER_SHIFT_LEFT:
            return x << y;
        case OPER_SHIFT_RIGHT:
            return x >> y;
        case OPER_MULTIPLY:
            return x * y;
        case OPER_DIVIDE:
            return x / y;
        case OPER_MODULO:
            return x % y;
        default:
            fprintf(stderr, "Invalid operation in operation()\n");
            return 0;
    }
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
        case OPER_MULTIPLY:
            return "*";
        case OPER_DIVIDE:
            return "/";
        case OPER_MODULO:
            return "%";
        /*case OPER_ADDRESS:
            return "-1&"; // Wacky so it can be handled by assembler.
        case OPER_DEREF:
            return "1*"; // Similar reason.*/
        default:
            return "";
    }
    fprintf(stderr, "Invalid operator");
    return "";
}

void fprint_expression(FILE *file, Ctx *ctx, Expression *expr)
{
    fprintf(file, "(");
    switch (expr->type)
    {
        case SYMBOLIC:
            {
                Symbol *symbol = resolve_symbol(ctx, expr->value.symbol);
                if (symbol)
                {
                    fprint_symbol(file, symbol);
                }
                else
                {
                    fprintf(file, "nill");
                }
            }
            break;
        case LITERAL:
            fprintf(file, "%d", expr->value.literal);
            break;
        case BINARY_OPERATION:
            fprint_expression(file, ctx, expr->value.operative.x);
            fprintf(file, " %s ", get_operator(expr->value.operative.operation));
            fprint_expression(file, ctx, expr->value.operative.y);
            break;
        case UNARY_OPERATION:
            fprintf(file, "%s", get_operator(expr->value.operative.operation));
            fprint_expression(file, ctx, expr->value.operative.x);
            break;
        case TERNARY_CONDITIONAL:
            fprint_expression(file, ctx, expr->value.ternary.condition);
            fprintf(file, " ? ");
            fprint_expression(file, ctx, expr->value.ternary.a);
            fprintf(file, " : ");
            fprint_expression(file, ctx, expr->value.ternary.b);
        default:
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

Expression *expression_assign_by(Trace trace, enum Operation operation, Expression *x, Expression *y)
{
    Expression *inside = expression_binary(trace, operation, x, y);
    Expression *expr = expression_unary(trace, OPER_ASSIGN_BY, inside);
    return expr;
}

Expression *expression_function(Trace trace, char *name, Expression *arguments)
{
    Expression *expr = alloc_expression(trace, FUNCTION_CALL);
    expr->value.function.name = strdup(name);
    expr->value.function.arguments = arguments;
    return expr;
}

Expression *expression_string(Trace trace, char *str)
{
    Expression *expr = alloc_expression(trace, STRING_LITERAL);
    expr->value.symbol = strdup(str);
    return expr;
}

Expression *expression_alloc_array(Trace trace, Expression *size)
{
    Expression *expr = alloc_expression(trace, ARRAY_ALLOC);
    expr->value.operative.x = size;
    return expr;
}

Expression *expression_literal_array(Trace trace, Expression *contents)
{
    Expression *expr = alloc_expression(trace, ARRAY_LITERAL);
    expr->value.operative.x = contents;
    return expr;
}

