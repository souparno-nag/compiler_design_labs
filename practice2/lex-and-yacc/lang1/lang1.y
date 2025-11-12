%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int yylex();
void yyerror(char* s) {};
int count_a = 0, count_b = 0;
%}

%token A B

%%
p :
/* empty */ 
| p line	{;}
;

line:
S '\n'	{ if (count_a == count_b) {
		printf("Rejected\n");
	} else {
		printf("Accepted\n");
	}
	count_a = 0; count_b = 0;
	}
;
S: a_list b_list;

a_list : a_list A	{count_a++;} | A	{count_a++;};
b_list : b_list B	{count_b++;} | B	{count_b++;};

%%

int main() {
yyparse();
}
