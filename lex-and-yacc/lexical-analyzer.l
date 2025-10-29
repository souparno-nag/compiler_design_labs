%{
    #include <stdio.h>
    #include <string.h>
    #include <stdbool.h>
    
    char* keywords[] = {"int","float","char","if","else","while","return","for"};
    bool isKeyword (char* word) {
        for (int i = 0; i < 8; i++) {
            if (strcmp(keywords[i], word) == 0) {
                return true;
            }
        }
        return false;
    }
%}

%option noyywrap

%%
[a-zA-Z]+   {
                if (isKeyword(yytext)) {
                    printf("Keyword: %s\n", yytext);
                } else {
                    printf("Identifier: %s\n", yytext);
                }
            }
[0-9]+      printf("Number: %s\n", yytext);
.|;         printf("Punctuation: %s\n", yytext);
"=="|"="|"+"|"-"|"*"|"/"|"<"|">"|"!="|"<="|">="         printf("Operator: %s\n", yytext);
[ \t\n]     ;
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
    fclose(yyin);
}