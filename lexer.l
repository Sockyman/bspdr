
%{
#include "trace.h"
#include "parser.tab.h"
#include "stringliteral.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define YY_USER_ACTION \
    yylloc = count(yylloc, yytext);
%}

%option noyywrap noinput nounput

%%

"let"     { return LET; }
"return"  { return RETURN; }
"if"      { return IF; }
"else"    { return ELSE; }
"while"   { return WHILE; }
"do"      { return DO; }
"for"     { return FOR; }
"goto"    { return GOTO; }
"arr"     { return ARRAY_DEC; }
"extern"  { return EXTERN; }
"include_asm" { return INCLUDE_ASM; }

"A" {
    yylval.character = 'a';
    return REGISTER;
}

"X" {
    yylval.character = 'x';
    return REGISTER;
}

"Y" {
    yylval.character = 'y';
    return REGISTER;
}

[a-zA-Z_][a-zA-Z0-9_]*  {
    yylval.str = strdup(yytext);
    return IDENTIFIER;
}

[0-9]+  {
    yylval.integer = atoi(yytext);
    return INTEGER;
}

0[xX][0-9a-fA-F]+  {
    yylval.integer = strtol(yytext, NULL, 16);
    return INTEGER;
}

0[bB][01]+  {
    yylval.integer = strtol(yytext + 2, NULL, 2);
    return INTEGER;
}

\'([^\\\"\'\n]|(\\.))\'  {
    yylval.integer = parse_character(yytext);
    return INTEGER;
}

\"((\\.)|[^\n\'\"\\])*\"  {
    yylval.str = remove_quotes(yytext);
    return STRING;
}

^#" "[0-9]+" "\".*\"(" "[0-9]+)*\n  {
    int lineno;
    char filename[128];
    sscanf(yytext, "# %d %s", &lineno, filename);
    yylloc = trace_new(filename, lineno, 1);
}

``[ -_a-~\n\t\r]*``  {
    yylval.str = strdup(yytext);
    return INLINE_ASM;
}

"=="     { return EQUAL; }
"<="     { return LESS_EQ; }
">="     { return GREATER_EQ; }
"!="     { return NOT_EQ; }
">>"     { return SHIFT_R; }
"<<"     { return SHIFT_L; }
"&&"     { return AND; }
"||"     { return OR; }

\n       { }
[ \t\r]  { }

.        { return yytext[0]; }

%%

/*\'(\\.)|[^\'\"\\]\'  {*/
