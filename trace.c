#include "trace.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int filename_count = 0;
char *filename_cache[FILENAME_CACHE_SIZE];
Trace global_trace = { "", 0, 0 };

Trace trace_new(char *filename, int lineno, int charno)
{
    char *file = NULL;
    if (filename_count > FILENAME_CACHE_SIZE)
    {
        fprintf(stderr, "To many file paths.");
    }

    for (int i = 0; i < filename_count; ++i)
    {
        if (strcmp(filename_cache[i], filename) == 0)
        {
            file = filename_cache[i];
            break;
        }
    }
    if (!file)
    {
        filename_cache[filename_count++] = file = strdup(filename);
    }

    Trace trace = { file, lineno, charno };
    return trace;
}

Trace trace_advance(Trace trace, int lines, int chars)
{
    trace.lineno += lines;
    trace.charno += chars;
    return trace;
}

void free_filenames(void)
{
    for (int i = 0; i < filename_count; ++i)
    {
        free(filename_cache[i]);
    }
}

void fprint_trace(FILE *file, Trace trace)
{
    char *name = trace.filename;
    if (!name)
    {
        name = "STDIN";
    }
    fprintf(file, "%s:%d:%d", name, trace.lineno, trace.charno);
}


Trace trace_get(void)
{
    return global_trace;
}

Trace count(char *text)
{
    for (int i = 0; i < strlen(text); ++i)
    {
        if (text[i] == '\n')
        {
            ++global_trace.lineno;
            global_trace.charno = 0;
        }
        else
        {
            ++global_trace.charno;
        }
    }
    return global_trace;
}

Trace trace_set(char *filename, int lineno)
{
    global_trace = trace_new(filename, lineno, 0);
    return global_trace;
}

