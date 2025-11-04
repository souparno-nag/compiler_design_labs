%{
    #include <stdio.h>
    #include "calc.tab.h"
%}

%%
[0-9]+      { yylval.f = atoi(yytext); return NUM; }
[ \t\f\n\v] ;
[+*-/()]    {return yytext[0];}
%%

int yywrap() {
return 1;
}