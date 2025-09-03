#include <stdio.h>
#include <string.h>
#include <ctype.h>

// --- Global Variables & Structures ---
char productions[20][25];
char terminals[20], non_terminals[20];
int num_prods = 0, num_terms = 0, num_non_terms = 0;

char leading_sets[20][20];
char trailing_sets[20][20];
char precedence_table[20][20];

// --- Helper Functions ---

// Function to add a character to a set if not already present
void addToSet(char *set, char c) {
    if (strchr(set, c) == NULL) {
        set[strlen(set)] = c;
    }
}

// Function to find the index of a character in an array
int getIndex(char c, const char *arr) {
    char *e = strchr(arr, c);
    if (e == NULL) return -1;
    return (int)(e - arr);
}


// --- Core Logic: Calculating LEADING and TRAILING ---

void findFirstAndFollow() {
    // 1. Identify terminals and non-terminals
    addToSet(terminals, '$'); // Add dollar sign to terminals
    for (int i = 0; i < num_prods; i++) {
        addToSet(non_terminals, productions[i][0]);
    }
    num_non_terms = strlen(non_terminals);

    for (int i = 0; i < num_prods; i++) {
        for (int j = 3; j < strlen(productions[i]); j++) {
            if (!isupper(productions[i][j])) {
                 addToSet(terminals, productions[i][j]);
            }
        }
    }
    num_terms = strlen(terminals);

    // 2. Calculate LEADING sets
    for (int i = 0; i < num_non_terms; i++) {
        for (int j = 0; j < num_prods; j++) {
            if (non_terminals[i] == productions[j][0]) {
                if (!isupper(productions[j][3])) {
                    addToSet(leading_sets[i], productions[j][3]);
                } else {
                     if (strlen(productions[j]) > 4 && !isupper(productions[j][4])) {
                        addToSet(leading_sets[i], productions[j][4]);
                     }
                }
            }
        }
    }
    
    // Iteratively complete LEADING sets
    int changed;
    do {
        changed = 0;
        for (int i = 0; i < num_non_terms; i++) {
             for (int j = 0; j < num_prods; j++) {
                if (non_terminals[i] == productions[j][0]) {
                    if (isupper(productions[j][3])) {
                        int nt_index = getIndex(productions[j][3], non_terminals);
                        for(int k = 0; k < strlen(leading_sets[nt_index]); k++) {
                            if(strchr(leading_sets[i], leading_sets[nt_index][k]) == NULL) {
                                addToSet(leading_sets[i], leading_sets[nt_index][k]);
                                changed = 1;
                            }
                        }
                    }
                }
             }
        }
    } while(changed);


    // 3. Calculate TRAILING sets
    for (int i = 0; i < num_non_terms; i++) {
        for (int j = 0; j < num_prods; j++) {
            if (non_terminals[i] == productions[j][0]) {
                char last_char = productions[j][strlen(productions[j]) - 1];
                if (!isupper(last_char)) {
                    addToSet(trailing_sets[i], last_char);
                } else {
                    if (strlen(productions[j]) > 4) {
                        char second_last = productions[j][strlen(productions[j]) - 2];
                        if(!isupper(second_last)){
                             addToSet(trailing_sets[i], second_last);
                        }
                    }
                }
            }
        }
    }
    
    // Iteratively complete TRAILING sets
    do {
        changed = 0;
        for (int i = 0; i < num_non_terms; i++) {
             for (int j = 0; j < num_prods; j++) {
                if (non_terminals[i] == productions[j][0]) {
                    char last_char = productions[j][strlen(productions[j]) - 1];
                    if (isupper(last_char)) {
                        int nt_index = getIndex(last_char, non_terminals);
                        for(int k = 0; k < strlen(trailing_sets[nt_index]); k++) {
                            if(strchr(trailing_sets[i], trailing_sets[nt_index][k]) == NULL) {
                                addToSet(trailing_sets[i], trailing_sets[nt_index][k]);
                                changed = 1;
                            }
                        }
                    }
                }
             }
        }
    } while(changed);
}

// --- Core Logic: Building the Precedence Table ---

void generatePrecedenceTable() {
    // Initialize table with spaces (Error)
    for(int i = 0; i < num_terms; i++) {
        for(int j = 0; j < num_terms; j++) {
            precedence_table[i][j] = ' ';
        }
    }
    
    // Apply precedence rules
    for (int i = 0; i < num_prods; i++) {
        char *rhs = productions[i] + 3; // Pointer to RHS
        for (int j = 0; j < strlen(rhs) - 1; j++) {
            char op1 = rhs[j];
            char op2 = rhs[j+1];
            int op1_idx = getIndex(op1, terminals);
            int op2_idx = getIndex(op2, terminals);

            // Rule 1: A -> ... op1 op2 ...  (op1 .> op2)
            if (op1_idx != -1 && op2_idx != -1) {
                precedence_table[op1_idx][op2_idx] = '=';
            }

            // Rule 2: A -> ... op1 B ...  (op1 <. LEAD(B))
            if (op1_idx != -1 && isupper(op2)) {
                int nt_idx = getIndex(op2, non_terminals);
                for (int k = 0; k < strlen(leading_sets[nt_idx]); k++) {
                    int lead_idx = getIndex(leading_sets[nt_idx][k], terminals);
                    precedence_table[op1_idx][lead_idx] = '<';
                }
            }
            
            // Rule 3: A -> ... B op2 ... (TRAIL(B) .> op2)
            if (isupper(op1) && op2_idx != -1) {
                int nt_idx = getIndex(op1, non_terminals);
                 for (int k = 0; k < strlen(trailing_sets[nt_idx]); k++) {
                    int trail_idx = getIndex(trailing_sets[nt_idx][k], terminals);
                    precedence_table[trail_idx][op2_idx] = '>';
                }
            }
            
            // Rule for A -> ... op1 B op2 ...
            if(j < strlen(rhs) - 2) {
                 char op3 = rhs[j+2];
                 int op3_idx = getIndex(op3, terminals);
                 if(op1_idx != -1 && isupper(op2) && op3_idx != -1) {
                    precedence_table[op1_idx][op3_idx] = '=';
                 }
            }
        }
    }
    
    // Handle '$' using start symbol (assumed to be the first one)
    int start_symbol_idx = 0; // E.g., E
    int dollar_idx = getIndex('$', terminals);
    
    // $ <. LEADING(S)
    for(int i = 0; i < strlen(leading_sets[start_symbol_idx]); i++) {
        int lead_idx = getIndex(leading_sets[start_symbol_idx][i], terminals);
        precedence_table[dollar_idx][lead_idx] = '<';
    }

    // TRAILING(S) .> $
    for(int i = 0; i < strlen(trailing_sets[start_symbol_idx]); i++) {
        int trail_idx = getIndex(trailing_sets[start_symbol_idx][i], terminals);
        precedence_table[trail_idx][dollar_idx] = '>';
    }
}


// --- Core Logic: Parsing the String ---

void parseString(char *input) {
    char stack[50] = "$";
    strcat(input, "$");
    int input_ptr = 0;

    printf("\n--- Parsing Steps ---\n");
    printf("%-20s %-20s %-10s\n", "Stack", "Input", "Action");
    printf("----------------------------------------------------\n");

    while (input_ptr < strlen(input)) {
        printf("%-20s %-20s ", stack, input + input_ptr);

        // Find top-most terminal on stack
        int stack_top_idx = strlen(stack) - 1;
        while (isupper(stack[stack_top_idx])) {
            stack_top_idx--;
        }
        char stack_top_term = stack[stack_top_idx];
        
        char current_input = input[input_ptr];

        int row = getIndex(stack_top_term, terminals);
        int col = getIndex(current_input, terminals);

        if (row == -1 || col == -1) {
            printf("ERROR: Invalid symbol\n");
            break;
        }

        char relation = precedence_table[row][col];

        if (relation == '<' || relation == '=') {
            printf("SHIFT %c\n", current_input);
            stack[strlen(stack)] = current_input;
            input_ptr++;
        } else if (relation == '>') {
            printf("REDUCE\n");
            char handle[20] = "";
            int handle_start_idx;

            // Step 1: Find the right-most terminal on the stack to start the scan
            int right_term_idx = strlen(stack) - 1;
            while (isupper(stack[right_term_idx])) {
                right_term_idx--;
            }

            // Step 2: Scan left from the right-most terminal to find the start of the handle
            while (1) {
                // Find the terminal to the left of our current position
                int left_term_idx = right_term_idx - 1;
                while (left_term_idx >= 0 && isupper(stack[left_term_idx])) {
                    left_term_idx--;
                }
                
                // If we've hit the beginning of the stack, the handle starts at index 1 (after '$')
                if (left_term_idx < 0) { 
                    handle_start_idx = 1;
                    break;
                }

                // Get the two adjacent terminals
                char left_term = stack[left_term_idx];
                char right_term = stack[right_term_idx];

                int row = getIndex(left_term, terminals);
                int col = getIndex(right_term, terminals);

                // If relation is '<', we found the beginning of the handle
                if (precedence_table[row][col] == '<') {
                    handle_start_idx = left_term_idx + 1;
                    break;
                }
                
                // Otherwise, continue scanning left
                right_term_idx = left_term_idx;
            }

            // Step 3: Extract the handle and truncate the stack
            strcpy(handle, stack + handle_start_idx);
            stack[handle_start_idx] = '\0';
            
            // Step 4: Push the corresponding non-terminal (generic 'E' is used here)
            // Note: A more robust parser would check 'handle' to decide whether to push E, T, or F.
            strcat(stack, "E");

        } else {
             printf("ERROR: No relation\n");
             break;
        }
        
        if (strcmp(stack, "$E") == 0 && input[input_ptr] == '$') {
             printf("%-20s %-20s ACCEPTED\n", stack, input + input_ptr);
             return;
        }
    }
    printf("\nString REJECTED.\n");
}


// --- Display and Main Functions ---

void displayInfo() {
    printf("\n--- Grammar Info ---\n");
    printf("Terminals    : %s\n", terminals);
    printf("Non-Terminals: %s\n", non_terminals);

    printf("\n--- LEADING Sets ---\n");
    for (int i = 0; i < num_non_terms; i++) {
        printf("LEADING(%c) = { %s }\n", non_terminals[i], leading_sets[i]);
    }

    printf("\n--- TRAILING Sets ---\n");
    for (int i = 0; i < num_non_terms; i++) {
        printf("TRAILING(%c) = { %s }\n", non_terminals[i], trailing_sets[i]);
    }

    printf("\n--- Operator Precedence Table ---\n");
    printf("      ");
    for (int i = 0; i < num_terms; i++) printf("%-4c", terminals[i]);
    printf("\n----------------------------------------\n");
    for (int i = 0; i < num_terms; i++) {
        printf("%-4c| ", terminals[i]);
        for (int j = 0; j < num_terms; j++) {
            printf("%-4c", precedence_table[i][j]);
        }
        printf("\n");
    }
}


int main() {
    printf("Enter the number of productions: ");
    scanf("%d", &num_prods);
    printf("Enter the productions (e.g., E->E+T, use 'i' for id):\n");
    for (int i = 0; i < num_prods; i++) {
        scanf("%s", productions[i]);
    }
    
    // Example Grammar:
    // num_prods = 3;
    // strcpy(productions[0], "E->E+T");
    // strcpy(productions[1], "E->T");
    // strcpy(productions[2], "T->i");

    findFirstAndFollow();
    generatePrecedenceTable();
    displayInfo();

    char input_string[50];
    printf("\nEnter the string to parse (e.g., i+i): ");
    scanf("%s", input_string);
    parseString(input_string);

    return 0;
}