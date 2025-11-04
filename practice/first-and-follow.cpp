#include <bits/stdc++.h>
using namespace std;

#define MAX 50

struct Production {
    char lhs;
    string rhs;
};

vector<Production> prod;
vector<char> nonTerminals;
vector<bool> visitedFirst, visitedFollow;

// check if character is a terminal symbol
bool isTerminal(char c) {
    return !(c >= 'A' && c <= 'Z');
}

// ensure no duplicate characters in first/follow sets
void addUnique (string& set, char c) {
    if (set.find(c) != string::npos) {
        set += c;
    }
}

void findFirst (string& result, char c) {
    // if it is a terminal, add it
    if (isTerminal(c) && c != '@') {
        addUnique(result, c);
        return;
    }
    // prevent infinite recursion
    int idx = c - 'A';
    if (visitedFirst[idx]) return;
    visitedFirst[idx] = true;
    // find all productions with this non-terminal on left side
    for (const auto& p : prod) {
        if (p.lhs == c) {
            if (p.rhs[0] == '@') {
                addUnique(result, '@');
            } else {
                int k = 0;
                bool epsilonChain = true;

                // process each symbol until no more epsilon
                while (k < p.rhs.length() && epsilonChain)
                {
                    epsilonChain = false;
                    if (isTerminal(p.rhs[k])) { // if terminal
                        addUnique(result, p.rhs[k]); // add to first
                        break;
                    } else {
                        string temp;
                        findFirst(temp, p.rhs[k]);

                        bool hasEps = false;
                        for (char ch: temp) {
                            if (ch != '@') {
                                addUnique(result, ch);
                            } else {
                                hasEps = true;
                            }
                        }
                        if (hasEps) { // if current symbol can be epsilon
                            epsilonChain = true; // check next symbol
                            k++;
                            if (k >= p.rhs.length()) { // if all symbols can be epsilon
                                addUnique(result, '@'); // add epsilon to first
                            }
                        }
                    }
                }
            }
        }
    }
    visitedFirst[idx] = false; // reset for other calculations
    return;
}

void findFollow(string& result, char c) {
    int idx = c - 'A';
    // prevent infinite recursion
    if (visitedFollow[idx]) return;
    visitedFollow[idx] = true;

    // rule 1: start symbol gets '$'
    if (prod[0].lhs == c) addUnique(result, '$');

    for (const auto& p : prod) {
        int len = p.rhs.length();
        for (int j = 0; j < len; j++) {
            if (p.rhs[j] == c) {
                if (j + 1 < len) {
                    char next = p.rhs[j+1];
                    if (isTerminal(next)) { // if next is terminal
                        addUnique(result, next); // add to Follow
                    } else { // if next is non-terminal
                        string temp;
                        findFirst(temp, next); // add first of next symbol

                        bool hasEps = false;
                        for (char ch : temp) {
                            if (ch != '@') {
                                addUnique(result, ch);
                            } else {
                                hasEps = true;
                            }
                        }
                        // if next can be epsilon, add follow
                        if (hasEps && p.lhs != c) {
                            string tempFollow;
                            findFollow(tempFollow, p.lhs);
                            for (char ch : tempFollow) {
                                addUnique(result, ch);
                            }
                        }
                    }
                }
            }
        }
    }
    visitedFollow[idx] = false;
}

int main() {
    int p;
    cout << "Enter number of productions: ";
    cin >> p;
    cin.ignore();

    cout << "Enter production (use @ for epsilon)" << endl;
    for (int i = 0; i < p; i++) {
        
    }
    return 0;
}