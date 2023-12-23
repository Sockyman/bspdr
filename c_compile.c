#include "c_compile.h"
#include "error.h"
#include <stdio.h>


void c_compile_program(Token *token)
{
    C_Writer writer;
    writer.header = tmpfile();
    writer.body = tmpfile();
    writer.arr_count = 0;

    fprintf(writer.header, "#include <stdint.h>\n");
    fprintf(writer.header, "#include <stddef.h>\n");

    c_compile(&writer, token);

    rewind(writer.header);
    int c = 0;
    while ((c = getc(writer.header)) != EOF)
    {
        putchar(c);
    }
    fclose(writer.header);

    rewind(writer.body);
    c = 0;
    while ((c = getc(writer.body)) != EOF)
    {
        putchar(c);
    }
    fclose(writer.body);
}

void c_compile(C_Writer* writer, Token *token)
{
    Token *t = token;
    while (t)
    {
        switch (t->type)
        {
            case TOK_COMPOUND:
                c_compile_compound(writer, t);
                break;
            case TOK_EXPR:
                c_compile_expression_token(writer, t);
                break;
            case TOK_VARIABLE:
                c_compile_variable(writer, t);
                break;
            case TOK_CONDITION:
                c_compile_condition(writer, t);
                break;
            case TOK_LOOP:
                c_compile_loop(writer, t);
                break;
            case TOK_FOR:
                c_compile_for(writer, t);
                break;
            case TOK_GOTO:
                c_compile_goto(writer, t);
                break;
            case TOK_LABEL:
                c_compile_label(writer, t);
                break;
            case TOK_RETURN:
                c_compile_return(writer, t);
                break;
            case TOK_FUNCTION:
                c_compile_function(writer, t);
                break;
            default:
                error(&t->trace, "token not allowed in c mode.");
                break;
        }
        t = t->next;
    }
}

void c_declare_variable(C_Writer* writer, Trace trace, char *name)
{
    fprintf(writer->body, "size_t %s;\n", name);
}

void c_compile_expression_unary(C_Writer *writer, FILE *file, Expression *expr)
{
    if (expr->value.operative.is_array_access)
    {
        fprintf(file, "((size_t*)");
        c_compile_expression(writer, file, expr->value.operative.x->value.operative.x);
        fprintf(file, ")[");
        c_compile_expression(writer, file, expr->value.operative.x->value.operative.y->value.operative.x);
        fprintf(file, "]");
    }
    else if (expr->value.operative.operation == OPER_DEREF)
    {
        fprintf(file, "*((size_t*)");
        c_compile_expression(writer, file, expr->value.operative.x);
        fprintf(file, ")");
    }
    else if (expr->value.operative.operation == OPER_ASSIGN_BY)
    {
        c_compile_expression(writer, file, expr->value.operative.x->value.operative.x);
        fprintf(file, " = ");
        c_compile_expression(writer, file, expr->value.operative.x);
    }
    else
    {
        fprintf(file, "%s", get_operator(expr->value.operative.operation));
        c_compile_expression(writer, file, expr->value.operative.x);
    }
}

void c_compile_expression_function(C_Writer *writer, FILE *file, Expression *expr)
{
    fprintf(file, "%s(", expr->value.function.name);
    Expression* arg = expr->value.function.arguments;
    while (arg)
    {
        c_compile_expression(writer, file, arg);
        if (arg->next)
        {
            fprintf(file, ", ");
        }
        arg = arg->next;
    }
    fprintf(file, ")");
}

void c_compile_expression_array_alloc(C_Writer *writer, FILE *file, Expression *expr)
{
    fprintf(writer->header, "size_t _array_%d[", writer->arr_count);
    c_compile_expression(writer, writer->header, expr->value.operative.x);
    fprintf(writer->header, "];\n");

    fprintf(file, "_array_%d", writer->arr_count++);
}

void c_compile_expression_array_literal(C_Writer *writer, FILE *file, Expression *expr)
{
    fprintf(writer->header, "size_t _array_%d[] = {", writer->arr_count);

    Expression *arg = expr->value.operative.x;
    while (arg)
    {
        c_compile_expression(writer, writer->header, arg);
        if (arg->next)
        {
            fprintf(writer->header, ", ");
        }
        arg = arg->next;
    }

    c_compile_expression(writer, writer->header, expr->value.operative.x);
    fprintf(writer->header, "};\n");

    fprintf(file, "_array_%d", writer->arr_count++);
}

void c_compile_expression(C_Writer *writer, FILE *file, Expression *expr)
{
    fprintf(file, "(");
    switch (expr->type)
    {
        case SYMBOLIC:
            fprintf(file, "%s", expr->value.symbol);
            break;
        case LITERAL:
            fprintf(file, "%d", expr->value.literal);
            break;
        case BINARY_OPERATION:
            c_compile_expression(writer, file, expr->value.operative.x);
            fprintf(file, " %s ", get_operator(expr->value.operative.operation));
            c_compile_expression(writer, file, expr->value.operative.y);
            break;
        case UNARY_OPERATION:
            c_compile_expression_unary(writer, file, expr);
            break;
        case TERNARY_CONDITIONAL:
            c_compile_expression(writer, file, expr->value.ternary.condition);
            fprintf(file, " ? ");
            c_compile_expression(writer, file, expr->value.ternary.a);
            fprintf(file, " : ");
            c_compile_expression(writer, file, expr->value.ternary.b);
            break;
        case FUNCTION_CALL:
            c_compile_expression_function(writer, file, expr);
            break;
        case STRING_LITERAL:
            fprintf(file, "\"%s\"", expr->value.symbol);
            break;
        case ARRAY_ALLOC:
            c_compile_expression_array_alloc(writer, file, expr);
            break;
        case ARRAY_LITERAL:
            c_compile_expression_array_literal(writer, file, expr);
            break;
    }
    fprintf(file, ")");

}

void c_compile_compound(C_Writer* writer, Token *token)
{
    fprintf(writer->body, "{\n");
    if (token->statements)
        c_compile(writer, token->statements);
    fprintf(writer->body, "}\n");
}

void c_compile_expression_token(C_Writer* writer, Token *token)
{
    c_compile_expression(writer, writer->body, token->expr);
    fprintf(writer->body, ";\n");
}

void c_compile_variable(C_Writer* writer, Token *token)
{
    StringList *current = token->ids;
    while (current)
    {
        c_declare_variable(writer, token->trace, current->str);
        current = current->next;
    }
    if (token->expr)
    {
        current = token->ids;
        while (current)
        {
            Expression sym = { token->trace, SYMBOLIC, {current->str}, NULL };
            Expression assign_symbol = { token->trace, BINARY_OPERATION };
            assign_symbol.value.operative.operation = OPER_ASSIGN;
            assign_symbol.value.operative.x = &sym;
            assign_symbol.value.operative.y = token->expr;
            c_compile_expression(writer, writer->body, &assign_symbol);
            fprintf(writer->body, ";\n");
            current = current->next;
        }
    }
}

void c_compile_condition(C_Writer* writer, Token *token)
{
    fprintf(writer->body, "if (");
    c_compile_expression(writer, writer->body, token->expr);
    fprintf(writer->body, ")\n");
    c_compile(writer, token->statements);
    if (token->statements_other)
    {
        fprintf(writer->body, "else\n");
        c_compile(writer, token->statements_other);
    }
}

void c_compile_loop(C_Writer* writer, Token *token)
{
    if (token->statements)
    {
        fprintf(writer->body, "while (");
        c_compile_expression(writer, writer->body, token->expr);
        fprintf(writer->body, ")\n");
        c_compile(writer, token->statements);
    }
    else
    {
        fprintf(writer->body, "do\n");
        c_compile(writer, token->statements_other);
        fprintf(writer->body, "while (");
        c_compile_expression(writer, writer->body, token->expr);
        fprintf(writer->body, ")\n");
    }
}

void c_compile_for(C_Writer* writer, Token *token)
{
    fprintf(writer->body, "{\n");
    c_compile(writer, token->statements);
    fprintf(writer->body, "while (");
    c_compile_expression(writer, writer->body, token->expr);
    fprintf(writer->body, ")\n{\n");
    c_compile(writer, token->statements_other);
    c_compile_expression(writer, writer->body, token->expr_other);
    fprintf(writer->body, ";\n}\n}\n");
}

void c_compile_goto(C_Writer* writer, Token *token)
{
    fprintf(writer->body, "goto %s\n", token->name);
}

void c_compile_label(C_Writer* writer, Token *token)
{
    fprintf(writer->body, "%s:\n", token->name);
}

void c_compile_return(C_Writer* writer, Token *token)
{
    fprintf(writer->body, "return ");
    if (token->expr)
        c_compile_expression(writer, writer->body, token->expr);
    fprintf(writer->body, ";\n");
}

void c_print_function_prototype(FILE *file, Token *token)
{
    fprintf(file, "size_t %s(", token->name);

    StringList *param = token->ids;
    while (param)
    {
        fprintf(file, "size_t %s", param->str);
        if (param->next)
        {
            fprintf(file, ", ");
        }
        param = param->next;
    }
    fprintf(file, ")");
}

void c_compile_function(C_Writer* writer, Token *token)
{
    c_print_function_prototype(writer->header, token);
    fprintf(writer->header, ";\n");

    c_print_function_prototype(writer->body, token);
    fprintf(writer->body, "\n");

    c_compile(writer, token->statements);
}

