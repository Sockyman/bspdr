
all:
	flex lexer.l
	bison -d parser.y
	gcc -g -Wall -o bspdr main.c expression.c assembly.c resolve.c \
		exprvalue.c context.c lex.yy.c parser.tab.c token.c trace.c

