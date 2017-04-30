%{
#include <stdio.h>
#include <math.h>

#include "tree.h"
#include "y.tab.h"

unsigned global_level = 0, nline = 1;

Block block;

%}

letter       [a-zA-Z_]
digit        [0-9]
token        [ ]

boolean      True|False
integer      -?{digit}+
double       {integer}\.{digit}+

one_char     "+"|"-"|"*"|"/"|"!"|"="|"<"|">"|"("|")"|":"|";"

keyword      break|case|continue|def|default|else|for|if|return|switch|while|print

id           {letter}({letter}|{digit})*

%%

{boolean}		{
	bool val(false);
	if ( yytext[0] == 'T' ) {
		val = true;
	}
	yylval.val = new Object<bool>(val);
	return BOOL;
}

{integer}		{
	yylval.val = new Object<int>(atoi(yytext));
	return INT;
}

or			{ return OR; }
and			{ return AND; }
not			{ return NOT; }

if 			{ return IF; }
while			{ return WHILE; }
else			{ return ELSE; }

\n			{ nline++; return '\n'; }
{one_char}        	{ return yytext[0]; }

{id}			{
	Variable *v = block.GetOrCreate(std::string(yytext, yyleng));
	yylval.val = v;
	return v->type;
}

[ ]+			;
.		  	{ fprintf(stderr, "Lexical error: Line %d: invalid lexem %s (ASCII: %c)\n", nline, yytext, yytext[0]); exit(-1); }

%%

int yywrap() {
	return 1;
}
