%{
    #include <stdio.h>
    #include <string.h>
    int maxLen = 0;
    char longest[100];
%}

%option noyywrap

%%
[a-zA-Z]+   {
                if (strlen(yytext) > maxLen) {
                    maxLen = strlen(yytext);
                    strcpy(longest, yytext);
                }
            }
.           ;
\n          ;
%%

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        printf("Unable to read file: %s\n", argv[1]);
        return 1;
    }
    yylex();
    printf("Longest word is %s\n", longest);
    fclose(yyin);
}