%{
#include "infix.tab.h"
%}
%%
[a-zA-Z] { yylval.c = yytext[0]; return ID; }
[ \t] ; // ignore whitespace
\n { return '\n'; }
. { return yytext[0]; }
%%
int yywrap() {
return 1;
}