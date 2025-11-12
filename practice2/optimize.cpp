#include <bits/stdc++.h>
using namespace std;

int main() {
    vector<string> v; string s;
    while (getline(cin, s)) {
        if (!s.empty()) {
            v.push_back(s); 
        }
    }
    map<string, int> m;
    for (string l : v) {
        if (l.empty() || l.find('=') == -1) {
            cout << l << endl;
            continue;
        }
        string a = l.substr(0, l.find('=')), b = l.substr(l.find('=')+1);
        a.erase(remove(a.begin(), a.end(), ' '), a.end());
        b.erase(remove(b.begin(), b.end(), ' '), b.end());
        
        if (b.find('+') != -1) {
            string x = b.substr(0, b.find('+')), y = b.substr(b.find('+')+1);
            if (m.count(x)) x = to_string(m[x]);
            if (m.count(y)) y = to_string(m[y]);
            if (isdigit(x[0]) && isdigit(y[0])) {m[a] = stoll(x) + stoll(y); cout << a << " = " << m[a] << endl;}
            else cout << a << " = " << x << " + " << y << endl;
        } else if (b.find('-') != -1) {
            string x = b.substr(0, b.find('-')), y = b.substr(b.find('-')+1);
            if (m.count(x)) x = to_string(m[x]);
            if (m.count(y)) y = to_string(m[y]);
            if (isdigit(x[0]) && isdigit(y[0])) {m[a] = stoll(x) - stoll(y); cout << a << " = " << m[a] << endl;}
            else cout << a << " = " << x << " - " << y << endl;
        } else if (b.find('*') != -1) {
            string x = b.substr(0, b.find('*')), y = b.substr(b.find('*')+1);
            if (m.count(x)) x = to_string(m[x]);
            if (m.count(y)) y = to_string(m[y]);
            if (isdigit(x[0]) && isdigit(y[0])) {m[a] = stoll(x) * stoll(y); cout << a << " = " << m[a] << endl;}
            else cout << a << " = " << x << " * " << y << endl;
        } else if (b.find('/') != -1) {
            string x = b.substr(0, b.find('/')), y = b.substr(b.find('/')+1);
            if (m.count(x)) x = to_string(m[x]);
            if (m.count(y)) y = to_string(m[y]);
            if (isdigit(x[0]) && isdigit(y[0])) {m[a] = stoll(x) / stoll(y); cout << a << " = " << m[a] << endl;}
            else cout << a << " = " << x << " / " << y << endl;
        } else {
            if (isdigit(b[0])) m[a] = stoll(b);
            else if (m.count(b)) {m[a] = m[b]; cout << a << " = " << m[a] << endl;}
            else cout << a << " = " << b << endl;
        }
    }
}