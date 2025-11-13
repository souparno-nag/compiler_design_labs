#include <bits/stdc++.h>
using namespace std;

int main() {
    vector<string> states, symbols, finals;
    map<pair<string, char>, set<string>> transitions;
    
    int n, m, f;
    cout << "Enter states, symbols, finals count: ";
    cin >> n >> m >> f;
    
    cout << "Enter states: ";
    for (int i = 0; i < n; i++) {
        string s; cin >> s;
        states.push_back(s);
    }
    
    cout << "Enter symbols: ";
    for (int i = 0; i < m; i++) {
        string s; cin >> s;
        symbols.push_back(s);
    }
    
    cout << "Enter final states: ";
    for (int i = 0; i < f; i++) {
        string s; cin >> s;
        finals.push_back(s);
    }
    
    // Clear input buffer
    cin.ignore();
    
    cout << "Enter transitions (comma-separated, use _ for empty):\n";
    for (string s : states) {
        for (string sy : symbols) {
            cout << "D(" << s << ", " << sy << "): ";
            string input;
            getline(cin, input);
            
            if (input != "_") {
                stringstream ss(input);
                string state;
                while (getline(ss, state, ',')) {
                    // Remove leading/trailing spaces
                    state.erase(0, state.find_first_not_of(" "));
                    state.erase(state.find_last_not_of(" ") + 1);
                    if (!state.empty()) {
                        transitions[{s, sy[0]}].insert(state);
                    }
                }
            }
        }
    }
    
    vector<set<string>> dfa_states;
    map<set<string>, int> state_index;
    vector<int> dfa_final;
    vector<map<char, int>> dfa_trans;
    
    set<string> start = {states[0]};
    dfa_states.push_back(start);
    state_index[start] = 0;
    
    for (int i = 0; i < dfa_states.size(); i++) {
        map<char, int> trans_map;
        for (auto& c : symbols) {
            set<string> new_state;
            for (auto& s : dfa_states[i]) {
                if (transitions.count({s, c[0]})) {
                    for (auto &next : transitions[{s, c[0]}]) {
                        new_state.insert(next);
                    }
                }
            }
            
            if (!new_state.empty()) {
                if (!state_index.count(new_state)) {
                    state_index[new_state] = dfa_states.size();
                    dfa_states.push_back(new_state);
                }
                trans_map[c[0]] = state_index[new_state];
            } else {
                trans_map[c[0]] = -1; // No transition
            }
        }
        dfa_trans.push_back(trans_map);
    }
    
    for (auto& state_set : dfa_states) {
        bool is_final = false;
        for (auto& s : state_set) {
            for (auto& f : finals) {
                if (s == f) { 
                    is_final = true; 
                    break;
                }
            }
            if (is_final) break;
        }
        dfa_final.push_back(is_final);
    }
    
    // Print DFA table
    cout << "\nDFA Transition Table:\n";
    cout << "State\t";
    for (auto& c : symbols) cout << c << "\t";
    cout << "Final\n";
    
    for (int i = 0; i < dfa_states.size(); i++) {
        cout << "{";
        auto it = dfa_states[i].begin();
        while (it != dfa_states[i].end()) {
            cout << *it;
            if (++it != dfa_states[i].end()) cout << ",";
        }
        cout << "}\t";
        
        for (auto& c : symbols) {
            if (dfa_trans[i].count(c[0]) && dfa_trans[i][c[0]] != -1) {
                cout << dfa_trans[i][c[0]] << "\t";
            } else {
                cout << "-\t";
            }
        }
        cout << (dfa_final[i] ? "Yes" : "No") << endl;
    }
    
    return 0;
}