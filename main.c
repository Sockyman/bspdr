#include "expression.h"
#include "assembly.h"
#include "parser.tab.h"
#include "resolve.h"
#include <string.h>
#include <stdio.h>

Expression *global_expression;
extern Ctx g_ctx;

int main()
{
    /*puts(
        "%section data\n"
        "_condition:\n\t%res 1\n"
        "foo:\n\t%res 2\n"
        "bar:\n\t%res 2\n"
        "baz:\n\t%res 2\n"
        "_t_0:\n\t%res 1\n"
        "_t_1:\n\t%res 2\n"
        "_t_2:\n\t%res 2\n"
        "_t_3:\n\t%res 2\n"
        "_t_4:\n\t%res 2\n"
        "_t_5:\n\t%res 2\n"
        "%section text\n"
        );*/

    g_ctx = create_context(stdout);
    yyparse();
    return 0;
}

