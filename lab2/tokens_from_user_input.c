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
        return (ch == ';') || (ch == '(') || (ch == ')') || (ch == '{') || (ch == '}') ||(ch == ',');
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
 
void analyzeFile(const char* filename) {
        FILE *file = fopen(filename, "r");
        if (!file) {
                printf("Failed to open file.");
                return;
        }
        char line [256];
        while (fgets(line, sizeof(line), file)) {
                identifyTokens(line);
        }
        fclose(file);
        return;
}
 
void getUserInputandAnalyze() {
        FILE *file = fopen("user_input.c", "w");
        if (!file) {
                printf("Failed to create file.\n");
                return;
        }
        printf("\nEnter up to 5 lines of code (end with an empty line):\n");
        char line[256];
        int count = 0;
        while (count < 5) {
                fgets(line, sizeof(line), stdin);
                if (strcmp(line, "\n") == 0) break;
                fputs(line, file);
                count++;
        }
        fclose(file);
        analyzeFile("user_input.c");
}
 
 
int main() {
        getUserInputandAnalyze();
        return 0;
}