#include "exprvalue.h"
#include "assembly.h"
#include "resolve.h"
#include <assert.h>

Exval resolve_binary(Ctx *ctx, Expression *expr, ExTarget target)
{
    switch (expr->value.operative.operation)
    {
        case OPER_ASSIGN:
            return do_assign(ctx, expr->value.operative.x, expr->value.operative.y, target);
        case OPER_LOG_AND:
            return do_logical(ctx, expr->value.operative.x, expr->value.operative.y, target, "jpz");
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

    if (x.type == EX_IMMEDIATE && y.type == EX_IMMEDIATE
            && operation_is_constexpr(expr->value.operative.operation))
    {
        return resolve_literal(ctx, expr, target);
    }

    release_temp(x);
    release_temp(y);
    Exval t = exval_from_target(target);
    switch (expr->value.operative.operation)
    {
        case OPER_ADD:
            return do_additive(ctx, x, y, t, "add", "adc");
        case OPER_SUBTRACT:
            return do_additive(ctx, x, y, t, "sub", "sbc");
        case OPER_BIT_AND:
            return do_bitwise(ctx, x, y, t, "and");
        case OPER_BIT_OR:
            return do_bitwise(ctx, x, y, t, "ora");
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
        default:
            return resolve_unary_generic(ctx, expr, target);
    }
}

Exval resolve_unary_generic(Ctx *ctx, Expression *expr, ExTarget target)
{
    Exval x = resolve(ctx, expr->value.operative.x, target_any());
    if (x.type == EX_IMMEDIATE && operation_is_constexpr(expr->value.operative.operation))
    {
        return resolve_literal(ctx, expr, target);
    }

    release_temp(x);
    Exval t = exval_from_target(target);
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
        case BINARY_OPERATION:
            return resolve_binary(ctx, expr, target);
    }
    return exval_none();
}


Exval do_assign(Ctx *ctx, Expression *destination, Expression *source, ExTarget target)
{
    Exval d;
    Exval t = exval_from_target(target);

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
        release_temp(s);

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
    fprintf(stderr, "error: cannot assign to unassignable expression.\n");
    return exval_none();
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
    release_temp(x);
    Exval t = exval_from_target(target);
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
        fprintf(stderr, "Cannot take address.\n");
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
    Exval t = exval_from_target(target);

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
    putins_exval(ctx, "sbc", y, 1);
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
    Exval t = exval_from_target(target);

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
    putins_imm_int(ctx, "adc", 0);
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
    putins_exval(ctx, "sbc", y, 1);
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

Exval do_shift(Ctx *ctx, Exval x, Exval y, Exval t, bool right)
{
    // Shifting by values greater then 255 will cause errors.

    //assert(!right);

    if (!right && y.type == EX_IMMEDIATE
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
        putins_imp(ctx, "way");
        putins_imp(ctx, "rol");
        putins_imp(ctx, "way");
    }
    else
    {
        putins_imp(ctx, "way");
        putins_imp(ctx, "rsh");
        putins_imp(ctx, "way");
        putins_imp(ctx, "ror");
    }

    putins_dir_anon_label(ctx, "jmp", loop_label);

    putlabeln(ctx, end_label);
    storea(ctx, t, 0);
    putins_imp(ctx, "tya");
    storea(ctx, t, 1);
    return t;
}

