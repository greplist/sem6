%{
/*
    syntax analyser
*/
unsigned nword = 0, nchar = 0, nline = 0;
%}

%option noyywrap

word [^ \t\n]+
eol \n

%%
{word}      { nword++; nchar += yyleng; }
{eol}       { nchar++; nline++; }
.           { nchar++; }
%%

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
    printf("%d, %d, %d\n", nline, nword, nchar);
    return 0;
}
