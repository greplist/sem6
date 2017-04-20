%{
#include <stdio.h>

unsigned global_level = 0, global_line = 1;

%}

letter       [a-zA-Z_]
digit        [0-9]
token        [ ]
bracket      [\(\)]

boolean      True|False
integer      -?{digit}+
double       {integer}\.{digit}+

binop        [+\-*/%\|\^]
condop       and|or
relop        [<>!]=?
asop         {binop}?=

keyword      break|case|continue|def|default|else|for|if|return|switch|while|print

id           {letter}({letter}|{digit})*

%%

\n                      { ++global_line; printf("\n"); }
:/\n                    { printf("(:) "); }
\n[ ]*/[^ ]             {
    ++global_line;
    printf("\n");

    unsigned curr_level = yyleng - 1;

    int delta = curr_level - global_level;
    switch (delta) {
        case 0:
            break;
        case 4:
            printf("(BLOCK, START)\n");
            break;
        default:
            if ( delta > 0 || delta % 4 != 0 ) {
                printf("\nError: Line %d:%d  invalid number of tokens: delta: %d\n",
                    global_line, yyleng, delta);
                exit(-1);
            }
            for (int i = 0, n = -delta / 4; i < n; i++)
                printf("(BLOCK, END)\n");
    }

    global_level = curr_level;
}

{boolean}            { printf("(BOOL, %s) ", yytext); }
{integer}            { printf("(INTEGER, %s) ", yytext); }
{double}             { printf("(DOUBLE, %s) ", yytext); }

[,]                  { printf("(COMMA) "); }
{bracket}            { printf("(BRASKET, '%s') ", yytext); }
{binop}              { printf("(BINOP, %s) ", yytext); }
{condop}             { printf("(CONDOP, %s) ", yytext); }
{relop}              { printf("(RELOP, %s)", yytext); }
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
