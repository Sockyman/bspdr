#include "error.h"
#include "trace.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

int error_count = 0;

void error(Trace *trace, char *format, ...)
{
    if (trace)
    {
        fprint_trace(stderr, *trace);
        fprintf(stderr, ": ");
    }
    va_list valist;
    va_start(valist, format);
    fprintf(stderr, "\033[0;31merror:\033[0m ");
    vfprintf(stderr, format, valist);
    fprintf(stderr, "\n");
    ++error_count;
}

bool is_okay(void)
{
    return error_count == 0;
}

