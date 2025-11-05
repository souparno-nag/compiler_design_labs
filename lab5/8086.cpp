#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <set>
#include <cctype>

bool isNumber(const std::string& s) {
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0] == '-') {
        if (s.length() == 1) return false;
        i = 1;
    }
    return std::all_of(s.begin() + i, s.end(),
                       [](unsigned char ch) { return std::isdigit(ch); });
}

std::string getJumpInstruction(const std::string& op) {
    if (op == "==") return "JE";
    if (op == "!=") return "JNE";
    if (op == "<")  return "JL";
    if (op == "<=") return "JLE";
    if (op == ">")  return "JG";
    if (op == ">=") return "JGE";
    return "";
}

void generateAssembly(const std::vector<std::string>& tac) {
    std::set<std::string> variables;

    // Collect variables
    for (const auto& line : tac) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (ss >> token) tokens.push_back(token);
        if (tokens.empty()) continue;

        // Condition
        if (tokens[0] == "if") {
            if (tokens.size() > 1 && !isNumber(tokens[1])) variables.insert(tokens[1]);
            if (tokens.size() > 3 && !isNumber(tokens[3])) variables.insert(tokens[3]);

        // Assignment
        } else if (tokens.size() > 1 && tokens[1] == "=") {
            if (!isNumber(tokens[0])) variables.insert(tokens[0]);
            if (tokens.size() > 2 && !isNumber(tokens[2])) variables.insert(tokens[2]);
            if (tokens.size() > 4 && !isNumber(tokens[4])) variables.insert(tokens[4]);
        }
    }

    std::cout << "; Generated 8086 Assembly Code\n";
    std::cout << "MODEL SMALL\nDATA\n";

    for (const auto& var : variables)
        std::cout << " " << var << " DW 0\n";

    std::cout << "CODE\nMAIN PROC\n";
    std::cout << " MOV AX, @DATA\n MOV DS, AX\n\n";

    for (const auto& line : tac) {
        std::cout << "; TAC: " << line << "\n";

        std::stringstream ss(line);
        std::string first_word;
        ss >> first_word;

        if (first_word.empty()) continue;

        // Label
        if (first_word.back() == ':') {
            std::cout << first_word << "\n\n";
            continue;
        }

        // Goto
        if (first_word == "goto") {
            std::string label;
            ss >> label;
            std::cout << " JMP " << label << "\n\n";
            continue;
        }

        // If statement
        if (first_word == "if") {
            std::string src1, op, src2, go, label;
            ss >> src1 >> op >> src2 >> go >> label;
            std::cout << " MOV AX, " << src1 << "\n";
            std::cout << " CMP AX, " << src2 << "\n";
            std::string jmp_inst = getJumpInstruction(op);
            if (!jmp_inst.empty())
                std::cout << " " << jmp_inst << " " << label << "\n\n";
            continue;
        }

        // Assignment / arithmetic
        std::stringstream line_ss(line);
        std::string dest, eq_op, src1, op, src2;
        line_ss >> dest >> eq_op >> src1;

        if (line_ss >> op >> src2) {
            if (isNumber(src1))
                std::cout << " MOV AX, " << src1 << "\n";
            else
                std::cout << " MOV AX, " << src1 << "\n";

            if (op == "+") {
                if (isNumber(src2)) std::cout << " ADD AX, " << src2 << "\n";
                else std::cout << " ADD AX, " << src2 << "\n";
            } else if (op == "-") {
                if (isNumber(src2)) std::cout << " SUB AX, " << src2 << "\n";
                else std::cout << " SUB AX, " << src2 << "\n";
            } else if (op == "*") {
                if (isNumber(src2)) {
                    std::cout << " MOV BX, " << src2 << "\n";
                    std::cout << " MUL BX\n";
                } else {
                    std::cout << " MUL " << src2 << "\n";
                }
            } else if (op == "/") {
                std::cout << " MOV DX, 0\n";
                if (isNumber(src2)) {
                    std::cout << " MOV BX, " << src2 << "\n";
                    std::cout << " DIV BX\n";
                } else {
                    std::cout << " DIV " << src2 << "\n";
                }
            }
            std::cout << " MOV " << dest << ", AX\n\n";
        } else {
            std::cout << " MOV AX, " << src1 << "\n";
            std::cout << " MOV " << dest << ", AX\n\n";
        }
    }

    std::cout << " MOV AH, 4CH\n INT 21H\n";
    std::cout << "MAIN ENDP\nEND MAIN\n";
}

int main() {
    std::vector<std::string> three_address_code;
    std::string line;

    std::cout << "Enter Three-Address Code (one instruction per line).\n";
    std::cout << "Ctrl+D to finish input.\n";

    while (std::getline(std::cin, line)) {
        if (!line.empty())
            three_address_code.push_back(line);
    }

    std::cout << "\n--- Generating 8086 Assembly Code ---\n\n";
    generateAssembly(three_address_code);
    return 0;
}
/*
t1 = a + b
if t1 > c goto L1


if a <= b goto L1

*/
