%{
    #include <stdio.h>
    #include <string.h>
    char oldword[100], newword[100];
%}

%option noyywrap

%%
[a-zA-Z]+   {
                if (strcmp(yytext, oldword) == 0) {
                    printf("%s", newword);
                } else {
                    printf("%s", yytext);
                }
            }
.|\n        { ECHO; }
%%

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <oldword> <newword> <file>\n", argv[0]);
        return 1;
    }
    strcpy(oldword, argv[1]);
    strcpy(newword, argv[2]);
    yyin = fopen(argv[3], "r");
    if (!yyin) {
        printf("Unable to read file: %s\n", argv[3]);
        return 1;
    }
    yylex();
    fclose(yyin);
}