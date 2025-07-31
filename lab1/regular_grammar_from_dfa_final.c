#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define MAX_LENGTH 100

int main() {
    int q;
    printf("Enter no of states: ");
    scanf("%d", &q);
    char *states[q];
    printf("Enter all the states: ");
    for (int i = 0; i < q; i++) {
        states[i] = malloc(MAX_LENGTH * sizeof(char));
        scanf("%s", states[i]);
    }
    // for (int i = 0; i < q; i++) {
    //     printf("%s\n", states[i]);
    // }
    int t;
    printf("Enter no of input symbols: ");
    scanf("%d", &t);
    // char ch;
    // scanf("%c", &ch);
    char inputs[t];
    for (int i = 0; i < t; i++){
        printf("Enter input symbol no %d: ",(i+1));
        scanf(" %c", &inputs[i]);
    }
    int f;
    printf("Enter no of final states: ");
    scanf("%d", &f);
    char *finals[f];
    for (int i = 0; i < f; i++){
        finals[i] = malloc(MAX_LENGTH * sizeof(char));
        printf("Enter final state no %d: ", (i + 1));
        scanf("%s", finals[i]);
    }
    printf("Enter transitions: \n");
    char *delta[q][t];
    for (int i = 0; i < q; i++) {
        for (int j = 0; j < t; j++) {
            delta[i][j] = malloc(MAX_LENGTH * sizeof(char));
            printf("d(%s,%c) : ",states[i],inputs[j]);
            fflush(stdout);
            scanf("%s", delta[i][j]);
        }
    }
    printf("Transition Table:\n\t");
    for (int j = 0; j < t; j++) {
        printf("%c\t",inputs[j]);
    }
    printf("\n");
    for (int i = 0; i < q; i++) {
        printf("%s\t",states[i]);
        for (int j = 0; j < t; j++) {
            printf("%s\t",delta[i][j]);
        }
        printf("\n");
    }
   
    printf("Presenting grammar G:\n");
    // non terminal symbols
    printf("Non Terminal symbols, N: {");
    for (int i = 0; i < q; i++) {
        printf("%s, ", states[i]);
    }
    printf("}\n");
    // terminal symbols
    printf("Terminal symbols, T: {");
    for (int i = 0; i < t; i++) {
        printf("%c, ", inputs[i]);
    }
    printf("}\n");
    // start symbol
    printf("Start symbol, S: {%s}\n", states[0]);
    // Production
    printf("Production, P: {\n");
    for (int i = 0; i < q; i++) {
        for (int j = 0; j < t; j++) {
            printf("d(%s,%c) : ",states[i],inputs[j]);
            printf("%s\n", delta[i][j]);
        }
    }
    printf("}");
    return 0;
}