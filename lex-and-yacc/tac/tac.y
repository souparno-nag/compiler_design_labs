%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int yylex();
void yyerror(char* s) {};
char* nt() {
static int t; char s[9];
sprintf(s, "t%d", ++t); 
return strdup(s);
}
%}

%union { int n; char* s;}
%token<n> NUM;
%token<s> ID;
%type<s> e t f
%left '+' '-'
%left '*' '/' 

%%
p : 
/* nothing */
| p e '\n'	{printf("\n");}
| p '\n'      {printf("\n");}
;

e :
t	{$$ = $1;}
| e'+'t	{$$=nt(); printf("%s=%s+%s",$$,$1,$3);}
| e'-'t {$$=nt(); printf("%s=%s-%s",$$,$1,$3);}
;

t :
f	{$$ = $1;}
| t'*'f {$$=nt(); printf("%s=%s*%s",$$,$1,$3);}
| t'/'f {$$=nt(); printf("%s=%s/%s",$$,$1,$3);}
;

f : 
NUM	{char s[9]; sprintf(s, "%d", $1); $$=strdup(s);}
|ID	{$$=$1;}
|'('e')'	{$$=$2;}
|ID'='e	{printf("%s=%s",$1,$3); $$=$3;}
;

%%

int main() {
yyparse();
}
