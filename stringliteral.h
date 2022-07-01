#ifndef STRINGLITERAL_H
#define STRINGLITERAL_H

#include "context.h"
#include <stdio.h>

int read_character(char *string, int i, int *c);
int parse_character(char *string);
char *remove_quotes(char *string);
void fprint_string_bytes(FILE *file, char *str);
char *combine_string(char *str_1, char *str_2);

#endif

