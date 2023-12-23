
all:
	flex lexer.l
	bison -d parser.y
	gcc -g -Wall -o bspdr main.c expression.c assembly.c resolve.c compile.c \
		exprvalue.c context.c lex.yy.c parser.tab.c token.c trace.c error.c \
		stringliteral.c external.c c_compile.c

install:
	cp bspdr ~/.local/bin/bspdr
	cp bsc ~/.local/bin/bsc

