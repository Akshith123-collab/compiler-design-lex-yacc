%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
%}

%option noyywrap

DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
NUMBER      {DIGIT}+

%%

"while"         { return WHILE; }
"if"            { return IF; }
"else"          { return ELSE; }

"=="            { yylval.text = strdup(yytext); return RELOP; }
"!="            { yylval.text = strdup(yytext); return RELOP; }
"<="            { yylval.text = strdup(yytext); return RELOP; }
">="            { yylval.text = strdup(yytext); return RELOP; }
"<"             { yylval.text = strdup(yytext); return RELOP; }
">"             { yylval.text = strdup(yytext); return RELOP; }

"="             { return '='; }

"+"             { return '+'; }
"-"             { return '-'; }
"*"             { return '*'; }
"/"             { return '/'; }

"("             { return '('; }
")"             { return ')'; }
"{"             { return '{'; }
"}"             { return '}'; }
";"             { return ';'; }

{NUMBER}        {
                    yylval.text = strdup(yytext);
                    return NUMBER;
                }

{ID}            {
                    yylval.text = strdup(yytext);
                    return ID;
                }

[ \t\r\n]+      { /* Ignore whitespace */ }

"//".*          { /* Ignore single-line comments */ }

.               {
                    fprintf(stderr,
                            "Lexical error: unexpected character '%s'\n",
                            yytext);
                    exit(EXIT_FAILURE);
                }

%%
