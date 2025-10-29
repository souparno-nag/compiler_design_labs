/*Lex Program to recognize and display keywords, numbers and words in a given statement*/
%{
    #include <stdio.h>
%} /* header files */

%% /* rule section (in form of pattern -> action) */
if |
else |
printf {printf("%s is a keyword\n", yytext);}
[0-9]+ {printf("%s is a number\n", yytext);}
[a-zA-Z]+ {printf("%s is a word\n", yytext);}
.|\n {ECHO;} /* skip */
%%

int main()
{
    printf("\n Enter the string: ");
    yylex();
}

int yywrap()
{
    return 1;
}