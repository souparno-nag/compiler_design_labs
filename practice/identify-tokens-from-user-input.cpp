#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <ctype.h>
#include <ios>
#include <fstream>
using namespace std;

vector<string> keywords = {"int", "main", "return", "for"};
vector<char> punctuations = {';', '.', ':'};
vector<char> assignmentOperators = {'=', '(', ')', '{', '}'};
vector<char> arithmeticOperators = {'+', '-', '*', '/'};
vector<char> logicalOperators = {'<', '>'};

bool identifyKeywords (string word) {
    int n = keywords.size();
    for (int i = 0; i < n; i++) {
        if (keywords[i] == word) {
            return true;
        }
    }
    return false;
}

bool identifyPunctuations (char ch) {
    int n = punctuations.size();
    for (int i = 0; i < n; i++) {
        if (punctuations[i] == ch) {
            return true;
        }
    }
    return false;
}

bool identifyArithmeticOperators (char ch) {
    int n = arithmeticOperators.size();
    for (int i = 0; i < n; i++) {
        if (arithmeticOperators[i] == ch) {
            return true;
        }
    }
    return false;
}

bool identifyAssignmentOperators (char ch) {
    int n = assignmentOperators.size();
    for (int i = 0; i < n; i++) {
        if (assignmentOperators[i] == ch) {
            return true;
        }
    }
    return false;
}

bool identifyLogicalOperators (char ch) {
    int n = logicalOperators.size();
    for (int i = 0; i < n; i++) {
        if (logicalOperators[i] == ch) {
            return true;
        }
    }
    return false;
}

void identifyTokens(string input) {
    string buffer = "";
    int i = 0;
    while (input[i] != '\0') {
        if (isspace(input[i]) || (input[i] == '\n')) {
            i++;
        }
        if (isalpha(input[i]) || input[i] == '_') {
            while (isalnum(input[i])) {
                buffer += input[i++];
            }
            if (identifyKeywords(buffer)) {
                cout << buffer << " is a keyword" << endl;
            } else {
                cout << buffer << " is an identifier" << endl;
            }
            buffer = "\0";
        } else if (isdigit(input[i])) {
            while (isdigit(input[i]))
            {
                buffer += input[i++];
            }
            cout << buffer << " is a number" << endl;
            buffer = "\0";
        } else if (identifyPunctuations(input[i])) {
            cout << input[i++] << " is a punctuation" << endl;
        }  else if (identifyArithmeticOperators(input[i])) {
            cout << input[i++] << " is an arithmetic operator" << endl;
        }  else if (identifyAssignmentOperators(input[i])) {
            cout << input[i++] << " is an assignment operator" << endl;
        } else if (identifyLogicalOperators(input[i])) {
            cout << input[i++] << " is a logical operator" << endl;
        }
    }
    return;
}

void analyzeFile (string filename) {
    fstream file(filename, ios::in);
    string line;
    while (getline(file, line))
    {
        identifyTokens(line);
    }    
}

void getUserInputandAnalyze() {
    fstream file("user_input.c", ios::out);
    string line;
    int count = 0;
    while (count < 5)
    {
        getline(cin, line);
        if (line == "\0") break;
        file << line << endl;
    }
    analyzeFile("user_input.c");
}

int main() {
    getUserInputandAnalyze();
}