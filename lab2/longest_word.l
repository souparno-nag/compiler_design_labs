/* longest_word.l */
%{
#include <stdio.h>
#include <string.h>

int maxlen = 0;
char longest[100];
%}

%%
[a-zA-Z]+   {
                if(strlen(yytext) > maxlen) {
                    maxlen = strlen(yytext);
                    strcpy(longest, yytext);
                }
            }
.|\n        ;
%%

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    yyin = fopen(argv[1], "r");
    if(!yyin) {
        perror("File open error");
        return 1;
    }
    yylex();
    printf("Longest word: %s (Length: %d)\n", longest, maxlen);
    fclose(yyin);
    return 0;
}
