#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

using namespace std;

struct Production {
  char left;
  string right;

  Production() {}
  Production(char l, string r) : left(l), right(r) {}
};

class LRParser {
private:
  map<pair<int, char>, string> action_table;
  map<pair<int, char>, int> goto_table;
  vector<Production> productions;
  set<int> all_states;
  set<char> terminals;
  set<char> non_terminals;

  void printStep(int step, const stack<int> &state_stack,
                 const stack<char> &symbol_stack, const string &input,
                 const string &action) {
    cout << setw(4) << step;

    // Print state stack
    stack<int> temp_states = state_stack;
    vector<int> states;
    while (!temp_states.empty()) {
      states.push_back(temp_states.top());
      temp_states.pop();
    }
    reverse(states.begin(), states.end());

    string state_str = "";
    for (int i = 0; i < states.size(); i++) {
      if (i > 0)
        state_str += " ";
      state_str += to_string(states[i]);
    }
    cout << setw(15) << state_str;

    // Print symbol stack
    stack<char> temp_symbols = symbol_stack;
    vector<char> symbols;
    while (!temp_symbols.empty()) {
      symbols.push_back(temp_symbols.top());
      temp_symbols.pop();
    }
    reverse(symbols.begin(), symbols.end());

    string symbol_str = "";
    for (char symbol : symbols) {
      symbol_str += symbol;
    }
    cout << setw(15) << symbol_str;

    // Print input
    cout << setw(20) << input;

    // Print action
    cout << setw(25) << action;

    cout << "\n";
  }

public:
  void addProduction(int index, char left, string right) {
    while (index >= productions.size()) {
      productions.resize(index + 1);
    }
    productions[index] = Production(left, right);
    non_terminals.insert(left);

    // Add terminals from right side
    for (char c : right) {
      if (c != 'e' && non_terminals.find(c) == non_terminals.end()) {
        terminals.insert(c);
      }
    }
  }

  void setActionEntry(int state, char symbol, string action) {
    action_table[{state, symbol}] = action;
    all_states.insert(state);
    if (symbol != '$') {
      terminals.insert(symbol);
    }
  }

  void setGotoEntry(int state, char symbol, int next_state) {
    goto_table[{state, symbol}] = next_state;
    all_states.insert(state);
    all_states.insert(next_state);
    non_terminals.insert(symbol);
  }

  void printParseTable() {
    cout << "\nPARSE TABLE:\n";
    cout << "=============\n";

    // Create ordered sets for display
    vector<int> ordered_states(all_states.begin(), all_states.end());
    vector<char> ordered_terminals(terminals.begin(), terminals.end());
    vector<char> ordered_non_terminals(non_terminals.begin(),
                                       non_terminals.end());

    sort(ordered_states.begin(), ordered_states.end());
    sort(ordered_terminals.begin(), ordered_terminals.end());
    sort(ordered_non_terminals.begin(), ordered_non_terminals.end());

    // Print header
    cout << setw(6) << "State";
    for (char t : ordered_terminals) {
      cout << setw(8) << t;
    }
    cout << setw(8) << "$";
    for (char nt : ordered_non_terminals) {
      cout << setw(8) << nt;
    }
    cout << "\n";

    // Print separator
    int total_width =
        6 + 8 * (ordered_terminals.size() + 1 + ordered_non_terminals.size());
    for (int i = 0; i < total_width; i++) {
      cout << "-";
    }
    cout << "\n";

    // Print table rows
    for (int state : ordered_states) {
      cout << setw(6) << state;

      // Action entries for terminals
      for (char t : ordered_terminals) {
        if (action_table.count({state, t})) {
          cout << setw(8) << action_table[{state, t}];
        } else {
          cout << setw(8) << "";
        }
      }

      // Action entry for $
      if (action_table.count({state, '$'})) {
        cout << setw(8) << action_table[{state, '$'}];
      } else {
        cout << setw(8) << "";
      }

      // Goto entries for non-terminals
      for (char nt : ordered_non_terminals) {
        if (goto_table.count({state, nt})) {
          cout << setw(8) << goto_table[{state, nt}];
        } else {
          cout << setw(8) << "";
        }
      }
      cout << "\n";
    }
  }

  void printProductions() {
    cout << "\nPRODUCTIONS:\n";
    cout << "=============\n";
    for (int i = 0; i < productions.size(); i++) {
      if (!productions[i].left)
        continue; // Skip empty productions
      cout << i << ": " << productions[i].left << " -> ";
      if (productions[i].right == "e") {
        cout << "ε";
      } else {
        cout << productions[i].right;
      }
      cout << "\n";
    }
  }

  bool parse(string input) {
    // Add $ to end of input if not present
    if (input.empty() || input.back() != '$') {
      input += '$';
    }

    stack<int> state_stack;
    stack<char> symbol_stack;

    state_stack.push(0); // Initial state

    int input_index = 0;
    int step = 1;

    cout << "\nPARSING TRACE:\n";
    cout << "===============\n";
    cout << setw(4) << "Step" << setw(15) << "State Stack" << setw(15)
         << "Symbol Stack" << setw(20) << "Input" << setw(25) << "Action"
         << "\n";
    cout << string(79, '-') << "\n";

    while (input_index < input.length()) {
      if (state_stack.empty()) {
        cout << "\nPARSE ERROR: State stack is empty\n";
        return false;
      }

      int current_state = state_stack.top();
      char current_input = input[input_index];

      string remaining_input = input.substr(input_index);

      // Look up action in parse table
      if (action_table.find({current_state, current_input}) ==
          action_table.end()) {
        printStep(step, state_stack, symbol_stack, remaining_input,
                  "ERROR - No action");
        cout << "\nPARSE ERROR: No action for state " << current_state
             << " and symbol '" << current_input << "'\n";
        return false;
      }

      string action = action_table[{current_state, current_input}];

      if (action == "accept") {
        printStep(step, state_stack, symbol_stack, remaining_input, "ACCEPT");
        cout << "\nPARSING SUCCESSFUL!\n";
        return true;
      } else if (action[0] == 's') { // Shift action
        int next_state = stoi(action.substr(1));

        printStep(step++, state_stack, symbol_stack, remaining_input,
                  "shift " + to_string(next_state));

        state_stack.push(next_state);
        symbol_stack.push(current_input);
        input_index++;
      } else if (action[0] == 'r') { // Reduce action
        int prod_num = stoi(action.substr(1));

        if (prod_num >= productions.size() || !productions[prod_num].left) {
          printStep(step, state_stack, symbol_stack, remaining_input,
                    "ERROR - Invalid production");
          cout << "\nPARSE ERROR: Invalid production number " << prod_num
               << "\n";
          return false;
        }

        Production &prod = productions[prod_num];

        string reduce_action = "reduce by " + to_string(prod_num) + " (" +
                               prod.left + "->" +
                               (prod.right == "e" ? "ε" : prod.right) + ")";
        printStep(step++, state_stack, symbol_stack, remaining_input,
                  reduce_action);

        // Pop symbols and states according to production right side
        int pop_count = (prod.right == "e") ? 0 : prod.right.length();

        for (int i = 0; i < pop_count; i++) {
          if (!state_stack.empty())
            state_stack.pop();
          if (!symbol_stack.empty())
            symbol_stack.pop();
        }

        // Push left side of production
        symbol_stack.push(prod.left);

        // Get goto state
        if (state_stack.empty()) {
          cout << "\nPARSE ERROR: State stack is empty during reduce\n";
          return false;
        }

        int top_state = state_stack.top();
        if (goto_table.find({top_state, prod.left}) == goto_table.end()) {
          cout << "\nPARSE ERROR: No goto entry for state " << top_state
               << " and non-terminal '" << prod.left << "'\n";
          return false;
        }

        int goto_state = goto_table[{top_state, prod.left}];
        state_stack.push(goto_state);
      } else {
        printStep(step, state_stack, symbol_stack, remaining_input,
                  "ERROR - Unknown action");
        cout << "\nPARSE ERROR: Unknown action '" << action << "'\n";
        return false;
      }
    }

    cout << "\nPARSE ERROR: Reached end of input without acceptance\n";
    return false;
  }
};

int main() {
  LRParser parser;

  cout << "LR PARSING ALGORITHM IMPLEMENTATION\n";
  cout << "====================================\n";

  // Get productions from user
  int num_productions;
  cout << "\nEnter number of productions: ";
  cin >> num_productions;
  cin.ignore();

  cout << "\nEnter productions in format: 'index left right' (use 'e' for "
          "epsilon)\n";
  cout << "Example: 0 E E+T  or  1 F e\n";

  for (int i = 0; i < num_productions; i++) {
    cout << "Production " << i + 1 << ": ";
    string line;
    getline(cin, line);

    istringstream iss(line);
    int index;
    char left;
    string right;

    if (iss >> index >> left >> right) {
      parser.addProduction(index, left, right);
    } else {
      cout << "Invalid format! Please try again.\n";
      i--;
    }
  }

  // Get ACTION table entries
  cout << "\nEnter ACTION table entries in format: 'state symbol action'\n";
  cout << "Examples: 0 i s5  or  2 + r1  or  1 $ accept\n";
  cout << "Enter 'done' when finished:\n";

  string input;
  while (getline(cin, input) && input != "done") {
    if (input.empty())
      continue;

    istringstream iss(input);
    int state;
    char symbol;
    string action;

    if (iss >> state >> symbol >> action) {
      parser.setActionEntry(state, symbol, action);
    } else {
      cout << "Invalid format! Please use: state symbol action\n";
    }
  }

  // Get GOTO table entries
  cout << "\nEnter GOTO table entries in format: 'state symbol next_state'\n";
  cout << "Example: 0 E 1\n";
  cout << "Enter 'done' when finished:\n";

  while (getline(cin, input) && input != "done") {
    if (input.empty())
      continue;

    istringstream iss(input);
    int state, next_state;
    char symbol;

    if (iss >> state >> symbol >> next_state) {
      parser.setGotoEntry(state, symbol, next_state);
    } else {
      cout << "Invalid format! Please use: state symbol next_state\n";
    }
  }

  // Display the parse table and productions
  parser.printProductions();
  parser.printParseTable();

  // Parse input strings
  string input_string;
  char continue_parsing = 'y';

  while (continue_parsing == 'y' || continue_parsing == 'Y') {
    cout << "\nEnter input string to parse (without $): ";
    cin >> input_string;
    cin.ignore();

    bool result = parser.parse(input_string);

    if (result) {
      cout << "\n✓ String ACCEPTED by the grammar!\n";
    } else {
      cout << "\n✗ String REJECTED by the grammar!\n";
    }

    cout << "\nDo you want to parse another string? (y/n): ";
    cin >> continue_parsing;
    cin.ignore();
  }

  cout << "\nProgram terminated.\n";
  return 0;
}

/*
6
1 E E+T
2 E T
3 T T*F
4 T F
5 F (E)
6 F i

0 i s5
0 ( s4
1 + s6
1 $ accept
2 + r2
2 * s7
2 ) r2
2 $ r2
3 + r4
3 * r4
3 ) r4
3 $ r4
4 i s5
4 ( s4
5 + r6
5 * r6
5 ) r6
5 $ r6
6 i s5
6 ( s4
7 i s5
7 ( s4
8 + s6
8 ) s11
9 + r1
9 * s7
9 ) r1
9 $ r1
10 + r3
10 * r3
10 ) r3
10 $ r3
11 + r5
11 * r5
11 ) r5
11 $ r5
done

0 E 1
0 T 2
0 F 3
4 E 8
4 T 2
4 F 3
6 T 9
6 F 3
7 F 10
done

i*i+i
n
*/