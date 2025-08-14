/* lexical_analyzer.l */
%{
#include <stdio.h>
#include <string.h>

char *keywords[] = {"int","float","char","if","else","while","return","for"};
int is_keyword(char *word) {
    for(int i=0; i<8; i++)
        if(strcmp(keywords[i], word) == 0)
            return 1;
    return 0;
}
%}

%%
[ \t\n]+                 ;   // ignore spaces
[a-zA-Z_][a-zA-Z0-9_]*   {
                            if(is_keyword(yytext))
                                printf("<KEYWORD, %s>\n", yytext);
                            else
                                printf("<IDENTIFIER, %s>\n", yytext);
                         }
[0-9]+                   { printf("<NUMBER, %s>\n", yytext); }
"=="|"!="|"<="|">="|"<"|">"|"="|"+"|"-"|"*"|"/"   { printf("<OPERATOR, %s>\n", yytext); }
.                        { printf("<PUNCTUATION, %s>\n", yytext); }
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
    fclose(yyin);
    return 0;
}
