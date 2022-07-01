#include "stringliteral.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int read_character(char *string, int i, int *c)
{
    char ch = string[i++];
    if (ch == '\0')
    {
        *c = EOF;
    }
    else if (ch == '\\')
    {
        ch = string[i++];
        switch (ch)
        {
            case 'n':
                *c = '\n';
                break;
            case 't':
                *c = '\t';
                break;
            case '0':
                *c = '\0';
                break;
            default:
                *c = ch;
                break;
        }
    }
    else
    {
        *c = ch;
    }
    return i;
}

int parse_character(char *string)
{
    int val = 0;
    read_character(string, 1, &val);
    return val;
}

char *remove_quotes(char *string)
{
    int len = strlen(string);
    char *new_string = malloc(len - 1);
    strncpy(new_string, string + 1, len - 2);
    new_string[len - 2] = '\0';
    return new_string;
}

void fprint_string_bytes(FILE *file, char *str)
{
    int i = 0;
    int c = 0;
    while (i = read_character(str, i, &c), c != EOF)
    {
        fprintf(file, "%d, ", c);
    }
    fprintf(file, "0\n");
}

char *combine_string(char *str_1, char *str_2)
{
    char *string = malloc(strlen(str_1) + strlen(str_2) + 1);
    strcpy(string, str_1);
    strcat(string, str_2);
    return string;
}

