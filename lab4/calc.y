%{
#include <stdio.h>
#include <stdlib.h>
int yylex();
void yyerror(const char *s);
%}
%token NUMBER
%left '+' '-'
%left '*' '/'
%%
program:
program expr '\n' { printf("Result: %d\n", $2); }
| /* empty */
;
expr:
NUMBER { $$ = $1; }
| expr '+' expr { $$ = $1 + $3; }
| expr '-' expr { $$ = $1 - $3; }
| expr '*' expr { $$ = $1 * $3; }
| expr '/' expr {
if ($3 == 0) {
yyerror("Division by zero");
$$ = 0;
} else {
$$ = $1 / $3;
}
}
| '(' expr ')' { $$ = $2; }
;
%%
void yyerror(const char *s) {
fprintf(stderr, "Error: %s\n", s);
}
int main() {
printf("Enter expressions)\n");
yyparse();
return 0;
}