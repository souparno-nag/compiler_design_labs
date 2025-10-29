%{
    #include <stdio.h>
    int lc = 0, sc = 0, wc = 0, cc = 0;
%}

%%
[\n] {lc++; cc += yyleng;}
[ \t] {sc++; cc += yyleng;}
[^\t\n ]+ {wc++; cc += yyleng;}
%%

int main()
{
    printf("Enter the input:\n");
    yylex();
    printf("No of words are: %d\n", wc);
    printf("No of characters are: %d\n", cc);
    printf("No of new lines are: %d\n", lc);
    printf("No of spaces are: %d\n", sc);
}

int yywrap() {
    return 1;
}