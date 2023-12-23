#ifndef C_WRITER_H
#define C_WRITER_H

#include <stdio.h>

typedef struct
{
    FILE *header;
    FILE *body;
    int arr_count;
} C_Writer;

#endif

