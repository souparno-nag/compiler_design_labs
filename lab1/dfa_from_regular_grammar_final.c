#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define MAX_LENGTH 100

int checkSymbol(char ch, char* inputs, int t) {
    for (int i = 0; i < t; i++) {
        if (ch == inputs[i]) {
            return i;
        }
    }
    return -1;
}

int checkFinalState(char* st, int f, char* finals[]) {
    for (int i = 0; i < f; i++) {
        if (strcmp(st, finals[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int getStateIndex(char* state, char* states[], int q) {
    for (int i = 0; i < q; i++) {
        if (strcmp(state, states[i]) == 0) {
            return i;
        }
    }
    return -1;
}

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

    int t;
    printf("Enter no of input symbols: ");
    scanf("%d", &t);
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
        printf("Enter final state no %d: ",(i+1));
        scanf("%s", finals[i]);
    }

    printf("Enter transitions: \n");
    char *delta[q][t];
    for (int i = 0; i < q; i++) {
        for (int j = 0; j < t; j++) {
            delta[i][j] = malloc(MAX_LENGTH * sizeof(char));
            printf("d(%s,%c) : ",states[i],inputs[j]);
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
   
    int n;
    printf("Enter the length of the terminal string: ");
    scanf("%d", &n);
    char w[n + 1];
    printf("Enter the terminal string: ");
    scanf("%s", w);
    w[n] = '\0';

    char* stateCounter = states[0];
    int flag = 1;

    for (int i = 0; i < n; i++) {
        int symPos = checkSymbol(w[i], inputs, t);
        if (symPos == -1) {
            flag = 0;
            break;
        }
        int currentStateIndex = getStateIndex(stateCounter, states, q);
        if (currentStateIndex == -1) {
            flag = 0;
            break;
        }
        stateCounter = delta[currentStateIndex][symPos];
    }

    if (flag && checkFinalState(stateCounter, f, finals)) {
        printf("%s is accepted\n", w);
    } else {
        printf("%s is not accepted\n", w);
    }
    return 0;
}
