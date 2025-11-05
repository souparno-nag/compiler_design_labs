%{
#include <stdlib.h>
#include <stdio.h>
int yylex();
void yyerror (const char *s);
int a_count = 0, b_count = 0;
%}

%token A B

%%
program:
/* empty */
| program line 
;

line: S '\n' {
		if (a_count != b_count && a_count > 0 && b_count > 0) {
			printf("Accepted: a^%d b^%d (m != n)\n", a_count, b_count);
		} else {
			printf("Rejected\n");
		}
		a_count = 0; b_count = 0;
	}
| '\n' { a_count = 0; b_count = 0; }
| error '\n' { printf("Rejected\n");
		a_count = 0; b_count = 0;
		yyerror;
}

S: a_list b_list;
a_list: a_list A {a_count++;} | A {a_count++;};
b_list: b_list B {b_count++;} | B {b_count++;};

%%

void yyerror (const char *s) {}

int main() {
printf("Enter the expression: (press Ctrl+D to exit)\n");
yyparse();
return 0;
}
