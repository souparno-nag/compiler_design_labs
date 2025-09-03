#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Global variables
char productions[20][20];
char nonTerminals[10];
char terminals[20];
char firstSets[10][20];
char followSets[10][20];
char parseTable[10][20][20];
int numProductions;
int numNonTerminals = 0;
int numTerminals = 0;

// Helper function to add a character to a set (string) without duplication
void addToSet(char *set, char c) {
    if (strchr(set, c) == NULL) {
        int len = strlen(set);
        set[len] = c;
        set[len + 1] = '\0';
    }
}

// Function to find the index of a non-terminal
int findNonTerminalIndex(char nt) {
    for (int i = 0; i < numNonTerminals; i++) {
        if (nonTerminals[i] == nt) return i;
    }
    return -1;
}

// Function to find the index of a terminal
int findTerminalIndex(char t) {
    for (int i = 0; i < numTerminals; i++) {
        if (terminals[i] == t) return i;
    }
    return -1;
}

// ----- Part (a): FIRST and FOLLOW Set Calculation -----

void findFirst(char symbol, int ntIndex) {
    // If the symbol is a terminal, its FIRST set is the symbol itself.
    if (!isupper(symbol)) {
        addToSet(firstSets[ntIndex], symbol);
        return;
    }

    // If the symbol is a non-terminal
    for (int i = 0; i < numProductions; i++) {
        if (productions[i][0] == symbol) {
            // Case 1: Production is X -> #
            if (productions[i][3] == '#') {
                addToSet(firstSets[ntIndex], '#');
            } 
            // Case 2: Production is X -> Y...
            else {
                // Recursively find FIRST of the first symbol in the RHS
                int k = 3;
                while (productions[i][k] != '\0') {
                    char nextSymbol = productions[i][k];
                    int hasEpsilon = 0;
                    
                    if (!isupper(nextSymbol)) { // It's a terminal
                        addToSet(firstSets[ntIndex], nextSymbol);
                        break; // No need to check further in this production
                    } else { // It's a non-terminal
                        int nextNtIndex = findNonTerminalIndex(nextSymbol);
                        // Find FIRST of this non-terminal if not already found
                        if (firstSets[nextNtIndex][0] == '\0') {
                           findFirst(nextSymbol, nextNtIndex);
                        }
                        // Add its FIRST set to the current symbol's FIRST set
                        for (int j = 0; j < strlen(firstSets[nextNtIndex]); j++) {
                            if (firstSets[nextNtIndex][j] == '#') {
                                hasEpsilon = 1;
                            } else {
                                addToSet(firstSets[ntIndex], firstSets[nextNtIndex][j]);
                            }
                        }
                    }
                    if (!hasEpsilon) break;
                    k++;
                }
            }
        }
    }
}

void calculateFirstSets() {
    for (int i = 0; i < numNonTerminals; i++) {
        findFirst(nonTerminals[i], i);
    }
}

void calculateFollowSets() {
    // Initialize all follow sets to empty
    for(int i = 0; i < numNonTerminals; i++) {
        strcpy(followSets[i], "");
    }

    // Rule 1: Place $ in FOLLOW(S), where S is the start symbol
    int startIndex = findNonTerminalIndex(productions[0][0]);
    addToSet(followSets[startIndex], '$');

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < numProductions; i++) {
            char lhs = productions[i][0];
            int lhsIndex = findNonTerminalIndex(lhs);

            for (int j = 3; productions[i][j] != '\0'; j++) {
                char currentSymbol = productions[i][j];
                if (isupper(currentSymbol)) {
                    int currentIndex = findNonTerminalIndex(currentSymbol);
                    char nextSymbol = productions[i][j + 1];
                    int originalLen = strlen(followSets[currentIndex]);

                    // Rule 2: Production A -> αBβ
                    if (nextSymbol != '\0') {
                        if (!isupper(nextSymbol)) { // Terminal
                            addToSet(followSets[currentIndex], nextSymbol);
                        } else { // Non-terminal
                            int nextIndex = findNonTerminalIndex(nextSymbol);
                            // Add FIRST(β) to FOLLOW(B)
                            for (int k = 0; k < strlen(firstSets[nextIndex]); k++) {
                                if (firstSets[nextIndex][k] != '#') {
                                    addToSet(followSets[currentIndex], firstSets[nextIndex][k]);
                                }
                            }
                            // If FIRST(β) contains ε, add FOLLOW(A) to FOLLOW(B)
                            if (strchr(firstSets[nextIndex], '#') != NULL) {
                                for (int k = 0; k < strlen(followSets[lhsIndex]); k++) {
                                    addToSet(followSets[currentIndex], followSets[lhsIndex][k]);
                                }
                            }
                        }
                    } 
                    // Rule 3: Production A -> αB or A -> αBβ where FIRST(β) contains ε
                    else { 
                        for (int k = 0; k < strlen(followSets[lhsIndex]); k++) {
                            addToSet(followSets[currentIndex], followSets[lhsIndex][k]);
                        }
                    }
                     if(strlen(followSets[currentIndex]) > originalLen) changed = 1;
                }
            }
        }
    }
}

void constructParseTable() {
    // Initialize table with "error" entries
    for (int i = 0; i < numNonTerminals; i++) {
        for (int j = 0; j < numTerminals; j++) {
            strcpy(parseTable[i][j], "error");
        }
    }
    
    for (int i = 0; i < numProductions; i++) {
        char lhs = productions[i][0];
        char* rhs = &productions[i][3];
        int lhsIndex = findNonTerminalIndex(lhs);

        char firstChar = rhs[0];
        char firstSetOfRHS[20];
        strcpy(firstSetOfRHS, "");

        // Step 1: Calculate FIRST of the RHS (α)
        if (!isupper(firstChar)) { // Terminal or Epsilon
            addToSet(firstSetOfRHS, firstChar);
        } else { // Non-terminal
            int k = 0;
            while(rhs[k] != '\0') {
                 if(!isupper(rhs[k])){ // a terminal appeared
                    addToSet(firstSetOfRHS, rhs[k]);
                    break;
                 }
                 int ntIndex = findNonTerminalIndex(rhs[k]);
                 int hasEpsilon = 0;
                 for(int l=0; l<strlen(firstSets[ntIndex]); l++){
                     if(firstSets[ntIndex][l] == '#') hasEpsilon = 1;
                     else addToSet(firstSetOfRHS, firstSets[ntIndex][l]);
                 }
                 if(!hasEpsilon) break;
                 k++;
                 if(rhs[k] == '\0') addToSet(firstSetOfRHS, '#'); // All symbols produced epsilon
            }
        }

        // Step 2: Populate the table based on FIRST(α)
        for (int j = 0; j < strlen(firstSetOfRHS); j++) {
            char terminal = firstSetOfRHS[j];
            if (terminal != '#') {
                int termIndex = findTerminalIndex(terminal);
                if (termIndex != -1) {
                    strcpy(parseTable[lhsIndex][termIndex], productions[i]);
                }
            }
        }

        // Step 3: Handle the epsilon case
        if (strchr(firstSetOfRHS, '#') != NULL) {
            int followIndex = findNonTerminalIndex(lhs);
            for (int j = 0; j < strlen(followSets[followIndex]); j++) {
                char terminal = followSets[followIndex][j];
                int termIndex = findTerminalIndex(terminal);
                if (termIndex != -1) {
                    strcpy(parseTable[lhsIndex][termIndex], productions[i]);
                }
            }
        }
    }
}

// ----- Part (b): Predictive Parsing Algorithm -----

void parseString(char *input) {
    char stack[50];
    int top = -1;
    int ip = 0;

    // Initialize stack
    stack[++top] = '$';
    stack[++top] = productions[0][0]; // Push start symbol

    printf("\n--- Parsing Trace ---\n");
    printf("%-20s %-20s %-20s\n", "Stack", "Input", "Action");
    printf("------------------------------------------------------------\n");

    while (top != -1) {
        // Print current stack and input
        char stackStr[50] = "", inputStr[50] = "";
        for(int i=0; i<=top; i++) stackStr[i] = stack[i];
        strcpy(inputStr, &input[ip]);
        printf("%-20s %-20s ", stackStr, inputStr);

        char stackTop = stack[top];
        char currentInput = input[ip];

        if (stackTop == currentInput) {
            printf("Match %c\n", currentInput);
            top--;
            ip++;
        } else if (!isupper(stackTop)) {
            printf("Error: Mismatch\n");
            break;
        } else {
            int ntIndex = findNonTerminalIndex(stackTop);
            int tIndex = findTerminalIndex(currentInput);
            
            if (ntIndex == -1 || tIndex == -1 || strcmp(parseTable[ntIndex][tIndex], "error") == 0) {
                printf("Error: No rule in parse table\n");
                break;
            }

            char* rule = parseTable[ntIndex][tIndex];
            printf("Apply %s\n", rule);
            top--; // Pop the non-terminal

            char* rhs = &rule[3];
            // If it's not an epsilon production, push RHS in reverse
            if (rhs[0] != '#') {
                int len = strlen(rhs);
                for (int i = len - 1; i >= 0; i--) {
                    stack[++top] = rhs[i];
                }
            }
        }
    }

    printf("------------------------------------------------------------\n");
    if (input[ip] == '\0' && top == -1) {
        printf("\n✅ Success! The string was parsed successfully.\n");
    } else {
        printf("\n❌ Failure! The string was rejected.\n");
    }
}

int main() {
    printf("Enter the number of productions: ");
    scanf("%d", &numProductions);
    printf("Enter the productions (e.g., E->E+T, use '#' for epsilon):\n");
    for (int i = 0; i < numProductions; i++) {
        scanf("%s", productions[i]);
    }

    // Discover non-terminals and terminals
    for (int i = 0; i < numProductions; i++) {
        addToSet(nonTerminals, productions[i][0]);
        for (int j = 3; productions[i][j] != '\0'; j++) {
            if (!isupper(productions[i][j]) && productions[i][j] != '#') {
                addToSet(terminals, productions[i][j]);
            }
        }
    }
    terminals[strlen(terminals)] = '$'; // Add end marker to terminals
    numNonTerminals = strlen(nonTerminals);
    numTerminals = strlen(terminals);

    // --- Experiment-7(a) ---
    printf("\n--- Starting Experiment-7(a): Table Construction ---\n");

    calculateFirstSets();
    printf("\nFIRST Sets:\n");
    for (int i = 0; i < numNonTerminals; i++) {
        printf("FIRST(%c) = { %s }\n", nonTerminals[i], firstSets[i]);
    }

    calculateFollowSets();
    printf("\nFOLLOW Sets:\n");
    for (int i = 0; i < numNonTerminals; i++) {
        printf("FOLLOW(%c) = { %s }\n", nonTerminals[i], followSets[i]);
    }

    constructParseTable();
    printf("\nPredictive Parse Table:\n");
    printf("%-10s", "");
    for (int i = 0; i < numTerminals; i++) printf("%-10s", &terminals[i]);
    printf("\n--------------------------------------------------------------------------------\n");
    for (int i = 0; i < numNonTerminals; i++) {
        printf("%-10c", nonTerminals[i]);
        for (int j = 0; j < numTerminals; j++) {
            printf("%-10s", parseTable[i][j]);
        }
        printf("\n");
    }

    // --- Experiment-7(b) ---
    printf("\n--- Starting Experiment-7(b): Parsing Input String ---\n");
    char inputString[50];
    printf("\nEnter the input string to parse (end with $): ");
    scanf("%s", inputString);
    
    parseString(inputString);

    return 0;
}