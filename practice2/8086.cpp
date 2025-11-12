#include <iostream>
#include <vector>
#include <set>
#include <sstream>
using namespace std;

bool isNum(string s) {
    for(char c:s) if(!isdigit(c)) return false;
    return !s.empty();
}

int main() {
    vector<string> tac;
    string line;
    set<string> vars;
    
    while(getline(cin, line)) 
        if(!line.empty()) tac.push_back(line);
    
    for(auto l:tac) {
        stringstream ss(l);
        string t;
        while(ss>>t) 
            if(!isNum(t) && t!="=" && t!="+" && t!="-" && t!="*" && t!="/")
                vars.insert(t);
    }
    
    for(auto l:tac) {
        cout<<"; "<<l<<"\n";
        string d,o,s1,op,s2;
        stringstream ss(l);
        ss>>d>>o>>s1>>op>>s2;
        
        cout<<"MOV AX,"<<s1<<"\n";
        if(op=="+") cout<<"ADD AX,"<<s2<<"\n";
        else if(op=="-") cout<<"SUB AX,"<<s2<<"\n";
        else if(op=="*") {cout<<"MOV BX,"<<s2<<"\nMUL AX, BX\n";}
        else if(op=="/") {cout<<"MOV DX,0\nMOV BX,"<<s2<<"\nDIV BX\n";}
        cout<<"MOV "<<d<<",AX\n\n";
    }
    
    cout<<"END";
}