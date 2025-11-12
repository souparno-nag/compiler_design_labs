#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define MAX 100

int checkStateIndex(char* states[], int q, char* c) {
    for (int i = 0; i < q; i++) {
        if (strcmp(c, states[i]) == 0) {
            return i;
        }
    }
    return -1;
}

int checkInputIndex(char inputs[], int t, char c) {
    for (int i = 0; i < t; i++) {
        if (c == inputs[i]) {
            return i;
        }
    }
    return -1;
}

bool checkFinalState(char* finals[], int f, char* c) {
    for (int i = 0; i < f; i++) {
        if (strcmp(c, finals[i]) == 0) {
            return true;
        }
    }
    return false;
}

int main() {
	int q;
	scanf("%d", &q);
	char* states[q];
	for (int i = 0; i < q; i++) {
	    states[i] = malloc(MAX*sizeof(char*));
	    scanf("%s", states[i]);
	}
	int t;
	scanf("%d", &t);
	char inputs[t];
	for (int i = 0; i < t; i++) {
	    scanf(" %c", &inputs[i]);
	}
	int f;
	scanf("%d", &f);
	char* finals[f];
	for (int i = 0; i < f; i++) {
	    finals[i] = malloc(MAX*sizeof(char*));
	    scanf("%s", finals[i]);
	}
	char* delta[q][t];
	for (int i = 0; i < q; i++) {
	    for (int j = 0; j < t; j++) {
	        delta[i][j] = malloc(MAX*sizeof(char*));
	        scanf("%s", delta[i][j]);
	    }
	}
// 	printf("\t");
	for (int i = 0; i < t; i++) {
	    printf("\t%c", inputs[i]);
	}
	printf("\n");
	for (int i = 0; i < q; i++) {
	    printf("%s\t", states[i]);
	    for (int j = 0; j < t; j++) {
	        printf("%s\t", delta[i][j]);
	    }
	    printf("\n");
	}
	char* input;
	input = malloc(MAX*sizeof(char));
	scanf("%s", input);
	int n = strlen(input);
	char* currentState = states[0];
	for (int i = 0; i < n; i++) {
	    char c = input[i];
	    int i = checkInputIndex(inputs, t, c);
	    int s = checkStateIndex(states, q, currentState);
	    if (i == -1 || s == -1) exit(1);
	    currentState = delta[s][i];
	}
	if (checkFinalState(finals, f, currentState)) {
	    printf("Accepted\n");
	} else {
	    printf("Rejected\n");
	}
}

