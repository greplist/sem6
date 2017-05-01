%{
#include <stdio.h>
#include <math.h>

#include "tree.h"
#include "y.tab.h"

unsigned global_level = 0, nline = 1, nchar = 0;

Block block;

%}

letter       [a-zA-Z_]
digit        [0-9]
token        [ ]

boolean      True|False
integer      -?{digit}+

one_char     "+"|"-"|"*"|"/"|"!"|"="|"<"|">"|"("|")"|":"|";"|","

keyword      break|case|continue|def|default|else|for|if|return|switch|while|print

id           {letter}({letter}|{digit})*

%%

{boolean}			{
	nchar += yyleng;

	bool val(false);
	if ( yytext[0] == 'T' ) {
		val = true;
	}
	yylval.val = new Object<bool>(val);
	return BOOL;
}

{integer}			{
	nchar += yyleng;

	yylval.val = new Object<int>(atoi(yytext));
	return INT;
}

or					{ nchar += yyleng; return OR; }
and					{ nchar += yyleng; return AND; }
not					{ nchar += yyleng; return NOT; }

if 					{ nchar += yyleng; return IF; }
while				{ nchar += yyleng; return WHILE; }
else				{ nchar += yyleng; return ELSE; }
def					{ nchar += yyleng; return DEF; }

\n					{ nchar = 0; nline++; return '\n'; }
{one_char}        	{ nchar += yyleng; return yytext[0]; }

{id}				{
	nchar += yyleng;

	Variable *v = block.GetOrCreate(std::string(yytext, yyleng));
	yylval.val = v;
	return v->type;
}

[ ]+				{ nchar += yyleng; }
.		  			{
	nchar += yyleng;

	fprintf(stderr, "Lexical error: Line %d:%d invalid lexem %s (ASCII: %c)\n", nline, nchar, yytext, yytext[0]);
	exit(-1);
}

%%

int yywrap() {
	return 1;
}
