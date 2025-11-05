%{
    #include <stdlib.h>
    #include "calc.tab.h"
%}

%%
[0-9]+      { yylval = atoi(yytext); return NUMBER; }
[ \t\f] ;
"("         return '(';
")"         return ')';
"+"         return '+';
"-"         return '-';
"*"         return '*';
"/"         return '/';
\n          return '\n';
.           ;
%%

int yywrap() {
return 1;
}