%{
/*
    syntax analyser
*/

%}

letter   [a-zA-Z_]
digit    [0-9]
token    [ \t]

boolean  TRUE | FALSE
integer  -?{digit}+
double   {integer}()\.{digit}+

operator < | <= | > | >= | == | !=

name    {letter}({letter}|{digit})+

%%

{boolean}            { return BOOL; }
{integer}            { return INT; }
{double}             { return DOUBLE; }

{token}=/{token}     { return ASSIGMENT; }
{operator}           { return OPERATOR; }

if                   {  return IF; }
else                 {  return ELSE; }
while                {  return WHILE; }
for                  {  return FOR; }
def                  {  return DEF; }
:\n                  {  return TOKEN; }

{name}               { return NAME; }

[ \t\n]+             {}
%%

int yywrap() {
    return 1;
}
