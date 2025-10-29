%{
    #include <stdio.h>
    #include <string.h>
    int count = 0;
    char target[100];
%}

%option noyywrap

%%
[a-zA-Z]+   {
                if (strcmp(yytext, target) == 0) {
                    count++;
                }
            }
.           ;
\n          ;
%%

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <word> <file>\n", argv[0]);
        return 1;
    }
    strcpy(target, argv[1]);
    yyin = fopen(argv[2], "r");
    if (!yyin) {
        printf("Unable to read file: %s\n", argv[2]);
        return 1;
    }
    yylex();
    printf("Count of %s is %d\n", target, count);
    fclose(yyin);
}