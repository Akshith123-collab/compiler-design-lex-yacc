%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "backend.h"

int yylex(void);
void yyerror(const char *s);

static char *make_string(const char *a,
                         const char *op,
                         const char *b)
{
    size_t n = strlen(a) + strlen(op) + strlen(b) + 4;
    char *result = malloc(n);

    if (!result) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    snprintf(result, n, "%s %s %s", a, op, b);
    return result;
}
%}

%union {
    char *text;
}

%token WHILE IF ELSE
%token <text> ID NUMBER RELOP

%type <text> expr condition loop_start if_prefix

%left '+' '-'
%left '*' '/'
%nonassoc IFX
%nonassoc ELSE

%start program

%%

program
    : stmt_list
      {
          printf("\nSyntax is CORRECT.\n");
      }
    ;

stmt_list
    : /* empty */
    | stmt_list stmt
    ;

stmt
    : assignment ';'
    | block
    | while_stmt
    | if_stmt
    ;

block
    : '{' stmt_list '}'
    ;

assignment
    : ID '=' expr
      {
          emit("%s = %s", $1, $3);
      }
    ;

loop_start
    : WHILE
      {
          $$ = newlabel();
          emit_label($$);
      }
    ;

while_stmt
    : loop_start '(' condition ')'
      {
          char *end = newlabel();

          emit("ifFalse %s goto %s", $3, end);
          $<text>$ = end;
      }
      stmt
      {
          emit("goto %s", $1);
          emit_label($<text>5);
      }
    ;

if_prefix
    : IF '(' condition ')'
      {
          $$ = newlabel();
          emit("ifFalse %s goto %s", $3, $$);
      }
    ;

if_stmt
    : if_prefix stmt %prec IFX
      {
          emit_label($1);
      }

    | if_prefix stmt ELSE
      {
          /*
           * The ELSE branch must be skipped after the THEN
           * branch finishes.
           */
          char *end = newlabel();

          emit("goto %s", end);
          emit_label($1);

          $<text>$ = end;
      }
      stmt
      {
          emit_label($<text>4);
      }
    ;

condition
    : expr RELOP expr
      {
          $$ = make_string($1, $2, $3);
      }
    ;

expr
    : expr '+' expr
      {
          $$ = newtemp();
          emit("%s = %s + %s", $$, $1, $3);
      }
    | expr '-' expr
      {
          $$ = newtemp();
          emit("%s = %s - %s", $$, $1, $3);
      }
    | expr '*' expr
      {
          $$ = newtemp();
          emit("%s = %s * %s", $$, $1, $3);
      }
    | expr '/' expr
      {
          $$ = newtemp();
          emit("%s = %s / %s", $$, $1, $3);
      }
    | '(' expr ')'
      {
          $$ = $2;
      }
    | ID
      {
          $$ = $1;
      }
    | NUMBER
      {
          $$ = $1;
      }
    ;

%%

void yyerror(const char *s)
{
    fprintf(stderr, "Syntax error: %s\n", s);
}

int main(void)
{
    printf("COMPILER FRONT END AND BACK END\n");
    printf("Enter the source program. Press Ctrl+D when finished.\n\n");

    if (yyparse() == 0) {
        print_tac();
        optimize_code();
        generate_target_code();
    } else {
        fprintf(stderr, "Compilation failed.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

