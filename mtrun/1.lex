%{
#include <stdio.h>

unsigned global_level = 0, global_line = 0;

%}

letter       [a-zA-Z_]
digit        [0-9]
token        [ ]
bracket      [\(\)]

boolean      True | False
integer      -?{digit}+
double       {integer}\.{digit}+

binop        [+\-*/%\|\^]
condop       && | \|\|
relop        [<>!]=?
asop         {binop}?=

keyword      break|case|continue|def|default|else|for|if|return|switch|while|print

id           {letter}({letter}|{digit})*

%%

\n                      { global_line++; printf("\n"); }
:/\n                    { printf("(:) "); }
\n[ ]+/[^ ]             {
    global_line++;

    unsigned curr_level = yyleng - 1;

    unsigned delta = global_level - curr_level;
    switch (delta) {
        case 0:
            printf("\n");
            break;
        case -4:
            printf("\n(TOKEN, START)\n");
            break;
        case 4:
            printf("\n(TOKEN, END)\n");
            break;
        default:
            printf("\nError: Line %d:%d  invalid number of tokens: delta: %d\n",
                global_line, yyleng, delta);
            exit(-1);
    }

    global_level = curr_level;
}

{boolean}            { printf("(BOOL, %s) ", yytext); }
{integer}            { printf("(INTEGER, %s) ", yytext); }
{double}             { printf("(DOUBLE, %s) ", yytext); }

[,]                  { printf("(COMMA) "); }
{bracket}            { printf("(BRASKET, '%s') ", yytext); }
{asop}               { printf("(ASSIGMENT, %s) ", yytext); }

{keyword}            { printf("(KEYWORD, %s) ", yytext); }

{id}                 { printf("(ID, '%s') ", yytext); }

%%

int yywrap() {
    return 1;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "could not open %s\n", argv[1]);
            exit(1);
        }
        yyin = file;
    }
    yylex();
    return 0;
}
