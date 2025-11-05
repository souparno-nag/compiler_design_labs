%{
#include <stdio.h>
#include <stdlib.h>
int yylex(void);
int yyerror(const char *s);
%}

%token NUMBER
%left '+' '-'
%left '*' '/'
%left UMINUS

%%
input:
      /* empty */
    | input expr '\n'   { printf("= %d\n", $2); }
;

expr:
      expr '+' expr     { $$ = $1 + $3; }
    | expr '-' expr     { $$ = $1 - $3; }
    | expr '*' expr     { $$ = $1 * $3; }
    | expr '/' expr     { $$ = $1 / $3; }
    | '-' expr %prec UMINUS  { $$ = -$2; }
    | '(' expr ')'      { $$ = $2; }
    | NUMBER
;
%%

int yyerror(const char *s) {
    printf("Syntax error\n");
    return 0;
}

int main() {
    return yyparse();
}