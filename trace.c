#include "trace.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int filename_count = 0;
char *filename_cache[FILENAME_CACHE_SIZE];
Trace global_trace = { "", 0, 0 };

Trace trace_new(char *filename, int first_line, int first_column)
{
    filename[strlen(filename) - 1] = '\0';
    filename = filename + 1;
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

    Trace trace = { file, first_line, first_line, first_column, first_column };
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
        name = "stdin";
    }
    fprintf(file, "%s:%d:%d", name, trace.first_line, trace.first_column);
}

Trace count(Trace trace, char *text)
{
    trace.first_column = trace.last_column;
    trace.first_line = trace.last_line;
    for (int i = 0; i < strlen(text); ++i)
    {
        if (text[i] == '\n')
        {
            ++trace.last_line;
            trace.last_column = 1;
        }
        else
        {
            ++trace.last_column;
        }
    }
    return trace;
}

Trace trace_span(Trace trace_1, Trace trace_2)
{
    Trace trace = trace_1;
    trace.last_line = trace_2.last_line;
    trace.last_column = trace_2.last_column;
    return trace;
}

