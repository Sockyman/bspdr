
%{
#include "parser.tab.h"
#include "trace.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
%}

%option noyywrap

%%

"let"                       { count(yytext); return LET; }
"return"                    { count(yytext); return RETURN; }
"if"                        { count(yytext); return IF; }
"else"                      { count(yytext); return ELSE; }
"while"                     { count(yytext); return WHILE; }
"do"                        { count(yytext); return DO; }
"goto"                      { count(yytext); return GOTO; }

[a-zA-Z_][a-zA-Z0-9_]*  {
    count(yytext);
    yylval.str = strdup(yytext);
    return IDENTIFIER;
}

[0-9]+  {
    count(yytext);
    yylval.integer = atoi(yytext);
    return INTEGER;
}

0[xX][0-9a-fA-F]+  {
    count(yytext);
    yylval.integer = strtol(yytext, NULL, 16);
    return INTEGER;
}

0[bB][01]+  {
    count(yytext);
    yylval.integer = strtol(yytext + 2, NULL, 2);
    return INTEGER;
}

^#" "[0-9]+" "\".*\"(" "[0-9]+)*\n  {
    int lineno;
    char filename[128];
    sscanf(yytext, "# %d %s", &lineno, filename);
    trace_set(filename, lineno);
    fprint_trace(stderr, trace_get());
}

``[ -_a-~\n\t\r]*``         { count(yytext); yylval.str = strdup(yytext); return INLINE_ASM; }

"=="                        { count(yytext); return EQUAL; }
"<="                        { count(yytext); return LESS_EQ; }
">="                        { count(yytext); return GREATER_EQ; }
"!="                        { count(yytext); return NOT_EQ; }
">>"                        { count(yytext); return SHIFT_R; }
"<<"                        { count(yytext); return SHIFT_L; }
"&&"                        { count(yytext); return AND; }
"||"                        { count(yytext); return OR; }

\n                          { count(yytext); }
[ \t\r]                     { count(yytext); }

.                           { count(yytext); return yytext[0]; }

%%

