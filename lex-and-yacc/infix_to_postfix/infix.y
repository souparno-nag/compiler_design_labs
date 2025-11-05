%{
#include <stdio.h>
#include <stdlib.h>
int yylex();
void yyerror (const char *s);
%}


%union {
char c;
}

%token <c> ID;
%type <c> expr;

%%
program:
/* empty */
| program expr '\n' {printf("\n");}
;

expr:
ID	{printf("%c",$1); $$ = $1;}
| expr '+' expr {printf("+"); $$ = '+';}
| expr '-' expr {printf("-"); $$ = '-';}
| expr '*' expr {printf("*"); $$ = '*';}
| expr '/' expr {printf("/"); $$ = '/';}
| '(' expr ')' {$$ = $2;}
;
%%

void yyerror (const char *s) {
fprintf(stderr, "Error: %s\n", s);
}

int main() {
printf("Infix to postfix conversion: (press Ctrl+D to exit)\n");
yyparse();
return 0;
}
