#include <stdio.h>
#include <string.h>

int checkSymbol(char ch, char* inputs, int t) {
    for (int i = 0; i < t; i++) {
        if (ch == inputs[i]) {
            return i;
        }
    }
    return -1;
}

int checkFinalState (int st, int f, int* finals) {
    for (int i = 0; i < f; i++) {
        if (st == finals[i]) {
            return 1;
        }
    }
    return 0;
}

int main() {
    int q;
    printf("Enter no of states: ");
    scanf("%d", &q);
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
    int finals[f];
    for (int i = 0; i < f; i++){
        printf("Enter final state no %d: ",(i+1));
        scanf("%d", &finals[i]);
    }
    printf("Enter transitions: \n");
    int delta[q][t];
    for (int i = 0; i < q; i++) {
        for (int j = 0; j < t; j++) {
            printf("d(q%d,%c) : ",i,inputs[j]);
            scanf("%d", &delta[i][j]);
        }
    }
    printf("Transition Table:\n\t");
    for (int j = 0; j < t; j++) {
        printf("%c\t",inputs[j]);
    }
    printf("\n");
    for (int i = 0; i < q; i++) {
        printf("q%d\t",i);
        for (int j = 0; j < t; j++) {
            printf("%d\t",delta[i][j]);
        }
        printf("\n");
    }
    do {
        int n;
        printf("Enter the length of the terminal string: ");
        scanf("%d", &n);
        char w[n];
        printf("Enter the terminal string: ");
        scanf("%s", w);
        w[n] = '\0';

        int stateCounter = 0, flag = 1;

        for (int i = 0; i < n; i++) {
            int symPos = checkSymbol(w[i], inputs, t);
            if (symPos == -1) {
                flag = 0;
                break;
            }
            stateCounter = delta[stateCounter][symPos];
        }

        if ((flag) && (checkFinalState(stateCounter, f, finals))) {
            printf("%s is accepted\n", w);
        } else {
            printf("%s is not accepted\n", w);
        }
        break;
    } while (1);
    return 0;
}