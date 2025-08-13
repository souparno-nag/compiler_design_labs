#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
 
const char *keywords[] = {"int", "return", "float", "char", "if", "else", "for", "while", "void"};
 
int isKeyword(const char *str) {
        for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
                if (strcmp(str, keywords[i]) == 0) {
                        return 1;
                }
        }
        return 0;
}
 
int isArithmeticOperator (char ch) {
        return (ch == '+') || (ch == '-') || (ch == '*') || (ch == '/') ;
}
 
int isAssignmentOperator(char ch) {
        return (ch == '=');
}
 
int isPunctuation(char ch) {
        return (ch == ';');
}
 
void identifyTokens(char *code) {
        int i = 0, j = 0;
        char buffer[100];
 
        while (code[i] != '\0') {
                if (isspace(code[i])) {
                        i++;
                }
                if (isalpha(code[i]) || (code[i] == '_')) {
                        j = 0;
                        while (isalnum(code[i]) || (code[i] == '_')) {
                                buffer[j++] = code[i++];
                        }
                        buffer[j] = '\0';
                        if (isKeyword(buffer)) {
                                printf("Keyword: %s\n", buffer);
                        } else {
                                printf("Identifier: %s\n", buffer);
                        }
                } else if (isdigit(code[i])) {
                        j = 0;
                        while (isdigit(code[i])) {
                                buffer[j++] = code[i++];
                        }
                        buffer[j] = '\0';
                        printf("Number: %s\n", buffer);
                } else if (isArithmeticOperator(code[i])) {
                        printf("Arithmetic Operator: %c\n",code[i++]);
                } else if (isAssignmentOperator(code[i])) {
                        printf("Assignment Operator: %c\n",code[i++]);
                } else if (isPunctuation(code[i])) {
                        printf("Punctuation: %c\n",code[i++]);
                }
        }
        return;
}
 
int main() {
        char code[] = "int a = b + c * d;";
        identifyTokens(code);
        return 0;
}