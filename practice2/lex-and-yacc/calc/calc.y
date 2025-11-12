%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int yylex(); void yyerror(char* s) {};
%}

%union { int n;}
%token <n> NUM;
%type <n> e t f
%left '+' '-'
%left '*' '/' '%'

%%
p :
/* empty */
| p e '\n'	{printf("%d\n", $2);}
;

e :
t	{$$ = $1;}
| e'+'t		{$$=$1+$3;}
| e'-'t		{$$=$1-$3;}
;

t :
f	{$$ = $1;}
| t'*'f         {$$=$1*$3;}
| t'/'f         {$$=$1/$3;}
| t'%'f		{$$=$1%$3;}
;

f :
NUM {$$ = $1;}
;
%%

int main() {
yyparse();
}


