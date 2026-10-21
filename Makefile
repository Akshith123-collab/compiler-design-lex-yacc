CC = gcc
CFLAGS = -Wall -Wextra -O2

.PHONY: all demo clean

all: compiler

parser.tab.c parser.tab.h &: parser.y backend.h
	bison -d parser.y

lex.yy.c: lexer.l parser.tab.h
	flex lexer.l

compiler: parser.tab.c lex.yy.c backend.c backend.h
	$(CC) $(CFLAGS) -o $@ parser.tab.c lex.yy.c backend.c

demo: compiler
	@echo '=== Source program: input.c ==='
	@cat input.c
	@printf '\n=== Compiler output ===\n'
	./compiler < input.c

clean:
	rm -f compiler parser.tab.c parser.tab.h lex.yy.c parser.output
