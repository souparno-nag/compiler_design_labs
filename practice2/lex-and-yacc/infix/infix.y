%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int yylex();
void yyerror(char* s) {};
%}

%union {char c;}
%token <c> ID;
%type <c> e;
%left '+' '-'
%left '*' '/'

%%
p :
/* empty */
| p e '\n' {printf("\n");}
;

e :
ID	{printf("%c", $1); $$ = $1;}
| e'+'e		{printf("+"); $$='+';}
| e'-'e         {printf("-"); $$='-';}
| e'*'e         {printf("*"); $$='*';}
| e'/'e         {printf("/"); $$='/';}
| '(' e ')'	{$$=$2;}
;
%%


int main() {
yyparse();
}
