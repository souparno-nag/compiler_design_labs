%{
#include <stdio.h>
#include <stdlib.h>
int yylex();
void yyerror(const char *s);
int n_value = 0;
%}
%token A B
%%
program:
program line
| /* empty */
;
line:
S '\n' {
printf("ACCEPTED (n=%d)\n", n_value);
n_value = 0;
}
| '\n' {
n_value = 0;
}
| error '\n' {
printf("REJECTED (syntax error)\n");
n_value = 0;
yyerrok;
}
;
S:
A B X Y
;
X:
X B B A A { n_value++; }
| B B /* base case: bba part */
;
Y:
Y B A {
n_value--;
if (n_value < 0) {
yyerror("Too many BA pairs");
YYERROR;
}
}
| A /* base case: final 'a' */
;
%%
void yyerror(const char *s) {
/* Error handled in line rule */
}
int main() {
printf("Language: { ab(bbaa)^n bba(ba)^n / n >= 0 }\n");
printf("Enter strings (Ctrl+D to exit):\n");
yyparse();
return 0;
}