#include <bits/stdc++.h>
using namespace std;

struct P {
    char l; string r;
};
vector<P> prods;
vector<char> nts, ts;
map<char, vector<char>> first, follow;

bool isT (char c) {return !(isupper(c));}

void addU(vector<char>& v, char c) {
    if (find(v.begin(), v.end(), c) == v.end()) v.push_back(c);
}

void firstSet(char nt) {
    if (!first[nt].empty()) return;
    for (auto& p : prods) {
        if (p.l == nt) {
            if (p.r == "@") {
                addU(first[nt], '@');
            } else {
                bool allE = true;
                for (char c : p.r) {
                    if (isT(c)) {
                        addU(first[nt], c);
                        break;
                    }
                    firstSet(c);
                    bool hasE = false;
                    for (char f : first[c]) {
                        if (f == '@') hasE = true;
                        else addU(first[nt], f);
                    }
                    if (!hasE) {allE = false; break;}
                }
                if (allE) addU(first[nt], '@');
            }
        }
    }
}

void followSet (char nt) {
    if (!follow[nt].empty()) return;
    if (prods[0].l == nt) addU(follow[nt], '$');
    for (auto& p : prods)  {
        if (find(p.r.begin(), p.r.end(), nt) != p.r.end()) {
            int i = find(p.r.begin(), p.r.end(), nt) - p.r.begin();
            if (i < p.r.size()-1) {
                char n = p.r[i+1];
                if (isT(n)) addU(follow[nt], n);
                else {
                    bool allE = true;
                    for (int j = i+1; j < p.r.size() && allE; j++) {
                        firstSet(p.r[j]);
                        allE = false;
                        for (char f : first[p.r[j]]) {
                            if (f == '@') allE = true;
                            else addU(follow[nt], f);
                        }
                    }
                    if (allE) {
                        followSet(p.l);
                        for (char f : follow[p.l]) addU(follow[nt], f);
                    }
                }
            } else {
                followSet(p.l);
                for (char f : follow[p.l]) addU (follow[nt], f);
            }
        }
    }
}

vector<vector<int>> makeTable() {
    vector<vector<int>> tab(nts.size(), vector<int> (ts.size(), -1));
    for (int i = 0; i < prods.size(); i++) {
        auto& p = prods[i];
        vector<char> fs;
        if (p.r == "@") fs = follow[p.l];
        else {
            bool allE = true;
            for (char c : p.r) {
                if (isT(c)) {
                    if (c != '@') addU(fs, c);
                    allE = (c == '@');
                    break;
                }
                allE = false;
                for (char f : first[c]) {
                    if (f != '@') addU(fs, f);
                    else allE = true;
                }
                if (!allE) break;
            }
            if (allE) {
                for (char f : follow[p.l]) addU(fs, f);  
            }
        }
        int ni = find(nts.begin(), nts.end(), p.l) - nts.begin();
        for (char c : fs) {
            if (c == '@') continue;
            int ti = find(ts.begin(), ts.end(), c) - ts.begin();
            if (ti < ts.size() && tab[ni][ti] == -1) tab[ni][ti] = i;
        }
    }
    return tab;
}

bool parse(vector<vector<int>>& tab, string s) {
    stack<char> st; st.push('$'); st.push(nts[0]); s += '$';
    int i = 0;
    cout << "Parsing:\nStack\tInput\tAction\n";
    while(!st.empty()) {
        string ss, is = s.substr(i);
        for(stack<char> t = st; !t.empty(); t.pop()) ss = string(1, t.top()) + ss;
        cout << ss << "\t" << is << "\t";
        
        if(st.top() == s[i]) {
            if(st.top() == '$') { cout << "Accept\n"; return true; }
            cout << "Match " << st.top() << endl;
            st.pop(); i++;
        } else if(isT(st.top())) {
            cout << "Error\n"; return false;
        } else {
            int ni = find(nts.begin(), nts.end(), st.top()) - nts.begin();
            int ti = find(ts.begin(), ts.end(), s[i]) - ts.begin();
            if(ni >= nts.size() || ti >= ts.size() || tab[ni][ti] == -1) {
                cout << "Error\n"; return false;
            }
            auto& p = prods[tab[ni][ti]];
            cout << p.l << "->" << p.r << endl;
            st.pop();
            if(p.r != "@") 
                for(int j = p.r.size()-1; j >= 0; j--) st.push(p.r[j]);
        }
    }
    return false;
}

int main() {
    int n; cin >> n; cin.ignore(); ts.push_back('$');
    for(int i = 0; i < n; i++) {
        string s; getline(cin, s);
        prods.push_back({s[0], s.substr(2)});
        addU(nts, s[0]);
        for(char c : s.substr(2)) if(isT(c) && c != '@') addU(ts, c);
    }
    
    for(char nt : nts) firstSet(nt);
    for(char nt : nts) followSet(nt);
    
    auto tab = makeTable();
    string inp; cout << "String: "; cin >> inp;
    cout << (parse(tab, inp) ? "ACCEPT" : "REJECT") << endl;
    return 0;
}