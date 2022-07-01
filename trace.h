#ifndef TRACE_H
#define TRACE_H

#include <stdio.h>

#define FILENAME_CACHE_SIZE 256

#define YYLTYPE Trace
#define YYLTYPE_IS_DECLARED

typedef struct Trace
{
    char *filename;
    int first_line;
    int last_line;
    int first_column;
    int last_column;
} Trace;

void free_filenames(void);
void fprint_trace(FILE *file, Trace trace);
Trace trace_new(char *filename, int lineno, int charno);
Trace count(Trace trace, char *text);
Trace trace_span(Trace trace_1, Trace trace_2);

#endif

