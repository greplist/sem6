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

%token <val> BOOL INT UNKNOWN_ID BOOL_ID INT_ID FUNC_ID
%token OR AND NOT
%token IF ELSE WHILE DEF
%left '-' '+' OR
%left '*' '/' AND
%nonassoc UMINUS NOT

%type <val> int_expr bool_expr while_stmt if_stmt stmt_list stmt block
%type <val> id id_list param params_list func_stmt func_call
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

stmt: id '=' int_expr '\n'				{
	Variable * var = (dynamic_cast<Variable *>($1));
	var->type = INT_ID;
	var->val = (dynamic_cast<Node *>($3))->val;
	$$ = new Node("=", $1, $3); 
}
	| id '=' bool_expr '\n'				{ (dynamic_cast<Variable *>($1))->type = BOOL_ID; $$ = new Node("=", $1, $3); }
	| while_stmt 						{ $$ = $1; }
	| if_stmt 							{ $$ = $1; }
	| func_call							{ $$ = $1; }
	| func_stmt							{ $$ = $1; }
	| '\n'								{ $$ = new Object<std::string>(""); }
	;

func_call: FUNC_ID '(' params_list ')'		{
	Variable *f = dynamic_cast<Variable *>($1);
	MultiNode *p = dynamic_cast<MultiNode *>($3);
	if ( f->val != p->Len() ) {
		yyerror("func call with invalid number of params");
		exit(-1);
	}

	$$ = new Node("CALL", $1, $3);
}

params_list: params_list ',' param 		{ (dynamic_cast<MultiNode *>($1))->Add($3); $$ = $1; }
	| param								{
	MultiNode *node = new MultiNode();
	node->Add($1);
	$$ = node;
}

param: int_expr						{ $$ = $1; }
	| bool_expr 					{ $$ = $1; }
	| FUNC_ID						{ $$ = $1; }
	;

func_stmt: DEF UNKNOWN_ID '(' id_list ')' block {
	Variable *f = dynamic_cast<Variable *>($2);
	MultiNode *p = dynamic_cast<MultiNode *>($4);
	f->type = FUNC_ID;
	f->val = p->Len();

	MultiNode *node = new MultiNode();
	node->Add(new Node("DEF ", $2, NULL));
	node->Add(new Node("(", $4, NULL));
	node->Add(new Node("): ", $6, NULL));
	$$ = node;
}

id_list: id_list ',' id					{ (dynamic_cast<MultiNode *>($1))->Add($3); $$ = $1; }
	| id								{
	MultiNode *node = new MultiNode();
	node->Add($1);
	$$ = node;
}

id : UNKNOWN_ID							{ $$ = $1; }
	| BOOL_ID							{ $$ = $1; }
	| INT_ID							{ $$ = $1; }
	| FUNC_ID							{ $$ = $1; }
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

int_expr: int_expr '+' int_expr		{
	int first = (dynamic_cast<Node *>($1))->val, second = (dynamic_cast<Node *>($3))->val;
	Node *node = new Node("+", $1, $3);
	node->val = first + second;
}
	| int_expr '-' int_expr			{
	int first = (dynamic_cast<Node *>($1))->val, second = (dynamic_cast<Node *>($3))->val;
	Node *node = new Node("-", $1, $3);
	node->val = first - second;
	$$ = node;
}
	| int_expr '*' int_expr			{
	int first = (dynamic_cast<Node *>($1))->val, second = (dynamic_cast<Node *>($3))->val;
	Node *node = new Node("*", $1, $3);
	node->val = first * second;
	$$ = node;
}
	| int_expr '/' int_expr			{
	int first = (dynamic_cast<Node *>($1))->val, second = (dynamic_cast<Node *>($3))->val;
	if (second == 0) {
		yyerror("divison by zero");
		exit(-1);
	}
	Node *node = new Node("/", $1, $3);
	node->val = first / second;
	$$ = node;
}
	| '-' int_expr %prec UMINUS 	{
	Node *node = new Node("-", $2, NULL);
	node->val = - (dynamic_cast<Node *>($2))->val;
	$$ = node;
}
	| '(' int_expr ')'				{ $$ = $2; }
	| INT_ID						{
	Node *node = new Node("Int Variable", $1, NULL);
	node->val = (dynamic_cast<Variable *>($1))->val;
	$$ = node;
}
	| INT 							{
	Node *node = new Node("Int Const", $1, NULL);
	node->val = (dynamic_cast<Object<int> *>($1))->val;
	$$ = node;
}
	;

bool_expr: bool_expr OR bool_expr	{ $$ = new Node("OR", $1, $3); }
	| bool_expr AND bool_expr		{ $$ = new Node("AND", $1, $3); }
	| NOT bool_expr					{ $$ = new Node("NOT", $2, NULL); }
	| '(' bool_expr ')'				{ $$ = $2; }
	| int_expr '<' int_expr			{ $$ = new Node("<", $1, $3); }
	| int_expr '>' int_expr			{ $$ = new Node(">", $1, $3); }
	| int_expr '=' '=' int_expr		{ $$ = new Node("==", $1, $4); }
	| int_expr '>' '=' int_expr		{ $$ = new Node(">=", $1, $4); }
	| int_expr '<' '=' int_expr		{ $$ = new Node("<=", $1, $4); }
	| int_expr '!' '=' int_expr		{ $$ = new Node("!=", $1, $4); }
	| BOOL_ID						{ $$ = $1; }
	| BOOL							{ $$ = $1; }
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
