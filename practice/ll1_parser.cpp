#include <bits/stdc++.h>
using namespace std;

struct Production {
    char lhs;
    string rhs;
};

vector<Production> productions;
vector<char> nonTerminals, terminals;
vector<bool> visitedFirst, visitedFollow;

int getTerminalIndex(char c) {
    for (int i = 0; i < terminals.size(); i++) {
        if (terminals[i] == c) return i;
    }
    return -1;
}

int getNonTerminalIndex(char c) {
    for (int i = 0; i < nonTerminals.size(); i++) {
        if (nonTerminals[i] == c) return i;
    }
    return -1;
}

int getProductionIndex(Production prod) {
    for (int i = 0; i < productions.size(); i++) {
        if (productions[i].lhs == prod.lhs && productions[i].rhs == prod.rhs) {
            return i;
        }
    }
    return -1;
}

bool isTerminal(char c) {
    return (c >= 'a' && c <= 'z') || c == '$' || c == '@';
}

void addUnique(vector<char>& result, char c) {
    if (find(result.begin(), result.end(), c) == result.end()) {
        result.push_back(c);
    }
}

void findFirst(vector<char>& result, char c) {
    if (isTerminal(c) && c != '@') {
        addUnique(result, c);
        return;
    }
    int idx = c - 'A';
    if (visitedFirst[idx]) return;
    visitedFirst[idx] = true;
    
    for (const auto& prod : productions) {
        if (prod.lhs == c) {
            if (prod.rhs[0] == '@') {
                addUnique(result, '@');
            } else {
                bool containsEpsilon = true;
                int k = 0;
                while (k < prod.rhs.length() && containsEpsilon) {
                    containsEpsilon = false;
                    if (isTerminal(prod.rhs[k])) {
                        addUnique(result, prod.rhs[k]);
                        break; // Break when we find a terminal
                    } else {
                        vector<char> first;
                        findFirst(first, prod.rhs[k]);
                        bool hasEpsilon = false;
                        for (char ch : first) {
                            if (ch == '@') {
                                hasEpsilon = true;
                            } else {
                                addUnique(result, ch);
                            }
                        }
                        if (hasEpsilon) {
                            containsEpsilon = true;
                            k++;
                            if (k >= prod.rhs.length()) {
                                addUnique(result, '@');
                            }
                        } else {
                            break; // No epsilon, stop processing this production
                        }
                    }
                }
            }
        }
    }
    visitedFirst[idx] = false;
}

void findFollow(vector<char>& result, char c) {
    int idx = c - 'A';
    if (visitedFollow[idx]) return;
    visitedFollow[idx] = true;
    
    if (productions[0].lhs == c) addUnique(result, '$');
    
    for (const auto& prod: productions) {
        int len = prod.rhs.length();
        for (int i = 0; i < len; i++) {
            if (prod.rhs[i] == c) {
                if (i + 1 < len) {
                    char next = prod.rhs[i+1];
                    if (isTerminal(next)) {
                        addUnique(result, next);
                    } else {
                        // Handle non-terminal case
                        int k = i + 1;
                        bool allEpsilon = true;
                        while (k < len && allEpsilon) {
                            vector<char> tempFirst;
                            findFirst(tempFirst, prod.rhs[k]);
                            allEpsilon = false;
                            for (char ch : tempFirst) {
                                if (ch != '@') {
                                    addUnique(result, ch);
                                } else {
                                    allEpsilon = true;
                                }
                            }
                            k++;
                        }
                        // If all remaining symbols can derive epsilon, add FOLLOW of lhs
                        if (allEpsilon && prod.lhs != c) {
                            vector<char> tempFollow;
                            findFollow(tempFollow, prod.lhs);
                            for (char ch : tempFollow) {
                                addUnique(result, ch);
                            }
                        }
                    }
                } else if (prod.lhs != c) {
                    // At the end of production
                    vector<char> tempFollow;
                    findFollow(tempFollow, prod.lhs);
                    for (char ch : tempFollow) {
                        addUnique(result, ch);
                    }
                }
            }
        }
    }
    visitedFollow[idx] = false;
}

void createParseTable(vector<vector<int>>& parseTable) {
    for (const auto& prod : productions) {
        vector<char> firstSet;
        
        // Get FIRST set of the production's RHS
        if (prod.rhs[0] == '@') {
            // For epsilon production, use FOLLOW of lhs
            findFollow(firstSet, prod.lhs);
        } else {
            // For non-epsilon, get FIRST of first symbol
            findFirst(firstSet, prod.rhs[0]);
            // Check if we need to include FOLLOW for epsilon cases
            bool hasEpsilon = (find(firstSet.begin(), firstSet.end(), '@') != firstSet.end());
            if (hasEpsilon) {
                vector<char> followSet;
                findFollow(followSet, prod.lhs);
                for (char ch : followSet) {
                    addUnique(firstSet, ch);
                }
                // Remove epsilon from parse table entries
                firstSet.erase(remove(firstSet.begin(), firstSet.end(), '@'), firstSet.end());
            }
        }
        
        int nt = getNonTerminalIndex(prod.lhs);
        int pIndex = getProductionIndex(prod);
        
        for (char ch : firstSet) {
            int t = getTerminalIndex(ch);
            if (t != -1 && nt != -1) {
                if (parseTable[nt][t] != -1) {
                    cout << "Conflict in parse table at [" << nt << "][" << t << "]" << endl;
                }
                parseTable[nt][t] = pIndex;
            }
        }
    }
}

bool parseString(const vector<vector<int>>& parseTable, const string& input) {
    stack<char> st;
    st.push('$');  // End marker
    st.push(nonTerminals[0]);  // Start symbol (S)
    
    int inputPtr = 0;
    char currentInput = input[inputPtr];
    
    cout << "Parsing steps for string: " << input << endl;
    cout << "Stack\t\tInput\t\tAction" << endl;
    cout << "-----\t\t-----\t\t------" << endl;
    
    while (!st.empty()) {
        // Display current state
        stack<char> temp = st;
        vector<char> stackContents;
        while (!temp.empty()) {
            stackContents.push_back(temp.top());
            temp.pop();
        }
        reverse(stackContents.begin(), stackContents.end());
        
        string stackStr;
        for (char c : stackContents) stackStr += c;
        
        string inputStr = input.substr(inputPtr);
        if (inputStr.empty()) inputStr = "$";
        
        cout << stackStr << "\t\t" << inputStr << "\t\t";
        
        char top = st.top();
        
        if (top == currentInput) {
            if (top == '$') {
                cout << "Accept" << endl;
                return true;
            }
            cout << "Match '" << currentInput << "'" << endl;
            st.pop();
            inputPtr++;
            currentInput = (inputPtr < input.length()) ? input[inputPtr] : '$';
        }
        else if (isTerminal(top)) {
            cout << "Error: Expected '" << top << "' but found '" << currentInput << "'" << endl;
            return false;
        }
        else {
            int ntIndex = getNonTerminalIndex(top);
            int tIndex = getTerminalIndex(currentInput);
            
            if (ntIndex == -1 || tIndex == -1) {
                cout << "Error: Invalid symbol" << endl;
                return false;
            }
            
            int productionIndex = parseTable[ntIndex][tIndex];
            
            if (productionIndex == -1) {
                cout << "Error: No production for " << top << " on input '" << currentInput << "'" << endl;
                return false;
            }
            
            Production prod = productions[productionIndex];
            cout << "Apply " << prod.lhs << " -> " << prod.rhs << endl;
            
            st.pop();
            
            // Push RHS in reverse order (so leftmost is on top)
            if (prod.rhs != "@") {  // If not epsilon production
                for (int i = prod.rhs.length() - 1; i >= 0; i--) {
                    st.push(prod.rhs[i]);
                }
            }
        }
    }
    
    return false;
}

int main() {
    int n;
    cin >> n;
    cin.ignore();
    
    // Add $ to terminals for end marker
    terminals.push_back('$');
    
    for (int i = 0; i < n; i++) {
        string input;
        getline(cin, input);
        
        char lhs = input[0];
        string rhs = input.substr(2);
        
        Production prod;
        prod.lhs = lhs;
        prod.rhs = rhs;
        productions.push_back(prod);
        
        if (find(nonTerminals.begin(), nonTerminals.end(), lhs) == nonTerminals.end()) {
            nonTerminals.push_back(lhs);
        }
        
        // BUG FIX: Add terminals from RHS correctly
        for (char ch : rhs) {
            if (isTerminal(ch) && ch != '@') {
                if (find(terminals.begin(), terminals.end(), ch) == terminals.end()) {
                    terminals.push_back(ch);
                }
            }
        }
    }
    
    cout << "Productions:" << endl;
    for (Production prod : productions) {
        cout << prod.lhs << " -> " << prod.rhs << endl;
    }
    
    visitedFirst.resize(26, false);
    visitedFollow.resize(26, false);
    
    cout << "\nFIRST and FOLLOW sets:" << endl;
    for (int i = 0; i < nonTerminals.size(); i++) {
        vector<char> first, follow;
        char nt = nonTerminals[i];
        findFirst(first, nt);
        findFollow(follow, nt);
        
        cout << "First (" << nt << "): {";
        for (int j = 0; j < first.size(); j++) {
            cout << first[j];
            if (j < first.size() - 1) cout << ", ";
        }
        cout << "}" << endl;
        
        cout << "Follow (" << nt << "): {";
        for (int j = 0; j < follow.size(); j++) {
            cout << follow[j];
            if (j < follow.size() - 1) cout << ", ";
        }
        cout << "}" << endl;
    }
    
    int nt_count = nonTerminals.size(), t_count = terminals.size();
    vector<vector<int>> parseTable(nt_count, vector<int>(t_count, -1));
    createParseTable(parseTable);
    
    cout << "\nParse Table:" << endl;
    cout << "\t\t";
    for (int i = 0; i < t_count; i++) {
        cout << terminals[i] << "\t\t";
    }
    cout << endl;
    
    for (int i = 0; i < nt_count; i++) {
        cout << nonTerminals[i] << "\t\t";
        for (int j = 0; j < t_count; j++) {
            if (parseTable[i][j] != -1) {
                int pIndex = parseTable[i][j];
                cout << productions[pIndex].lhs << "->" << productions[pIndex].rhs << "\t";
            } else {
                cout << "-\t\t";
            }
        }
        cout << endl;
    }
    
    // String parsing section
    string inputString;
    cout << "\nEnter string to parse (without spaces): ";
    cin >> inputString;
    
    bool result = parseString(parseTable, inputString);
    
    if (result) {
        cout << "\n✓ String '" << inputString << "' is ACCEPTED by the grammar!" << endl;
    } else {
        cout << "\n✗ String '" << inputString << "' is REJECTED by the grammar!" << endl;
    }
    
    return 0;
}

/*
3
S=CC
C=cC
C=d
cdcd
*/