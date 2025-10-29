%{
#include <stdio.h>
#include <stdlib.h>
int yylex();
void yyerror(const char *s);
%}
%union {
char c;
}
%token <c> ID
%type <c> expr
%left '+' '-'
%left '*' '/'
%%
program:
program expr '\n' { printf("\n"); }
| /* empty */
;
expr:
ID { printf("%c", $1); $$ = $1; }
| expr '+' expr { printf("+"); $$ = '+'; }
| expr '-' expr { printf("-"); $$ = '-'; }
| expr '*' expr { printf("*"); $$ = '*'; }
| expr '/' expr { printf("/"); $$ = '/'; }
| '(' expr ')' { $$ = $2; }
;
%%
void yyerror(const char *s) {
fprintf(stderr, "Error: %s\n", s);
}
int main() {
printf("Infix to Postfix Converter\n");
printf("Enter infix expressions (Ctrl+D to exit):\n");
yyparse();
return 0;
}