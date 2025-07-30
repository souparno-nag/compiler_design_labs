#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STATES 20
#define MAX_SYMBOLS 10

// Helper: Find index of a state name
int get_state_index(char *state, char states[][10], int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(state, states[i]) == 0)
            return i;
    }
    return -1;
}

// Helper: Check if a state is already in the set
int is_in_set(char *state, char set[][10], int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(set[i], state) == 0)
            return 1;
    }
    return 0;
}

// Add state to set if not present
void add_to_set(char *state, char set[][10], int *count) {
    if (!is_in_set(state, set, *count)) {
        strcpy(set[*count], state);
        (*count)++;
    }
}

// Compare two sets
int compare_sets(char set1[][10], int count1, char set2[][10], int count2) {
    if (count1 != count2)
        return 0;
    for (int i = 0; i < count1; i++) {
        if (!is_in_set(set1[i], set2, count2))
            return 0;
    }
    return 1;
}

int main() {
    int nfa_state_count, symbol_count, nfa_final_count;
    char nfa_states[MAX_STATES][10];
    char symbols[MAX_SYMBOLS];
    char nfa_finals[MAX_STATES][10];
    char transition[MAX_STATES][MAX_SYMBOLS][MAX_STATES][10];
    int transition_count[MAX_STATES][MAX_SYMBOLS] = {0};

    // Input NFA
    printf("Enter number of NFA states: ");
    scanf("%d", &nfa_state_count);
    printf("Enter state names:\n");
    for (int i = 0; i < nfa_state_count; i++) {
        scanf("%s", nfa_states[i]);
    }

    printf("Enter number of input symbols: ");
    scanf("%d", &symbol_count);
    printf("Enter input symbols:\n");
    for (int i = 0; i < symbol_count; i++) {
        scanf(" %c", &symbols[i]);
    }

    printf("Enter number of final states: ");
    scanf("%d", &nfa_final_count);
    printf("Enter final state names:\n");
    for (int i = 0; i < nfa_final_count; i++) {
        scanf("%s", nfa_finals[i]);
    }

    // NFA transitions
    printf("Enter transitions (comma-separated states, or - for empty):\n");
    for (int i = 0; i < nfa_state_count; i++) {
        for (int j = 0; j < symbol_count; j++) {
            printf("δ(%s, %c): ", nfa_states[i], symbols[j]);
            char input[100];
            scanf("%s", input);

            if (strcmp(input, "-") == 0)
                continue;

            char *token = strtok(input, ",");
            while (token != NULL) {
                strcpy(transition[i][j][transition_count[i][j]++], token);
                token = strtok(NULL, ",");
            }
        }
    }

    // DFA Construction
    char dfa_states[100][MAX_STATES][10];
    int dfa_state_count = 0;
    int dfa_final[100] = {0};
    int dfa_transitions[100][MAX_SYMBOLS];

    // DFA start state = {start state of NFA}
    strcpy(dfa_states[0][0], nfa_states[0]);
    int dfa_state_sizes[100] = {1};
    dfa_state_count = 1;

    for (int i = 0; i < dfa_state_count; i++) {
        for (int j = 0; j < symbol_count; j++) {
            char new_set[MAX_STATES][10];
            int new_count = 0;

            for (int k = 0; k < dfa_state_sizes[i]; k++) {
                int idx = get_state_index(dfa_states[i][k], nfa_states, nfa_state_count);
                for (int l = 0; l < transition_count[idx][j]; l++) {
                    add_to_set(transition[idx][j][l], new_set, &new_count);
                }
            }

            // Check if new_set already exists
            int found = -1;
            for (int x = 0; x < dfa_state_count; x++) {
                if (compare_sets(new_set, new_count, dfa_states[x], dfa_state_sizes[x])) {
                    found = x;
                    break;
                }
            }

            if (found == -1) {
                // New DFA state
                for (int x = 0; x < new_count; x++) {
                    strcpy(dfa_states[dfa_state_count][x], new_set[x]);
                }
                dfa_state_sizes[dfa_state_count] = new_count;
                found = dfa_state_count;
                dfa_state_count++;
            }

            dfa_transitions[i][j] = found;
        }
    }

    // Determine final states in DFA
    for (int i = 0; i < dfa_state_count; i++) {
        for (int j = 0; j < dfa_state_sizes[i]; j++) {
            if (is_in_set(dfa_states[i][j], nfa_finals, nfa_final_count)) {
                dfa_final[i] = 1;
                break;
            }
        }
    }

    // Output DFA
    printf("\nDFA Transition Table:\n");
    printf("State\t");
    for (int i = 0; i < symbol_count; i++)
        printf("%c\t", symbols[i]);
    printf("Final\n");

    for (int i = 0; i < dfa_state_count; i++) {
        printf("{");
        for (int j = 0; j < dfa_state_sizes[i]; j++) {
            printf("%s", dfa_states[i][j]);
            if (j != dfa_state_sizes[i] - 1) printf(",");
        }
        printf("}\t");
        for (int j = 0; j < symbol_count; j++) {
            printf("%d\t", dfa_transitions[i][j]);
        }
        printf("%s\n", dfa_final[i] ? "Yes" : "No");
    }

    return 0;
}
