%{
/*
    syntax yacc
*/
#include <stdio.h>
%}

%token BOOL INT DOUBLE OPERATOR ASSIGMENT IF ELSE WHILE FOR DEF NAME TOKEN

%%

while: WHILE number OPERATOR number { printf("found while\n"); }

number: INT
    |   DOUBLE
    |   NAME
    ;

%%

extern FILE *yyin;

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "could not open %s\n", argv[1]);
            exit(1);
        }
        yyin = file;
    }
    while (!feof(yyin)) {
        yyparse();
    }
    return 0;
}

void yyerror(char *s) {
    fprintf(stderr, "%s\n", s);
}
