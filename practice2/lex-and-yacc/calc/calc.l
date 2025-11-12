%{
#include "calc.tab.h"
%}

%%
[0-9]*	{yylval.n = atoi(yytext); return NUM;}
"+"|"-"|"*"|"/"|"%"	{return yytext[0];}
"\n"	{return '\n';}
[ \t]	;
.	;
%%

int yywrap() {
return 1;
} 
