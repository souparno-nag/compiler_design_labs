%{
#include "tac.tab.h"
%}

%%
[0-9]*	{yylval.n = atoi(yytext); return NUM;}
[a-zA-Z][a-zA-Z0-9]*	{yylval.s = strdup(yytext); return ID;}
"+"|"-"|"*"|"/"|"%"	{return yytext[0];}
"\n"	{return '\n';}
.	;
%%

int yywrap() {
return 1;
}
