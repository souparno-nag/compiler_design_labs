%option noyywrap
%{
#include "Ex15.tab.h"
#include <stdlib.h>
#include <string.h>
%}

digit   [0-9]
id      [A-Za-z_][A-Za-z0-9_]*
ws      [ \t\r\n]+

%%
{ws}                    {}
"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }

{digit}+               { yylval.intval = atoi(yytext); return NUMBER; }
{id}                   { yylval.id = strdup(yytext); return ID; }

"=="                    { return EQ; }
"!="                    { return NE; }
"<="                    { return LE; }
">="                    { return GE; }

"<"                     { return '<'; }
">"                     { return '>'; }
"="                     { return '='; }

"+"                     { return '+'; }
"-"                     { return '-'; }
"*"                     { return '*'; }
"/"                     { return '/'; }

"("                     { return '('; }
")"                     { return ')'; }
"{"                     { return '{'; }
"}"                     { return '}'; }
";"                     { return ';'; }

.                       {}
%%
