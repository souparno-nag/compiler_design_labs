%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int yylex();
void yyerror(char* s) {};
int count1 = 0, count2 = 0;
%}

%token A B

%%
p :
/* empty */
| p line 
;

line :
S '\n'	{ if (count1 == count2) {
		printf("Accepted\n");
	} else {
		printf("Rejected\n");
	}
	count1 = 0; count2 = 0;
	}
| '\n'	{ count1 = 0; count2 = 0; }
| error '\n'	{ printf("Rejected\n"); count1 = 0; count2 = 0;
		yyerror;
		}
;

S : A B X B B A Y;

X : X B B A A {count1++;} | B B A A {count1++;};
Y : Y B A {count2++;} | B A {count2++;};
%%

int main() {
yyparse();
}
