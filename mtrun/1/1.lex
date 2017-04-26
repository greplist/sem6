%{
#include <stdio.h>

unsigned global_level = 0, nline = 1, nsymbol = 1;

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

\n                      { ++nline; nsymbol = 0; printf("\n"); }
:/\n                    { ++nsymbol; printf("(:) "); }
\n[ ]*/[^ ]             {
    ++nline;
    nsymbol = yyleng - 1;
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
                printf("\n\nError: Line %d:%d  invalid number of tokens: delta: %d\n",
                    nline, nsymbol, delta);
                exit(-1);
            }
            for (int i = 0, n = -delta / 4; i < n; i++)
                printf("(BLOCK, END)\n");
    }

    global_level = curr_level;
}

{boolean}            { nsymbol += yyleng; printf("(BOOL, %s) ", yytext); }
{integer}            { nsymbol += yyleng; printf("(INTEGER, %s) ", yytext); }
{double}             { nsymbol += yyleng; printf("(DOUBLE, %s) ", yytext); }

[,]                  { nsymbol += yyleng; printf("(COMMA) "); }
{bracket}            { nsymbol += yyleng; printf("(BRASKET, '%s') ", yytext); }
{binop}              { nsymbol += yyleng; printf("(BINOP, %s) ", yytext); }
{condop}             { nsymbol += yyleng; printf("(CONDOP, %s) ", yytext); }
{relop}              { nsymbol += yyleng; printf("(RELOP, %s) ", yytext); }
{asop}               { nsymbol += yyleng; printf("(ASSIGMENT, %s) ", yytext); }


{keyword}            { nsymbol += yyleng; printf("(KEYWORD, %s) ", yytext); }

{id}                 { nsymbol += yyleng; printf("(ID, '%s') ", yytext); }

[ ]+                 { nsymbol += yyleng; }
.                    {
    nsymbol += yyleng;
    printf("\n\nError: Line %d:%d invalid lexem: %s\n", nline, nsymbol, yytext);
    exit(-1);
}

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
