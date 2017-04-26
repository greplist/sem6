%{
#include <stdio.h>

#include "tree.h"
#include "lex.yy.h"

extern Block block;
extern unsigned nline;

void yyerror(const char* s);
%}

%union {
	IPrintable *val;
}

%token <val> BOOL INT ID
%token OR AND NOT
%token IF ELSE WHILE DEF
%left '-' '+' OR
%left '*' '/' AND
%nonassoc UMINUS NOT

%type <val> int_expr bool_expr while_stmt if_stmt stmt_list stmt block
%%

code: stmt_list '\n' 			{ $1->Print(0); }
	;

stmt_list: stmt_list stmt		{ (dynamic_cast<MultiNode *>($1))->Add($2); $$ = $1; }
	| stmt				{
	MultiNode *node = new MultiNode();
	node->Add($1);
	$$ = node;
}
	;

stmt: ID '=' int_expr '\n'		{ $$ = new Node("=", $1, $3); }
	| ID '=' bool_expr '\n'		{ $$ = new Node("=", $1, $3); }
	| while_stmt '\n'		{ $$ = $1; }
	| if_stmt '\n'			{ $$ = $1; }
	| '\n'				{ $$ = new Object<std::string>(""); }
	;

while_stmt: WHILE bool_expr block {
	MultiNode *node = new MultiNode();
	node->Add(new Node("WHILE {", $2, NULL));
	node->Add(new Node(") DO {", $3, NULL));
	node->Add(new Object<std::string>("}"));
	$$ = node;
}

if_stmt: IF bool_expr block {
	MultiNode *node = new MultiNode();
	node->Add(new Node("IF (", $2, NULL));
	node->Add(new Node(") THEN (", $3, NULL));
	node->Add(new Object<std::string>(")"));
	$$ = node;
}
	| IF bool_expr block ELSE block {
	MultiNode *node = new MultiNode();
	node->Add(new Node("IF (", $2, NULL));
	node->Add(new Node(") THEN (", $3, NULL));
	node->Add(new Node(") ELSE (", $5, NULL));
	node->Add(new Object<std::string>(")"));
	$$ = node;
}
	;

block: ':' '\n' stmt_list ';' 		{ $$ = $3; }

int_expr: int_expr '+' int_expr		{ $$ = new Node("+", $1, $3); }
	| int_expr '-' int_expr		{ $$ = new Node("-", $1, $3); }
	| int_expr '*' int_expr		{ $$ = new Node("*", $1, $3); }
	| int_expr '/' int_expr		{ $$ = new Node("/", $1, $3); }
	| '-' int_expr %prec UMINUS 	{ $$ = new Node("-", $2, NULL); }
	| '(' int_expr ')'		{ $$ = $2; }
	| INT 				{ $$ = $1; }
	| ID				{ $$ = $1; }
	;

bool_expr: bool_expr OR bool_expr	{ $$ = new Node("OR", $1, $3); }
	| bool_expr AND bool_expr	{ $$ = new Node("AND", $1, $3); }
	| NOT bool_expr			{ $$ = new Node("NOT", $2, NULL); }
	| '(' bool_expr ')'		{ $$ = $2; }
	| int_expr '<' int_expr		{ $$ = new Node("<", $1, $3); }
	| int_expr '>' int_expr		{ $$ = new Node(">", $1, $3); }
	| int_expr '=' '=' int_expr	{ $$ = new Node("==", $1, $4); }
	| int_expr '>' '=' int_expr	{ $$ = new Node(">=", $1, $4); }
	| int_expr '<' '=' int_expr	{ $$ = new Node("<=", $1, $4); }
	| int_expr '!' '=' int_expr	{ $$ = new Node("!=", $1, $4); }
	| BOOL				{ $$ = $1; }
	;

%%

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
    	    fprintf(stderr, "Error: cannot open file: %s\n", argv[1]);
            exit(1);
        }
        yyin = file;
    }
    while (!feof(yyin)) {
        yyparse();
    }
    return 0;
}

void yyerror(const char* s) {
    fprintf(stderr, "Error: Line %d: %s\n", nline, s);
}
