#ifndef C_COMPILE_H
#define C_COMPILE_H

#include "expression.h"
#include "context.h"
#include "token.h"
#include "c_writer.h"

void c_compile_program(Token *token);
void c_compile(C_Writer* writer, Token *token);

void c_declare_variable(C_Writer* writer, Trace trace, char *name);
void c_compile_expression(C_Writer* writer, FILE *file, Expression *expr);
void c_compile_compound(C_Writer* writer, Token *token);
void c_compile_expression_token(C_Writer* writer, Token *token);
void c_compile_variable(C_Writer* writer, Token *token);
void c_compile_condition(C_Writer* writer, Token *token);
void c_compile_loop(C_Writer* writer, Token *token);
void c_compile_for(C_Writer* writer, Token *token);
void c_compile_goto(C_Writer* writer, Token *token);
void c_compile_label(C_Writer* writer, Token *token);
void c_compile_return(C_Writer* writer, Token *token);
void c_compile_function(C_Writer* writer, Token *token);

#endif

