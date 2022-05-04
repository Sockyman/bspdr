#ifndef TRACE_H
#define TRACE_H

#include <stdio.h>

#define FILENAME_CACHE_SIZE 256

typedef struct Trace
{
    char *filename;
    int lineno;
    int charno;
} Trace;

Trace trace_new(char *filename, int lineno, int charno);
Trace trace_advance(Trace trace, int lines, int chars);
void free_filenames(void);
void fprint_trace(FILE *file, Trace trace);

Trace count(char *text);
Trace trace_get(void);
Trace trace_set(char *filename, int lineno);

#endif

