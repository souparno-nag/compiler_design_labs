#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

struct Production {
  char left;
  string right;

  Production(char l, string r) : left(l), right(r) {}
};

struct LRItem {
  int prod_num;
  int dot_pos;

  LRItem(int p, int d) : prod_num(p), dot_pos(d) {}

  bool operator<(const LRItem &other) const {
    if (prod_num != other.prod_num)
      return prod_num < other.prod_num;
    return dot_pos < other.dot_pos;
  }

  bool operator==(const LRItem &other) const {
    return prod_num == other.prod_num && dot_pos == other.dot_pos;
  }
};

class SLRParser {
private:
  vector<Production> productions;
  set<char> non_terminals;
  set<char> terminals;
  map<char, set<char>> first_sets;
  map<char, set<char>> follow_sets;
  vector<set<LRItem>> states;
  map<pair<int, char>, int> goto_table;
  map<pair<int, char>, string> action_table;

  void computeFirstSets() {
    bool changed = true;
    while (changed) {
      changed = false;
      for (const auto &prod : productions) {
        set<char> old_first = first_sets[prod.left];

        if (prod.right == "e") { // epsilon production
          first_sets[prod.left].insert('e');
        } else {
          for (char c : prod.right) {
            if (terminals.count(c)) {
              first_sets[prod.left].insert(c);
              break;
            } else {
              for (char f : first_sets[c]) {
                if (f != 'e')
                  first_sets[prod.left].insert(f);
              }
              if (!first_sets[c].count('e'))
                break;
              if (c == prod.right.back()) {
                first_sets[prod.left].insert('e');
              }
            }
          }
        }

        if (first_sets[prod.left] != old_first)
          changed = true;
      }
    }
  }

  void computeFollowSets() {
    char start_symbol = productions[0].left;
    follow_sets[start_symbol].insert('$');

    bool changed = true;
    while (changed) {
      changed = false;
      for (const auto &prod : productions) {
        for (int i = 0; i < prod.right.length(); i++) {
          char A = prod.right[i];
          if (non_terminals.count(A)) {
            set<char> old_follow = follow_sets[A];

            // Add FIRST of beta to FOLLOW(A)
            bool all_epsilon = true;
            for (int j = i + 1; j < prod.right.length(); j++) {
              char next = prod.right[j];
              if (terminals.count(next)) {
                follow_sets[A].insert(next);
                all_epsilon = false;
                break;
              } else {
                for (char f : first_sets[next]) {
                  if (f != 'e')
                    follow_sets[A].insert(f);
                }
                if (!first_sets[next].count('e')) {
                  all_epsilon = false;
                  break;
                }
              }
            }

            // If A is at the end or all symbols after A can derive epsilon
            if (all_epsilon) {
              for (char f : follow_sets[prod.left]) {
                follow_sets[A].insert(f);
              }
            }

            if (follow_sets[A] != old_follow)
              changed = true;
          }
        }
      }
    }
  }

  set<LRItem> closure(set<LRItem> items) {
    bool changed = true;
    while (changed) {
      changed = false;
      set<LRItem> new_items = items;

      for (const auto &item : items) {
        if (item.dot_pos < productions[item.prod_num].right.length()) {
          char next = productions[item.prod_num].right[item.dot_pos];
          if (non_terminals.count(next)) {
            for (int i = 0; i < productions.size(); i++) {
              if (productions[i].left == next) {
                LRItem new_item(i, 0);
                if (find(items.begin(), items.end(), new_item) == items.end()) {
                  new_items.insert(new_item);
                  changed = true;
                }
              }
            }
          }
        }
      }
      items = new_items;
    }
    return items;
  }

  set<LRItem> goTo(set<LRItem> state, char symbol) {
    set<LRItem> result;
    for (const auto &item : state) {
      if (item.dot_pos < productions[item.prod_num].right.length() &&
          productions[item.prod_num].right[item.dot_pos] == symbol) {
        result.insert(LRItem(item.prod_num, item.dot_pos + 1));
      }
    }
    return closure(result);
  }

  void constructStates() {
    // Initial state with augmented start production
    set<LRItem> initial_state;
    initial_state.insert(LRItem(0, 0));
    states.push_back(closure(initial_state));

    for (int i = 0; i < states.size(); i++) {
      set<char> symbols;

      // Collect all symbols after dots
      for (const auto &item : states[i]) {
        if (item.dot_pos < productions[item.prod_num].right.length()) {
          symbols.insert(productions[item.prod_num].right[item.dot_pos]);
        }
      }

      for (char symbol : symbols) {
        set<LRItem> new_state = goTo(states[i], symbol);
        if (!new_state.empty()) {
          // Check if state already exists
          int state_index = -1;
          for (int j = 0; j < states.size(); j++) {
            if (states[j] == new_state) {
              state_index = j;
              break;
            }
          }

          if (state_index == -1) {
            states.push_back(new_state);
            state_index = states.size() - 1;
          }

          goto_table[{i, symbol}] = state_index;
        }
      }
    }
  }

  void constructParseTable() {
    for (int i = 0; i < states.size(); i++) {
      for (const auto &item : states[i]) {
        // Reduce items
        if (item.dot_pos == productions[item.prod_num].right.length()) {
          if (item.prod_num == 0 &&
              productions[0].left == productions[0].left) { // Accept
            action_table[{i, '$'}] = "accept";
          } else {
            for (char a : follow_sets[productions[item.prod_num].left]) {
              action_table[{i, a}] = "r" + to_string(item.prod_num);
            }
          }
        }
        // Shift items
        else {
          char next = productions[item.prod_num].right[item.dot_pos];
          if (terminals.count(next)) {
            if (goto_table.count({i, next})) {
              action_table[{i, next}] = "s" + to_string(goto_table[{i, next}]);
            }
          }
        }
      }
    }
  }

public:
  void addProduction(char left, string right) {
    productions.push_back(Production(left, right));
    non_terminals.insert(left);

    for (char c : right) {
      if (c != 'e' && !non_terminals.count(c)) {
        terminals.insert(c);
      }
    }
  }

  void buildParseTable() {
    computeFirstSets();
    computeFollowSets();
    constructStates();
    constructParseTable();
  }

  void printFirstSets() {
    cout << "\nFIRST SETS:\n";
    cout << "============\n";
    for (char nt : non_terminals) {
      cout << "FIRST(" << nt << ") = { ";
      bool first = true;
      for (char c : first_sets[nt]) {
        if (!first)
          cout << ", ";
        cout << (c == 'e' ? "ε" : string(1, c));
        first = false;
      }
      cout << " }\n";
    }
  }

  void printFollowSets() {
    cout << "\nFOLLOW SETS:\n";
    cout << "=============\n";
    for (char nt : non_terminals) {
      cout << "FOLLOW(" << nt << ") = { ";
      bool first = true;
      for (char c : follow_sets[nt]) {
        if (!first)
          cout << ", ";
        cout << string(1, c);
        first = false;
      }
      cout << " }\n";
    }
  }

  void printStates() {
    cout << "\nLR(0) STATES:\n";
    cout << "==============\n";
    for (int i = 0; i < states.size(); i++) {
      cout << "State " << i << ":\n";
      for (const auto &item : states[i]) {
        cout << "  " << productions[item.prod_num].left << " -> ";
        string right = productions[item.prod_num].right;
        for (int j = 0; j <= right.length(); j++) {
          if (j == item.dot_pos)
            cout << "•";
          if (j < right.length()) {
            cout << (right[j] == 'e' ? 'e' : right[j]);
          }
        }
        cout << "\n";
      }
      cout << "\n";
    }
  }

  void printParseTable() {
    cout << "\nSLR PARSE TABLE:\n";
    cout << "=================\n";

    // Print header
    cout << setw(8) << "State";
    for (char t : terminals) {
      cout << setw(8) << t;
    }
    cout << setw(8) << "$";
    for (char nt : non_terminals) {
      cout << setw(8) << nt;
    }
    cout << "\n";

    // Print separator
    for (int i = 0; i < 8 * (1 + terminals.size() + 1 + non_terminals.size());
         i++) {
      cout << "-";
    }
    cout << "\n";

    // Print table rows
    for (int i = 0; i < states.size(); i++) {
      cout << setw(8) << i;

      // Action part
      for (char t : terminals) {
        if (action_table.count({i, t})) {
          cout << setw(8) << action_table[{i, t}];
        } else {
          cout << setw(8) << "";
        }
      }

      // $ column
      if (action_table.count({i, '$'})) {
        cout << setw(8) << action_table[{i, '$'}];
      } else {
        cout << setw(8) << "";
      }

      // Goto part
      for (char nt : non_terminals) {
        if (goto_table.count({i, nt})) {
          cout << setw(8) << goto_table[{i, nt}];
        } else {
          cout << setw(8) << "";
        }
      }
      cout << "\n";
    }
  }

  void printProductions() {
    cout << "\nGRAMMAR PRODUCTIONS:\n";
    cout << "=====================\n";
    for (int i = 0; i < productions.size(); i++) {
      cout << i << ": " << productions[i].left << " -> ";
      if (productions[i].right == "e") {
        cout << "ε";
      } else {
        cout << productions[i].right;
      }
      cout << "\n";
    }
  }
};

int main() {
  SLRParser parser;

  int num_productions;
  cout << "Enter number of productions: ";
  cin >> num_productions;
  cin.ignore(); // Clear input buffer

  cout << "\nEnter productions in the format 'A->abc' (use 'e' for epsilon):\n";
  cout << "Example: E->E+T or A->e\n\n";

  for (int i = 0; i < num_productions; i++) {
    string production;
    cout << "Production " << i + 1 << ": ";
    getline(cin, production);

    // Parse production
    size_t arrow_pos = production.find("->");
    if (arrow_pos != string::npos) {
      char left = production[0];
      string right = production.substr(arrow_pos + 2);
      parser.addProduction(left, right);
    } else {
      cout << "Invalid format! Please use 'A->abc' format.\n";
      i--; // Retry this production
    }
  }

  parser.buildParseTable();

  parser.printProductions();
  parser.printFirstSets();
  parser.printFollowSets();
  //  parser.printStates();
  parser.printParseTable();

  return 0;
}

/*
6
E->E+T
E->T  
T->T*F
T->F
F->(E)
F->id
*/