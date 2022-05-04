#ifndef ERROR_H
#define ERROR_H

typedef enum result
{
    ERR = 0,
    OK = 1,
} result;

struct error
{
    char *text;
    struct error *next;
};


#endif

