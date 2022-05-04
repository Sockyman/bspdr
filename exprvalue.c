#include "exprvalue.h"

Exval exval_from_target(ExTarget target)
{
    switch (target.type)
    {
        case DISCARD:
            return exval_none();
        case ANY:
            return exval_temp();
        case CONDITION:
            return exval_condition();
        case EXVAL:
            return target.target;
    }
    return exval_none();
}

void release_temp(Exval exval)
{

}

Exval exval_direct(Expression *expr)
{
    Exval exval = { EX_DIRECT, { expr } };
    return exval;
}

Exval exval_immediate(Expression *expr)
{
    Exval exval = { EX_IMMEDIATE, { expr } };
    return exval;
}

Exval exval_temp()
{
    static int temps = 0;
    Exval exval = { EX_TEMP };
    exval.value.index = temps++;
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

