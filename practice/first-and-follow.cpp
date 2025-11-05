#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>

using namespace std;

#define MAX 50

struct Production {
    char lhs;
    string rhs;
};

vector<Production> prod;
vector<char> nonTerminals;
vector<bool> visitedFirst, visitedFollow;

bool isTerminal(char c) {
    return !(c >= 'A' && c <= 'Z');
}

void addUnique(string& set, char c) {
    if (set.find(c) == string::npos) {
        set += c;
    }
}

void findFirst(string& result, char c) {
    if (isTerminal(c) && c != '@') {
        addUnique(result, c);
        return;
    }

    int idx = c - 'A';
    if (visitedFirst[idx]) return;
    visitedFirst[idx] = true;

    for (const auto& p : prod) {
        if (p.lhs == c) {
            if (p.rhs[0] == '@') {
                addUnique(result, '@');
            } else {
                int k = 0;
                bool epsilonChain = true;
                while (k < p.rhs.length() && epsilonChain) {
                    epsilonChain = false;
                    if (isTerminal(p.rhs[k])) {
                        addUnique(result, p.rhs[k]);
                        break;
                    } else {
                        string temp;
                        findFirst(temp, p.rhs[k]);
                        bool hasEps = false;
                        for (char ch : temp) {
                            if (ch != '@')
                                addUnique(result, ch);
                            else
                                hasEps = true;
                        }
                        if (hasEps) {
                            epsilonChain = true;
                            k++;
                            if (k >= p.rhs.length())
                                addUnique(result, '@');
                        }
                    }
                }
            }
        }
    }
    visitedFirst[idx] = false;
}

void findFollow(string& result, char c) {
    int idx = c - 'A';
    if (visitedFollow[idx]) return;
    visitedFollow[idx] = true;

    if (prod[0].lhs == c) addUnique(result, '$');

    for (const auto& p : prod) {
        size_t len = p.rhs.length();
        for (size_t j = 0; j < len; j++) {
            if (p.rhs[j] == c) {
                if (j + 1 < len) {
                    char next = p.rhs[j + 1];
                    if (isTerminal(next)) {
                        addUnique(result, next);
                    } else {
                        string temp;
                        findFirst(temp, next);
                        bool hasEps = false;
                        for (char ch : temp) {
                            if (ch != '@')
                                addUnique(result, ch);
                            else
                                hasEps = true;
                        }
                        if (hasEps && p.lhs != c) {
                            string tempFollow;
                            findFollow(tempFollow, p.lhs);
                            for (char ch : tempFollow)
                                addUnique(result, ch);
                        }
                    }
                } else if (p.lhs != c) {
                    string tempFollow;
                    findFollow(tempFollow, p.lhs);
                    for (char ch : tempFollow)
                        addUnique(result, ch);
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

    cout << "Enter productions (use @ for epsilon):\n";
    for (int i = 0; i < p; i++) {
        string input;
        getline(cin, input);

        char lhs = input[0];
        size_t equalPos = input.find('=');
        string rhsPart = input.substr(equalPos + 1);

        size_t start = 0;
        size_t end = rhsPart.find('|');
        
        while (true) {
            string token = rhsPart.substr(start, end - start);
            
            Production production;
            production.lhs = lhs;
            production.rhs = token;
            prod.push_back(production);
            
            if (find(nonTerminals.begin(), nonTerminals.end(), lhs) == nonTerminals.end()) {
                nonTerminals.push_back(lhs);
            }
            
            if (end == string::npos) break;
            start = end + 1;
            end = rhsPart.find('|', start);
        }
    }

    // Initialize visited arrays
    visitedFirst.resize(26, false);
    visitedFollow.resize(26, false);

    cout << "\nFIRST and FOLLOW sets:\n";
    for (char nt : nonTerminals) {
        fill(visitedFirst.begin(), visitedFirst.end(), false);
        fill(visitedFollow.begin(), visitedFollow.end(), false);

        string first, follow;
        findFirst(first, nt);
        findFollow(follow, nt);

        cout << "FIRST(" << nt << ") = { ";
        for (char ch : first) cout << ch << " ";
        cout << "}" << endl;
        
        cout << "FOLLOW(" << nt << ") = { ";
        for (char ch : follow) cout << ch << " ";
        cout << "}" << endl;
    }
    
    return 0;
}