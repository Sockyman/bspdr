#ifndef ERROR_H
#define ERROR_H

#include "trace.h"
#include <stdbool.h>

void error(Trace *trace, char *format, ...);
bool is_okay(void);

#endif

