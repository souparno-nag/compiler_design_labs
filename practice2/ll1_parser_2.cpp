#include <bits/stdc++.h>
using namespace std;

struct Production { char lhs; string rhs; };
vector<Production> productions;
vector<char> nonTerminals, terminals;
map<char, vector<char>> first, follow;

bool isTerminal(char c) { return !isupper(c); }

void addUnique(vector<char>& v, char c) {
    if (find(v.begin(), v.end(), c) == v.end()) v.push_back(c);
}

void computeFirst(char nt) {
    if (!first[nt].empty()) return;
    
    for (auto& prod : productions) {
        if (prod.lhs == nt) {
            if (prod.rhs == "@") addUnique(first[nt], '@');
            else {
                bool allEpsilon = true;
                for (char c : prod.rhs) {
                    if (isTerminal(c)) {
                        addUnique(first[nt], c);
                        allEpsilon = false;
                        break;
                    } else {
                        computeFirst(c);
                        bool hasEpsilon = false;
                        for (char f : first[c]) {
                            if (f == '@') hasEpsilon = true;
                            else addUnique(first[nt], f);
                        }
                        if (!hasEpsilon) { allEpsilon = false; break; }
                    }
                }
                if (allEpsilon) addUnique(first[nt], '@');
            }
        }
    }
}

void computeFollow(char nt) {
    if (!follow[nt].empty()) return;
    
    if (nt == productions[0].lhs) addUnique(follow[nt], '$');
    
    for (auto& prod : productions) {
        for (int i = 0; i < prod.rhs.size(); i++) {
            if (prod.rhs[i] == nt) {
                if (i + 1 < prod.rhs.size()) {
                    char next = prod.rhs[i + 1];
                    if (isTerminal(next)) addUnique(follow[nt], next);
                    else {
                        bool allEpsilon = true;
                        for (int j = i + 1; j < prod.rhs.size() && allEpsilon; j++) {
                            computeFirst(prod.rhs[j]);
                            allEpsilon = false;
                            for (char f : first[prod.rhs[j]]) {
                                if (f == '@') allEpsilon = true;
                                else addUnique(follow[nt], f);
                            }
                        }
                        if (allEpsilon) {
                            computeFollow(prod.lhs);
                            for (char f : follow[prod.lhs]) addUnique(follow[nt], f);
                        }
                    }
                } else if (prod.lhs != nt) {
                    computeFollow(prod.lhs);
                    for (char f : follow[prod.lhs]) addUnique(follow[nt], f);
                }
            }
        }
    }
}

vector<vector<int>> createParseTable() {
    vector<vector<int>> table(nonTerminals.size(), vector<int>(terminals.size(), -1));
    
    for (int i = 0; i < productions.size(); i++) {
        auto& prod = productions[i];
        vector<char> firstSet;
        
        if (prod.rhs == "@") firstSet = follow[prod.lhs];
        else {
            bool allEpsilon = true;
            for (char c : prod.rhs) {
                if (isTerminal(c)) {
                    if (c != '@') addUnique(firstSet, c);
                    allEpsilon = (c == '@');
                    break;
                } else {
                    allEpsilon = false;
                    for (char f : first[c]) {
                        if (f != '@') addUnique(firstSet, f);
                        else allEpsilon = true;
                    }
                    if (!allEpsilon) break;
                }
            }
            if (allEpsilon) 
                for (char f : follow[prod.lhs]) addUnique(firstSet, f);
        }
        
        int ntIdx = find(nonTerminals.begin(), nonTerminals.end(), prod.lhs) - nonTerminals.begin();
        for (char c : firstSet) {
            if (c == '@') continue;
            int tIdx = find(terminals.begin(), terminals.end(), c) - terminals.begin();
            if (tIdx < terminals.size() && table[ntIdx][tIdx] == -1) 
                table[ntIdx][tIdx] = i;
        }
    }
    return table;
}

bool parseString(vector<vector<int>>& table, string input) {
    stack<char> st;
    st.push('$'); st.push(nonTerminals[0]);
    input += '$';
    int ptr = 0;
    
    cout << "Parsing:\nStack\t\tInput\t\tAction\n";
    
    while (!st.empty()) {
        string stackStr, inputStr = input.substr(ptr);
        for (stack<char> temp = st; !temp.empty(); temp.pop()) stackStr = string(1, temp.top()) + stackStr;
        
        cout << stackStr << "\t\t" << inputStr << "\t\t";
        
        if (st.top() == input[ptr]) {
            if (st.top() == '$') { cout << "Accept\n"; return true; }
            cout << "Match '" << st.top() << "'\n";
            st.pop(); ptr++;
        } else if (isTerminal(st.top())) {
            cout << "Error\n"; return false;
        } else {
            int nt = find(nonTerminals.begin(), nonTerminals.end(), st.top()) - nonTerminals.begin();
            int t = find(terminals.begin(), terminals.end(), input[ptr]) - terminals.begin();
            
            if (nt >= nonTerminals.size() || t >= terminals.size() || table[nt][t] == -1) {
                cout << "Error\n"; return false;
            }
            
            auto& prod = productions[table[nt][t]];
            cout << prod.lhs << "->" << prod.rhs << "\n";
            st.pop();
            if (prod.rhs != "@") 
                for (int i = prod.rhs.size() - 1; i >= 0; i--) st.push(prod.rhs[i]);
        }
    }
    return false;
}

int main() {
    int n; cin >> n; cin.ignore();
    terminals.push_back('$');
    
    for (int i = 0; i < n; i++) {
        string s; getline(cin, s);
        productions.push_back({s[0], s.substr(2)});
        addUnique(nonTerminals, s[0]);
        for (char c : s.substr(2)) 
            if (isTerminal(c) && c != '@') addUnique(terminals, c);
    }
    
    cout << "Productions:\n";
    for (auto& p : productions) cout << p.lhs << " -> " << p.rhs << endl;
    
    for (char nt : nonTerminals) computeFirst(nt);
    for (char nt : nonTerminals) computeFollow(nt);
    
    cout << "\nFIRST & FOLLOW:\n";
    for (char nt : nonTerminals) {
        cout << "First(" << nt << "): {";
        for (int i = 0; i < first[nt].size(); i++) 
            cout << first[nt][i] << (i < first[nt].size()-1 ? ", " : "}\n");
        cout << "Follow(" << nt << "): {";
        for (int i = 0; i < follow[nt].size(); i++) 
            cout << follow[nt][i] << (i < follow[nt].size()-1 ? ", " : "}\n");
    }
    
    auto parseTable = createParseTable();
    cout << "\nParse Table:\n\t";
    for (char t : terminals) cout << t << "\t";
    cout << endl;
    
    for (int i = 0; i < nonTerminals.size(); i++) {
        cout << nonTerminals[i] << "\t";
        for (int j = 0; j < terminals.size(); j++) {
            if (parseTable[i][j] != -1) 
                cout << productions[parseTable[i][j]].lhs << "->" 
                     << productions[parseTable[i][j]].rhs << "\t";
            else cout << "-\t";
        }
        cout << endl;
    }
    
    string input; cout << "\nEnter string: "; cin >> input;
    bool result = parseString(parseTable, input);
    cout << (result ? "✓ ACCEPTED" : "✗ REJECTED") << endl;
    
    return 0;
}