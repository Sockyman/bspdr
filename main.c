#include "expression.h"
#include "assembly.h"
#include "parser.tab.h"
#include "resolve.h"
#include "token.h"
#include "compile.h"
#include "context.h"
#include "trace.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "c_compile.h"

extern Token *g_token;

void put_file(FILE *file)
{
    int c;
    rewind(file);
    while ((c = getc(file)) != EOF)
        putchar(c);
}

int main(int argc, char **argv)
{
    FILE *text = tmpfile();
    FILE *const_output = tmpfile();
    FILE *data = tmpfile();

    g_token = NULL;

    Ctx ctx = create_context(text, data, const_output);

    yyparse();
    compile_program(&ctx, g_token);

    //c_compile_program(g_token);

    if (g_token)
        free_token(g_token);
    free_filenames();
    free_symbols(ctx.globals);

    if (!is_okay())
        return 1;

    put_file(text);
    put_file(const_output);
    put_file(data);

    fclose(text);
    fclose(const_output);
    fclose(data);

    return 0;
}

