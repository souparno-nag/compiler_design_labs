/* count_word.l */
%{
#include <stdio.h>
#include <string.h>

char target[100];
int count = 0;
%}

%%
[a-zA-Z]+   {
                if(strcmp(yytext, target) == 0)
                    count++;
            }
.           ;
\n          ;
%%

int main(int argc, char *argv[]) {
    if(argc < 3) {
        printf("Usage: %s <word> <file>\n", argv[0]);
        return 1;
    }
    strcpy(target, argv[1]);
    yyin = fopen(argv[2], "r");
    if(!yyin) {
        perror("File open error");
        return 1;
    }
    yylex();
    printf("Frequency of '%s' = %d\n", target, count);
    fclose(yyin);
    return 0;
}

