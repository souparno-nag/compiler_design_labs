%{
    #include <stdio.h>
    int v = 0, c = 0;
%}

%%
[ \t\n]+   { /* skip whitespace */ }
[aeiouAEIOU] { v++; }
[a-zA-Z]   { c++; }
.          { /* ignore other characters */ }
%%

int main() 
{
    printf("Enter the input string (press Ctrl+D to end input):\n");
    yylex();
    printf("No of vowels are: %d\n", v);
    printf("No of consonants are: %d\n", c);
    return 0;
}

int yywrap()
{
    return 1;
}