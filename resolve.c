#include "exprvalue.h"
#include "assembly.h"
#include "resolve.h"
#include "error.h"
#include <stdlib.h>
#include <assert.h>

Exval resolve_binary(Ctx *ctx, Expression *expr, ExTarget target)
{
    switch (expr->value.operative.operation)
    {
        case OPER_ASSIGN:
            return do_assign(ctx, expr->value.operative.x, expr->value.operative.y, target);
        case OPER_LOG_AND:
            return do_logical(ctx, expr->value.operative.x, expr->value.operative.y, target, "jz");
        case OPER_LOG_OR:
            return do_logical(ctx, expr->value.operative.x, expr->value.operative.y, target, "jnz");
        default:
            return resolve_binary_generic(ctx, expr, target);
    }
}

Exval resolve_binary_generic(Ctx *ctx, Expression *expr, ExTarget target)
{
    Exval x = resolve(ctx, expr->value.operative.x, target_any());
    Exval y = resolve(ctx, expr->value.operative.y, target_any());

    if (is_constexpr(&x, &y, NULL, expr->value.operative.operation))
    {
        return resolve_literal(ctx, expr, target);
    }

    release_temp(ctx, x);
    release_temp(ctx, y);
    Exval t = exval_from_target(ctx, target);
    switch (expr->value.operative.operation)
    {
        case OPER_ADD:
            return do_additive(ctx, x, y, t, "add", "addc");
        case OPER_SUBTRACT:
            return do_additive(ctx, x, y, t, "sub", "subc");
        case OPER_BIT_AND:
            return do_bitwise(ctx, x, y, t, "and");
        case OPER_BIT_OR:
            return do_bitwise(ctx, x, y, t, "or");
        case OPER_BIT_XOR:
            return do_bitwise(ctx, x, y, t, "xor");
        case OPER_LESS:
            return do_relational(ctx, x, y, t, false);
        case OPER_GREATER:
            return do_relational(ctx, y, x, t, false);
        case OPER_LESS_EQ:
            return do_relational(ctx, y, x, t, true);
        case OPER_GREATER_EQ:
            return do_relational(ctx, x, y, t, true);
        case OPER_EQUAL:
            return do_equality(ctx, x, y, t, true);
        case OPER_NOT_EQUAL:
            return do_equality(ctx, x, y, t, false);
        case OPER_SHIFT_LEFT:
            return do_shift(ctx, x, y, t, false);
        case OPER_SHIFT_RIGHT:
            return do_shift(ctx, x, y, t, true);
        case OPER_MULTIPLY:
            return do_complex(expr->trace, ctx, x, y, t, "mul");
        case OPER_DIVIDE:
            return do_complex(expr->trace, ctx, x, y, t, "div");
        case OPER_MODULO:
            return do_complex(expr->trace, ctx, x, y, t, "mod");
        default:
            fprintf(stderr, "Unhandled binary operation.\n");
            return exval_none();
    }
}

Exval resolve_unary(Ctx *ctx, Expression *expr, ExTarget target)
{
    switch (expr->value.operative.operation)
    {
        case OPER_LOG_NOT:
            return do_logical_not(ctx, expr->value.operative.x, target);
        case OPER_ADDRESS:
            return do_address_of(ctx, expr->value.operative.x, target);
        case OPER_DEREF:
            return do_deref(ctx, expr->value.operative.x, target);
        case OPER_ASSIGN_BY:
            return do_assign_by(ctx, expr, target);
        default:
            return resolve_unary_generic(ctx, expr, target);
    }
}

Exval resolve_unary_generic(Ctx *ctx, Expression *expr, ExTarget target)
{
    Exval x = resolve(ctx, expr->value.operative.x, target_any());
    if (is_constexpr(&x, NULL, NULL, expr->value.operative.operation))
    {
        return resolve_literal(ctx, expr, target);
    }

    release_temp(ctx, x);
    Exval t = exval_from_target(ctx, target);
    switch (expr->value.operative.operation)
    {
        case OPER_BIT_NOT:
            return do_bitwise_not(ctx, x, t);
        case OPER_NEGATE:
            return do_negate(ctx, x, t);
        default:
            break;
    }
    fprintf(stderr, "Unhandled unary operation.\n");
    return exval_none();
}

Exval resolve_symbolic(Ctx *ctx, Expression *expr, ExTarget target)
{
    Symbol *symbol = resolve_symbol(ctx, expr->value.symbol);
    if (!symbol)
        error(&expr->trace, "\'%s\' is not declared.", expr->value.symbol);
    else if (symbol->type != VARIABLE)
        error(&expr->trace, "\'%s\' is not a variable.", expr->value.symbol);
    else if (!symbol->is_defined)
        error(&expr->trace, "\'%s\' is not in scope.", expr->value.symbol);

    switch (target.type)
    {
        case ANY:
            return exval_direct(expr);
        case CONDITION:
            return do_move(ctx, exval_direct(expr), exval_condition());
        case EXVAL:
            return do_move(ctx, exval_direct(expr), target.target);
        default:
            return exval_none();

    }
}

Exval resolve_literal(Ctx *ctx, Expression *expr, ExTarget target)
{
    switch (target.type)
    {
        case ANY:
            return exval_immediate(expr);
        case CONDITION:
            return do_move(ctx, exval_immediate(expr), exval_condition());
        case EXVAL:
            return do_move(ctx, exval_immediate(expr), target.target);
        default:
            return exval_none();
    }
}

Exval resolve_string_literal(Ctx *ctx, Expression *expr, ExTarget target)
{

    int lbl = alloc_string_literal(ctx, expr->value.symbol);
    Exval val = exval_section(lbl);
    if (target.type == ANY)
        return val;
    else if (target.type != DISCARD)
        return do_move(ctx, val, exval_from_target(ctx, target));
    return exval_none();
}

Exval resolve_array_alloc(Ctx *ctx, Expression *expr, ExTarget target)
{
    int size;
    if (try_reduce(expr->value.operative.x, &size))
    {
        int arr = alloc_array(ctx, size);
        Exval val = exval_section(arr);
        if (target.type == ANY)
            return val;
        else if (target.type != DISCARD)
            return do_move(ctx, val, exval_from_target(ctx, target));
    }
    else
    {
        error(&expr->trace, "array size must be a compile time constant.");
    }
    return exval_none();
}

Exval resolve_literal_array(Ctx *ctx, Expression *expr, ExTarget target)
{
    Expression *current = expr->value.operative.x;
    int size = 0;
    while (current)
    {
        ++size;
        current = current->next;
    }
    int *arr = malloc(sizeof(int) * size);

    current = expr->value.operative.x;
    for (int i = 0; i < size; ++i)
    {
        int val;
        if (!try_reduce(current, &val))
        {
            error(&current->trace, "array literal elements must be compile time constants.");
            return exval_none();
        }
        arr[i] = val;
        current = current->next;
    }

    int lbl = alloc_array_literal(ctx, arr, size);
    Exval val = exval_section(lbl);

    if (target.type == ANY)
        return val;
    else if (target.type != DISCARD)
        return do_move(ctx, val, exval_from_target(ctx, target));
    return exval_none();
}

Exval resolve(Ctx *ctx, Expression *expr, ExTarget target)
{
    switch (expr->type)
    {
        case SYMBOLIC:
            return resolve_symbolic(ctx, expr, target);
        case LITERAL:
            return resolve_literal(ctx, expr, target);
        case UNARY_OPERATION:
            return resolve_unary(ctx, expr, target);
        case TERNARY_CONDITIONAL:
            return do_ternary(ctx, expr, target);
        case BINARY_OPERATION:
            return resolve_binary(ctx, expr, target);
        case FUNCTION_CALL:
            return resolve_function_call(ctx, expr, target);
        case STRING_LITERAL:
            return resolve_string_literal(ctx, expr, target);
        case ARRAY_ALLOC:
            return resolve_array_alloc(ctx, expr, target);
        case ARRAY_LITERAL:
            return resolve_literal_array(ctx, expr, target);
    }
    return exval_none();
}

Exval resolve_function_call(Ctx *ctx, Expression *expr, ExTarget target)
{
    char *name = expr->value.function.name;
    Expression *arg = expr->value.function.arguments;

    int param = 0;
    while (arg)
    {
        Exval arg_target = exval_param(name, param++);
        resolve(ctx, arg, target_exval(arg_target));
        arg = arg->next;
    }

    return call_function(expr->trace, ctx, target, name, param);
}

Exval do_complex(Trace trace, Ctx *ctx, Exval x, Exval y, Exval t, char *op)
{
    do_move(ctx, x, exval_param(op, 0));
    do_move(ctx, y, exval_param(op, 1));
    return call_function(trace, ctx, target_exval(t), op, 2);
}

Exval call_function(Trace trace, Ctx *ctx, ExTarget target, char *name, int param)
{
    Symbol *symbol = resolve_symbol(ctx, name);
    if (symbol && symbol->type != FUNCTION)
    {
        error(&trace, "\'%s\' is not a function.", name);
    }
    else if (symbol && symbol->parameters != param)
    {
        error(&trace, "\'%s\' declared with different parameters.", name);
    }
    else if (!symbol)
    {
        symbol = declare_symbol(ctx, FUNCTION, name, param, GLOBAL_SCOPE, false);
    }

    fprintf(output(ctx), "\tcall ");
    putfunctionname(output(ctx), symbol->name);
    fprintf(output(ctx), "\n");
    return do_move(ctx, exval_return(), exval_from_target(ctx, target));
}

Exval do_assign(Ctx *ctx, Expression *destination, Expression *source, ExTarget target)
{
    Exval d;
    Exval t = exval_from_target(ctx, target);

    if (destination->type == SYMBOLIC)
    {
        d = resolve(ctx, destination, target_any());
        resolve(ctx, source, target_exval(d));
        do_move(ctx, d, t);
        return t;
    }
    else if (destination->type == UNARY_OPERATION
            && destination->value.operative.operation == OPER_DEREF)
    {
        Exval to_deref = resolve(ctx, destination->value.operative.x, target_any());
        if (to_deref.type == EX_IMMEDIATE)
        {
            to_deref.type = EX_DIRECT;
            resolve(ctx, source, target_exval(to_deref));
            return t;
        }

        Exval s = resolve(ctx, source, target_any());
        release_temp(ctx, s);

        putins_exval(ctx, "ldx", to_deref, 0);
        putins_exval(ctx, "ldy", to_deref, 1);

        loada(ctx, s, 0);
        putins_ind(ctx, "sta");
        storea(ctx, t, 0);

        int noinc_label = anon_label(ctx);
        putins_imp(ctx, "inx");
        putins_dir_anon_label(ctx, "jnc", noinc_label);
        putins_imp(ctx, "iny");
        putlabeln(ctx, noinc_label);

        loada(ctx, s, 1);
        putins_ind(ctx, "sta");
        storea(ctx, t, 1);

        return t;
    }
    error(&destination->trace, "cannot assign to unassignable expression.");
    return exval_none();
}

Exval do_assign_by(Ctx *ctx, Expression *expr, ExTarget target)
{
    return do_assign(ctx, expr->value.operative.x->value.operative.x,
            expr->value.operative.x, target);
}

Exval do_additive(Ctx *ctx, Exval x, Exval y, Exval t, char *working_ins, char *working_ins_carry)
{
    loada(ctx, x, 0);
    putins_exval(ctx, working_ins, y, 0);
    storea(ctx, t, 0);
    loada(ctx, x, 1);
    putins_exval(ctx, working_ins_carry, y, 1);
    storea(ctx, t, 1);
    return t;
}

Exval do_deref(Ctx *ctx, Expression *expr, ExTarget target)
{
    Exval x = resolve(ctx, expr, target_any());
    release_temp(ctx, x);
    Exval t = exval_from_target(ctx, target);
    if (x.type == EX_IMMEDIATE)
    {
        x.type = EX_DIRECT;
        if (target.type == ANY)
        {
            return x;
        }
        else
        {
            return do_move(ctx, x, t);
        }
    }
    else
    {
        putins_exval(ctx, "ldx", x, 0);
        putins_exval(ctx, "ldy", x, 1);
        putins_ind(ctx, "lda");
        storea(ctx, t, 0);
        int noinc_label = anon_label(ctx);
        putins_imp(ctx, "inx");
        putins_dir_anon_label(ctx, "jnc", noinc_label);
        putins_imp(ctx, "iny");
        putlabeln(ctx, noinc_label);
        putins_ind(ctx, "lda");
        storea(ctx, t, 1);
    }
    return t;
}

Exval do_address_of(Ctx *ctx, Expression *expr, ExTarget target)
{
    Exval x = resolve(ctx, expr, target_any());
    if (x.type == EX_DIRECT)
    {
        x.type = EX_IMMEDIATE;
    }
    else
    {
        error(&expr->trace, "cannot determine address of value.");
        return exval_none();
    }

    switch (target.type)
    {
        case ANY:
            return x;
        case EXVAL:
            return do_move(ctx, x, target.target);
        case CONDITION:
            return do_move(ctx, x, exval_condition());
        default:
            return x;
    }

}

Exval do_bitwise(Ctx *ctx, Exval x, Exval y, Exval t, char *working_ins)
{
    loada(ctx, x, 0);
    putins_exval(ctx, working_ins, y, 0);
    storea(ctx, t, 0);
    loada(ctx, x, 1);
    putins_exval(ctx, working_ins, y, 1);
    storea(ctx, t, 1);
    return t;
}

Exval do_logical(Ctx *ctx, Expression *first, Expression *second, ExTarget target, char *working_ins)
{
    int end_label = anon_label(ctx);
    Exval t = exval_from_target(ctx, target);

    resolve(ctx, first, target_condition());
    putins_dir_anon_label(ctx, working_ins, end_label);
    resolve(ctx, second, target_condition());
    putlabeln(ctx, end_label);
    if (t.type != EX_CONDITION)
    {
        storea(ctx, t, 0);
        storea(ctx, t, 1);
    }
    return t;
}

Exval do_equality(Ctx *ctx, Exval x, Exval y, Exval t, bool equal)
{
    int end_label = anon_label(ctx);
    loada(ctx, x, 0);
    putins_exval(ctx, "cmp", y, 0);
    putins_dir_anon_label(ctx, "jnz", end_label);
    loada(ctx, x, 1);
    putins_exval(ctx, "subc", y, 1);
    putlabeln(ctx, end_label);
    if (equal)
    {
        putins_imp(ctx, "tfa");
        putins_imm_int(ctx, "and", ZERO_FLAG);
    }
    storea(ctx, t, 0);
    storea(ctx, t, 1);
    return t;
}

Exval do_logical_not(Ctx *ctx, Expression *expr, ExTarget target)
{
    Exval t = exval_from_target(ctx, target);

    resolve(ctx, expr, target_condition());
    putins_imp(ctx, "tfa");
    if (t.type != EX_CONDITION)
    {
        putins_imp(ctx, "not");
        putins_imm_int(ctx, "and", ZERO_FLAG);
        storea(ctx, t, 0);
        storea(ctx, t, 1);
    }
    else
    {
        putins_imm_int(ctx, "xor", ZERO_FLAG);
        putins_imp(ctx, "taf");
    }

    return t;
}

Exval do_bitwise_not(Ctx *ctx, Exval x, Exval t)
{
    loada(ctx, x, 0);
    putins_imp(ctx, "not");
    storea(ctx, t, 0);
    loada(ctx, x, 1);
    putins_imp(ctx, "not");
    storea(ctx, t, 1);
    return t;
}

Exval do_negate(Ctx *ctx, Exval x, Exval t)
{
    loada(ctx, x, 1);
    putins_imp(ctx, "not");
    putins_imp(ctx, "tax");
    loada(ctx, x, 0);
    putins_imp(ctx, "not");
    putins_imp(ctx, "inc");
    storea(ctx, t, 0);
    putins_imp(ctx, "txa");
    putins_imm_int(ctx, "addc", 0);
    storea(ctx, t, 1);
    return t;
}

Exval do_move(Ctx *ctx, Exval source, Exval destination)
{
    if (destination.type != EX_NONE)
    {
        loada(ctx, source, 0);
        storea(ctx, destination, 0);
        loada(ctx, source, 1);
        storea(ctx, destination, 1);
    }
    return destination;
}

Exval do_relational(Ctx *ctx, Exval x, Exval y, Exval t, bool equal)
{
    loada(ctx, x, 0);
    putins_exval(ctx, "cmp", y, 0);
    loada(ctx, x, 1);
    putins_exval(ctx, "subc", y, 1);
    putins_imp(ctx, "tfa");
    if (!equal)
    {
        putins_imp(ctx, "not");
    }
    putins_imm_int(ctx, "and", CARRY_FLAG);
    storea(ctx, t, 0);
    storea(ctx, t, 1);

    return x;
}

Exval do_ternary(Ctx *ctx, Expression *expr, ExTarget target)
{
    int val;
    if (try_reduce(expr->value.ternary.condition, &val))
    {
        if (val)
        {
            return resolve(ctx, expr->value.ternary.a, target);
        }
        else
        {
            return resolve(ctx, expr->value.ternary.b, target);
        }
    }

    Exval src;
    Exval dest = exval_from_target(ctx, target);

    int label_alt = anon_label(ctx);
    int label_end = anon_label(ctx);
    resolve(ctx, expr->value.ternary.condition, target_condition());
    putins_dir_anon_label(ctx, "jz", label_alt);
    src = resolve(ctx, expr->value.ternary.a, target);
    do_move(ctx, src, dest);
    putins_dir_anon_label(ctx, "jmp", label_end);
    putlabeln(ctx, label_alt);
    src = resolve(ctx, expr->value.ternary.b, target);
    do_move(ctx, src, dest);
    putlabeln(ctx, label_end);
    return dest;
}

Exval do_shift(Ctx *ctx, Exval x, Exval y, Exval t, bool right)
{
    // Shifting by values greater then 255 will cause errors.

    if (!right && y.type == EX_IMMEDIATE
            && y.value.expression->type == LITERAL
            && y.value.expression->value.literal == 1)
    {
        loada(ctx, x, right ? 1 : 0);
        putins_imp(ctx, right ? "rsh" : "lsh");
        storea(ctx, t, right ? 1 : 0);
        loada(ctx, x, right ? 0 : 1);
        putins_imp(ctx, right ? "ror" : "rol");
        storea(ctx, t, right ? 0 : 1);
        return t;

    }

    int loop_label = anon_label(ctx);
    int end_label = anon_label(ctx);

    putins_exval(ctx, "ldx", y, 0);
    loada(ctx, x, 0);
    putins_exval(ctx, "ldy", x, 1);

    putlabeln(ctx, loop_label);
    putins_imp(ctx, "dex");
    putins_dir_anon_label(ctx, "jnc", end_label);

    if (!right)
    {
        putins_imp(ctx, "lsh");
        putins_imp(ctx, "xay");
        putins_imp(ctx, "rol");
        putins_imp(ctx, "xay");
    }
    else
    {
        putins_imp(ctx, "xay");
        putins_imp(ctx, "rsh");
        putins_imp(ctx, "xay");
        putins_imp(ctx, "ror");
    }

    putins_dir_anon_label(ctx, "jmp", loop_label);

    putlabeln(ctx, end_label);
    storea(ctx, t, 0);
    putins_imp(ctx, "tya");
    storea(ctx, t, 1);
    return t;
}

