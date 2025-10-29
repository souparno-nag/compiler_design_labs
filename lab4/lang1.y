%{
#include <stdio.h>
#include <stdlib.h>
int yylex();
void yyerror(const char *s);
int a_count = 0, b_count = 0;
%}
%token A B
%%
program:
program line
| /* empty */
;
line:
S '\n' {
if (a_count != b_count && a_count > 0 && b_count > 0) {
printf("ACCEPTED: a^%d b^%d (m != n)\n", a_count, b_count);
} else {
printf("REJECTED\n");
}
a_count = 0; b_count = 0;
}
| '\n' { a_count = 0; b_count = 0; }
| error '\n' {
printf("REJECTED\n");
a_count = 0; b_count = 0;
yyerrok;
}
;
S:
alist blist
;
alist:
alist A { a_count++; }
| A { a_count++; }
;
blist:
blist B { b_count++; }
| B { b_count++; }
;
%%
void yyerror(const char *s) {
/* Error handled in line rule */
}
int main() {
printf("Language: { a^n b^m / m != n }\n");
printf("Enter strings (Ctrl+D to exit):\n");
yyparse();
return 0;
}
