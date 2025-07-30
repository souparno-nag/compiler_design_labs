#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 20

int numStates, numSymbols, numFinals;
char states[MAX], symbols[MAX], startState;
char finalStates[MAX];
char transition[MAX][MAX];

int stateIndex(char state) {
    for (int i = 0; i < numStates; i++) {
        if (states[i] == state)
            return i;
    }
    return -1;
}

int isFinal(char state) {
    for (int i = 0; i < numFinals; i++) {
        if (finalStates[i] == state)
            return 1;
    }
    return 0;
}

int main() {
    int i, j;

    // Input states
    printf("Enter number of states: ");
    scanf("%d", &numStates);
    printf("Enter state names (single uppercase letters): ");
    for (i = 0; i < numStates; i++) {
        scanf(" %c", &states[i]);
    }

    // Input input symbols
    printf("Enter number of input symbols: ");
    scanf("%d", &numSymbols);
    printf("Enter input symbols: ");
    for (i = 0; i < numSymbols; i++) {
        scanf(" %c", &symbols[i]);
    }

    // Input start state
    printf("Enter start state: ");
    scanf(" %c", &startState);

    // Input final states
    printf("Enter number of final states: ");
    scanf("%d", &numFinals);
    printf("Enter final states: ");
    for (i = 0; i < numFinals; i++) {
        scanf(" %c", &finalStates[i]);
    }

    // Input transition table
    printf("Enter transition table (next state for state and input symbol):\n");
    for (i = 0; i < numStates; i++) {
        for (j = 0; j < numSymbols; j++) {
            printf("δ(%c, %c) = ", states[i], symbols[j]);
            scanf(" %c", &transition[i][j]);
        }
    }

    // Output the grammar
    printf("\nRegular Grammar G = (N, T, P, S):\n");

    printf("\nN (Non-terminals): { ");
    for (i = 0; i < numStates; i++) {
        printf("%c ", states[i]);
    }
    printf("}\n");

    printf("T (Terminals): { ");
    for (i = 0; i < numSymbols; i++) {
        printf("%c ", symbols[i]);
    }
    printf("}\n");

    printf("S (Start symbol): %c\n", startState);

    printf("P (Productions):\n");
    for (i = 0; i < numStates; i++) {
        for (j = 0; j < numSymbols; j++) {
            char from = states[i];
            char to = transition[i][j];
            char symbol = symbols[j];
            if (isFinal(to)) {
                // A → aB and A → a
                printf("  %c → %c%c\n", from, symbol, to);
                printf("  %c → %c\n", from, symbol);
            } else {
                printf("  %c → %c%c\n", from, symbol, to);
            }
        }
    }

    return 0;
}
