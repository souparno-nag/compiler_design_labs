%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

char* keywords[] = {"int", "main", "return"};
bool isKeyword(char* s) {
for (int i = 0; i < 3; i++) {
if (strcmp(s, keywords[i]) == 0) return true;
}
return false;
}
%}

%option noyywrap

%%
[a-zA-Z][a-zA-Z0-9]*	{if (isKeyword(yytext)) {
				printf("Keyword: %s\n", yytext);
			} else {
				printf("Identifier: %s\n", yytext);
			}}
[0-9]*		{printf("Number: %s\n", yytext);}
"+"|"-"|"*"|"/"	{printf("Operator: %s\n", yytext);}
[ \t]		;
%%

int main(int argc, char* argv[]) {
if (argc < 2) {
printf("Usage: %s <file>\n", argv[0]);
return 1;
}
yyin = fopen(argv[1], "r");
yylex();
fclose(yyin);
}
