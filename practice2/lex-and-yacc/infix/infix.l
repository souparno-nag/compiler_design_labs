%{
#include "infix.tab.h"
%}

%%
[a-z]*	{yylval.c = yytext[0]; return ID;}
"\n"	{return '\n';}
[ \t]	;
.	{return yytext[0];}
%%

int yywrap() {
return 1;
}
