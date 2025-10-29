%{
#include "lang1.tab.h"
%}
%%
a { return A; }
b { return B; }
\n { return '\n'; }
. { return yytext[0]; }
%%
int yywrap() {
return 1;
}