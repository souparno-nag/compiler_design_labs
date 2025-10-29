#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

struct Production {
  char left;
  string right;
  Production() {}
  Production(char l, string r) : left(l), right(r) {}
};

struct LR1Item {
  int prod_num;
  int dot_pos;
  char lookahead;

  LR1Item(int p, int d, char l) : prod_num(p), dot_pos(d), lookahead(l) {}

  bool operator<(const LR1Item &other) const {
    if (prod_num != other.prod_num)
      return prod_num < other.prod_num;
    if (dot_pos != other.dot_pos)
      return dot_pos < other.dot_pos;
    return lookahead < other.lookahead;
  }

  bool operator==(const LR1Item &other) const {
    return prod_num == other.prod_num && dot_pos == other.dot_pos &&
           lookahead == other.lookahead;
  }
};

struct LR0Item {
  int prod_num;
  int dot_pos;

  LR0Item(int p, int d) : prod_num(p), dot_pos(d) {}
  LR0Item(const LR1Item &lr1) : prod_num(lr1.prod_num), dot_pos(lr1.dot_pos) {}

  bool operator<(const LR0Item &other) const {
    if (prod_num != other.prod_num)
      return prod_num < other.prod_num;
    return dot_pos < other.dot_pos;
  }

  bool operator==(const LR0Item &other) const {
    return prod_num == other.prod_num && dot_pos == other.dot_pos;
  }
};

class LALRParser {
private:
  vector<Production> productions;
  set<char> non_terminals;
  set<char> terminals;
  map<char, set<char>> first_sets;

  // LALR(1) states and tables
  vector<set<LR1Item>> lalr_states;
  map<pair<int, char>, int> goto_table;
  map<pair<int, char>, string> action_table;

  char start_symbol = 0;
  char augmented_symbol = 0;

  void finalizeGrammar() {
    set<char> all_symbols;
    for (const auto &p : productions) {
      for (char c : p.right) {
        if (c != 'e')
          all_symbols.insert(c);
      }
    }

    terminals.clear();
    for (char c : all_symbols) {
      if (non_terminals.find(c) == non_terminals.end()) {
        terminals.insert(c);
      }
    }
    terminals.insert('$');
  }

  void computeFirstSets() {
    for (char nt : non_terminals) {
      first_sets[nt].clear();
    }

    bool changed = true;
    while (changed) {
      changed = false;
      for (const auto &prod : productions) {
        char A = prod.left;
        set<char> old_first = first_sets[A];

        if (prod.right == "e") {
          first_sets[A].insert('e');
        } else {
          bool all_have_epsilon = true;
          for (size_t i = 0; i < prod.right.size(); ++i) {
            char X = prod.right[i];

            if (terminals.find(X) != terminals.end() || X == '$') {
              first_sets[A].insert(X);
              all_have_epsilon = false;
              break;
            } else if (non_terminals.find(X) != non_terminals.end()) {
              for (char f : first_sets[X]) {
                if (f != 'e') {
                  first_sets[A].insert(f);
                }
              }
              if (first_sets[X].find('e') == first_sets[X].end()) {
                all_have_epsilon = false;
                break;
              }
            } else {
              first_sets[A].insert(X);
              all_have_epsilon = false;
              break;
            }
          }

          if (all_have_epsilon) {
            first_sets[A].insert('e');
          }
        }

        if (first_sets[A] != old_first) {
          changed = true;
        }
      }
    }
  }

  set<char> getFirstOfString(const string &str) {
    set<char> result;

    if (str.empty()) {
      result.insert('e');
      return result;
    }

    bool all_have_epsilon = true;
    for (size_t i = 0; i < str.size(); ++i) {
      char X = str[i];

      if (terminals.find(X) != terminals.end() || X == '$') {
        result.insert(X);
        all_have_epsilon = false;
        break;
      } else if (non_terminals.find(X) != non_terminals.end()) {
        for (char f : first_sets[X]) {
          if (f != 'e') {
            result.insert(f);
          }
        }
        if (first_sets[X].find('e') == first_sets[X].end()) {
          all_have_epsilon = false;
          break;
        }
      } else {
        result.insert(X);
        all_have_epsilon = false;
        break;
      }
    }

    if (all_have_epsilon) {
      result.insert('e');
    }

    return result;
  }

  set<LR1Item> closure(const set<LR1Item> &I) {
    set<LR1Item> items = I;
    bool changed = true;

    while (changed) {
      changed = false;
      set<LR1Item> new_items = items;

      for (const auto &item : items) {
        const Production &prod = productions[item.prod_num];

        if (item.dot_pos < prod.right.length()) {
          char B = prod.right[item.dot_pos];

          if (non_terminals.find(B) != non_terminals.end()) {
            string beta;
            if (item.dot_pos + 1 < prod.right.length()) {
              beta = prod.right.substr(item.dot_pos + 1);
            }

            string beta_a = beta + item.lookahead;
            set<char> first_beta_a = getFirstOfString(beta_a);

            for (size_t p = 0; p < productions.size(); ++p) {
              if (productions[p].left == B) {
                for (char b : first_beta_a) {
                  if (b != 'e') {
                    LR1Item new_item((int)p, 0, b);
                    if (new_items.find(new_item) == new_items.end()) {
                      new_items.insert(new_item);
                      changed = true;
                    }
                  }
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

  set<LR1Item> goTo(const set<LR1Item> &state, char X) {
    set<LR1Item> moved;

    for (const auto &item : state) {
      const Production &prod = productions[item.prod_num];

      if (item.dot_pos < prod.right.length() && prod.right[item.dot_pos] == X) {
        moved.insert(LR1Item(item.prod_num, item.dot_pos + 1, item.lookahead));
      }
    }

    if (moved.empty()) {
      return moved;
    }

    return closure(moved);
  }

  char chooseAugmentedSymbol() {
    const string candidates = "S'@#~`&";
    for (char c : candidates) {
      if (non_terminals.find(c) == non_terminals.end() &&
          terminals.find(c) == terminals.end() && c != '$') {
        return c;
      }
    }

    for (char c = 'A'; c <= 'Z'; ++c) {
      if (non_terminals.find(c) == non_terminals.end() &&
          terminals.find(c) == terminals.end()) {
        return c;
      }
    }

    return 'Z';
  }

  // Get the core of a state (items without lookaheads)
  set<LR0Item> getStateCore(const set<LR1Item> &state) {
    set<LR0Item> core;
    for (const auto &item : state) {
      core.insert(LR0Item(item));
    }
    return core;
  }

  void constructLALRStates() {
    // Create augmented grammar
    augmented_symbol = chooseAugmentedSymbol();
    Production aug_prod(augmented_symbol, string(1, start_symbol));
    productions.insert(productions.begin(), aug_prod);
    non_terminals.insert(augmented_symbol);

    computeFirstSets();

    // Step 1: Build LR(0) states with multiple lookaheads
    map<set<LR0Item>, set<LR1Item>> core_to_items;

    // Initial state
    set<LR1Item> I0;
    I0.insert(LR1Item(0, 0, '$'));
    set<LR1Item> initial_state = closure(I0);

    vector<set<LR1Item>> temp_states;
    map<pair<int, char>, int> temp_goto;
    temp_states.push_back(initial_state);

    // Build all states first
    for (size_t i = 0; i < temp_states.size(); ++i) {
      set<char> symbols;
      for (const auto &item : temp_states[i]) {
        const Production &prod = productions[item.prod_num];
        if (item.dot_pos < prod.right.length()) {
          char X = prod.right[item.dot_pos];
          if (X != 'e') {
            symbols.insert(X);
          }
        }
      }

      for (char X : symbols) {
        set<LR1Item> J = goTo(temp_states[i], X);
        if (!J.empty()) {
          int state_index = -1;
          for (size_t s = 0; s < temp_states.size(); ++s) {
            if (temp_states[s] == J) {
              state_index = (int)s;
              break;
            }
          }

          if (state_index == -1) {
            temp_states.push_back(J);
            state_index = (int)temp_states.size() - 1;
          }

          temp_goto[{(int)i, X}] = state_index;
        }
      }
    }

    // Step 2: Group states by their cores and merge
    for (const auto &state : temp_states) {
      set<LR0Item> core = getStateCore(state);
      if (core_to_items.find(core) == core_to_items.end()) {
        core_to_items[core] = state;
      } else {
        // Merge lookaheads
        for (const auto &item : state) {
          core_to_items[core].insert(item);
        }
      }
    }

    // Step 3: Create LALR states from merged cores
    lalr_states.clear();
    goto_table.clear();

    map<set<LR0Item>, int> core_to_state_index;
    int state_counter = 0;

    for (const auto &entry : core_to_items) {
      lalr_states.push_back(entry.second);
      core_to_state_index[entry.first] = state_counter++;
    }

    // Step 4: Build GOTO table for LALR states
    for (size_t i = 0; i < lalr_states.size(); ++i) {
      set<char> symbols;
      for (const auto &item : lalr_states[i]) {
        const Production &prod = productions[item.prod_num];
        if (item.dot_pos < prod.right.length()) {
          char X = prod.right[item.dot_pos];
          if (X != 'e') {
            symbols.insert(X);
          }
        }
      }

      for (char X : symbols) {
        set<LR1Item> J = goTo(lalr_states[i], X);
        if (!J.empty()) {
          set<LR0Item> target_core = getStateCore(J);
          auto it = core_to_state_index.find(target_core);
          if (it != core_to_state_index.end()) {
            goto_table[{(int)i, X}] = it->second;
          }
        }
      }
    }
  }

  void constructParseTable() {
    action_table.clear();

    for (size_t i = 0; i < lalr_states.size(); ++i) {
      for (const auto &item : lalr_states[i]) {
        const Production &prod = productions[item.prod_num];

        if (item.dot_pos == prod.right.length()) {
          // Reduce item
          if (item.prod_num == 0) {
            // Accept item
            action_table[{(int)i, '$'}] = "acc";
          } else {
            // Reduce item
            string action = "r" + to_string(item.prod_num);
            pair<int, char> key = {(int)i, item.lookahead};

            if (action_table.find(key) != action_table.end() &&
                action_table[key] != action) {
              cerr << "REDUCE-REDUCE CONFLICT at state " << i << " symbol '"
                   << item.lookahead << "': existing=" << action_table[key]
                   << " new=" << action << "\n";
            } else {
              action_table[key] = action;
            }
          }
        } else {
          // Shift item
          char a = prod.right[item.dot_pos];
          if (a != 'e' && (terminals.find(a) != terminals.end() || a == '$')) {
            auto goto_it = goto_table.find({(int)i, a});
            if (goto_it != goto_table.end()) {
              string action = "s" + to_string(goto_it->second);
              pair<int, char> key = {(int)i, a};

              if (action_table.find(key) != action_table.end() &&
                  action_table[key] != action) {
                cerr << "SHIFT-REDUCE CONFLICT at state " << i << " symbol '"
                     << a << "': existing=" << action_table[key]
                     << " new=" << action << "\n";
              } else {
                action_table[key] = action;
              }
            }
          }
        }
      }
    }
  }

public:
  void addProduction(char left, const string &right) {
    productions.push_back(Production(left, right));
    non_terminals.insert(left);
    if (start_symbol == 0) {
      start_symbol = left;
    }
  }

  void buildParseTable() {
    finalizeGrammar();
    constructLALRStates();
    constructParseTable();
  }

  void printProductions() {
    cout << "\nGRAMMAR PRODUCTIONS (indexed):\n";
    for (size_t i = 0; i < productions.size(); ++i) {
      cout << i << ": " << productions[i].left << " -> ";
      if (productions[i].right == "e") {
        cout << "ε";
      } else {
        cout << productions[i].right;
      }
      cout << "\n";
    }
  }

  void printFirstSets() {
    cout << "\nFIRST SETS:\n";
    for (char nt : non_terminals) {
      if (nt == augmented_symbol)
        continue;
      cout << "FIRST(" << nt << ") = { ";
      bool first = true;
      for (char c : first_sets[nt]) {
        if (!first)
          cout << ", ";
        if (c == 'e')
          cout << "ε";
        else
          cout << c;
        first = false;
      }
      cout << " }\n";
    }
  }

  void printLALRStates() {
    cout << "\nLALR(1) STATES:\n";
    for (size_t i = 0; i < lalr_states.size(); ++i) {
      cout << "State " << i << ":\n";

      for (const auto &item : lalr_states[i]) {
        cout << "  " << productions[item.prod_num].left << " -> ";
        const string &rhs = productions[item.prod_num].right;

        for (size_t k = 0; k <= rhs.length(); ++k) {
          if (k == item.dot_pos)
            cout << "•";
          if (k < rhs.length()) {
            if (rhs[k] == 'e')
              cout << "ε";
            else
              cout << rhs[k];
          }
        }
        cout << " , " << item.lookahead << "\n";
      }
      cout << "\n";
    }
  }

  void printActionTable() {
    cout << "\nACTION TABLE:\n";
    vector<char> terms(terminals.begin(), terminals.end());
    sort(terms.begin(), terms.end());

    cout << setw(6) << "State";
    for (char t : terms) {
      cout << setw(8) << t;
    }
    cout << "\n";

    for (size_t i = 0; i < lalr_states.size(); ++i) {
      cout << setw(6) << i;
      for (char t : terms) {
        auto it = action_table.find({(int)i, t});
        if (it != action_table.end()) {
          cout << setw(8) << it->second;
        } else {
          cout << setw(8) << "";
        }
      }
      cout << "\n";
    }
  }

  void printGotoTable() {
    cout << "\nGOTO TABLE:\n";
    vector<char> nts;
    for (char nt : non_terminals) {
      if (nt != augmented_symbol) {
        nts.push_back(nt);
      }
    }
    sort(nts.begin(), nts.end());

    cout << setw(6) << "State";
    for (char nt : nts) {
      cout << setw(8) << nt;
    }
    cout << "\n";

    for (size_t i = 0; i < lalr_states.size(); ++i) {
      cout << setw(6) << i;
      for (char nt : nts) {
        auto it = goto_table.find({(int)i, nt});
        if (it != goto_table.end()) {
          cout << setw(8) << it->second;
        } else {
          cout << setw(8) << "";
        }
      }
      cout << "\n";
    }
  }

  void printCombinedParseTable() {
    cout << "\nLALR(1) PARSE TABLE:\n";
    vector<char> terms(terminals.begin(), terminals.end());
    sort(terms.begin(), terms.end());

    vector<char> nts;
    for (char nt : non_terminals) {
      if (nt != augmented_symbol) {
        nts.push_back(nt);
      }
    }
    sort(nts.begin(), nts.end());

    cout << setw(6) << "State";
    for (char t : terms) {
      cout << setw(8) << t;
    }
    for (char nt : nts) {
      cout << setw(8) << nt;
    }
    cout << "\n";

    for (size_t i = 0; i < lalr_states.size(); ++i) {
      cout << setw(6) << i;

      // Action entries
      for (char t : terms) {
        auto at = action_table.find({(int)i, t});
        if (at != action_table.end()) {
          cout << setw(8) << at->second;
        } else {
          cout << setw(8) << "";
        }
      }

      // Goto entries
      for (char nt : nts) {
        auto gt = goto_table.find({(int)i, nt});
        if (gt != goto_table.end()) {
          cout << setw(8) << gt->second;
        } else {
          cout << setw(8) << "";
        }
      }
      cout << "\n";
    }
  }

  void printStatistics() {
    cout << "\nPARSER STATISTICS:\n";
    cout << "Productions: " << productions.size() << "\n";
    cout << "LALR(1) States: " << lalr_states.size() << "\n";
    cout << "Terminals: " << terminals.size() << "\n";
    cout << "Non-terminals: " << non_terminals.size() << "\n";

    // Check for conflicts
    map<pair<int, char>, set<string>> actions_by_key;
    for (const auto &entry : action_table) {
      actions_by_key[entry.first].insert(entry.second);
    }

    bool has_conflict = false;
    cout << "\nCONFLICT ANALYSIS:\n";
    for (const auto &kv : actions_by_key) {
      if (kv.second.size() > 1) {
        has_conflict = true;
        auto [state, symbol] = kv.first;
        cout << "Conflict at state " << state << " symbol '" << symbol << "':";
        for (const auto &action : kv.second) {
          cout << " " << action;
        }
        cout << "\n";
      }
    }

    if (!has_conflict) {
      cout << "No conflicts found - Grammar is LALR(1)!\n";
    }

    cout << "Status: "
         << (has_conflict ? "NOT LALR(1) - Conflicts exist"
                          : "LALR(1) - No conflicts")
         << "\n";
  }
};

int main() {
  LALRParser parser;
  cout << "LOOK-AHEAD LR (LALR) PARSE TABLE CONSTRUCTOR\n";
  cout << "============================================\n\n";

  int n;
  cout << "Enter number of productions: ";
  if (!(cin >> n) || n <= 0) {
    cout << "Invalid number of productions.\n";
    return 1;
  }
  cin.ignore(numeric_limits<streamsize>::max(), '\n');

  cout << "\nEnter productions in the format A->xyz (use 'e' for epsilon).\n";
  cout << "Examples:\n";
  cout << "  E->E+T\n";
  cout << "  T->T*F\n";
  cout << "  F->(E)\n";
  cout << "  F->id\n\n";

  for (int i = 0; i < n; ++i) {
    string line;
    cout << "Production " << (i + 1) << ": ";
    if (!getline(cin, line)) {
      cout << "Error reading input.\n";
      return 1;
    }

    // Parse production
    size_t pos = line.find("->");
    if (pos == string::npos) {
      cout << "Invalid format. Use A->xyz format. Retry this production.\n";
      --i;
      continue;
    }

    // Extract left-hand side
    char lhs = 0;
    for (size_t k = 0; k < pos; ++k) {
      if (line[k] != ' ' && line[k] != '\t') {
        lhs = line[k];
        break;
      }
    }

    if (!lhs) {
      cout << "Invalid left-hand side symbol. Retry this production.\n";
      --i;
      continue;
    }

    // Extract right-hand side
    string rhs = line.substr(pos + 2);
    // Remove spaces and tabs
    rhs.erase(remove_if(rhs.begin(), rhs.end(),
                        [](char c) { return c == ' ' || c == '\t'; }),
              rhs.end());

    if (rhs.empty()) {
      cout << "Empty right-hand side. Use 'e' for epsilon. Retry this "
              "production.\n";
      --i;
      continue;
    }

    parser.addProduction(lhs, rhs);
    cout << "Added: " << lhs << " -> " << (rhs == "e" ? "ε" : rhs) << "\n";
  }

  cout << "\nBuilding LALR(1) parse table...\n";
  parser.buildParseTable();

  // Display results
  parser.printProductions();
  parser.printFirstSets();
  //  parser.printLALRStates();
  parser.printCombinedParseTable();
  parser.printStatistics();

  return 0;
}