/* main.c
 CLR(1) -> LALR(1) parser generator + parser in C
 Dev-C++ friendly (avoid C99 mixed declarations)
 Input grammar: productions with tokens separated by spaces, e.g.
 E -> E + T
 E -> T
 T -> T * F
 T -> F
 F -> ( E )
 F -> id
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_RHS 64
#define MAX_TOKLEN 64
/* Tunable limits */
#define MAX_PRODS 400
#define MAX_SYMS 400
#define MAX_TERM 200
#define MAX_STATES 1200
#define MAX_ITEMS_PER_STATE 1024
/* Symbol table */
static char sym_name[MAX_SYMS][MAX_TOKLEN];
static int sym_count = 0;
static int is_terminal[MAX_SYMS]; /* 1 = terminal, 0 = nonterminal */
static int term_index_of_sym[MAX_SYMS]; /* maps symbol idx -> terminal index, -1 if not a terminal */
static int terminals[MAX_TERM];
static int term_count = 0;
/* Productions */
typedef struct {
 int lhs;
 int rhs[MAX_RHS];
 int rhs_len;
} Prod;
static Prod prods[MAX_PRODS];
static int prod_count = 0;
/* FIRST sets (symbol x terminal bitset) */
static unsigned char FIRST[MAX_SYMS][MAX_TERM];
/* LR(1) Item */
typedef struct {
 int prod; /* production index */
 int dot; /* dot position */
 unsigned char look[MAX_TERM]; /* lookahead set over terminals */
} Item;
/* State: set of items */
typedef struct {
 Item items[MAX_ITEMS_PER_STATE];
 int item_count;
} State;
/* Dynamic arrays for states and tables */
static State *CLRstates = NULL;
static State *LALRstates = NULL;
static int CLRstate_count = 0;
static int LALRstate_count = 0;
/* ACTION/GOTO tables: allocated after sizes known */
static int **ACTION_CLR = NULL; /* ACTION_CLR[state][term_index] */
static int **GOTO_CLR = NULL; /* GOTO_CLR[state][symbol_index] */
static int **ACTION_LALR = NULL; /* ACTION_LALR[state][term_index] */
static int **GOTO_LALR = NULL; /* GOTO_LALR[state][symbol_index] */
/* Utility: add/get symbol id */
int get_sym(const char *s) {
 int i;
 for (i = 0; i < sym_count; ++i)
 if (strcmp(sym_name[i], s) == 0) return i;
 if (sym_count >= MAX_SYMS) { fprintf(stderr, "Exceeded MAX_SYMS\n"); exit(1); }
 strncpy(sym_name[sym_count], s, MAX_TOKLEN-1);
 sym_name[sym_count][MAX_TOKLEN-1] = '\0';
 is_terminal[sym_count] = 1; /* default terminal; will mark LHS as nonterminal later */
 term_index_of_sym[sym_count] = -1;
 sym_count++;
 return sym_count - 1;
}
/* register terminal symbol into terminals[] */
int add_terminal_sym(int s) {
 if (term_index_of_sym[s] != -1) return term_index_of_sym[s];
 if (term_count >= MAX_TERM) { fprintf(stderr, "Exceeded MAX_TERM\n"); exit(1); }
 term_index_of_sym[s] = term_count;
 terminals[term_count] = s;
 term_count++;
 return term_count - 1;
}
/* tokenize a line into tokens separated by whitespace */
int tokenize_line(char *line, char tokens[][MAX_TOKLEN]) {
 int n = 0;
 char *p = strtok(line, " \t\r\n");
 while (p) {
 strncpy(tokens[n], p, MAX_TOKLEN-1);
 tokens[n][MAX_TOKLEN-1] = '\0';
 n++;
 p = strtok(NULL, " \t\r\n");
 }
 return n;
}
/* compute FIRST sets (conservative: terminals only; no epsilon support) */
void compute_FIRST() {
 int i, j;
 /* initialize */
 for (i = 0; i < sym_count; ++i)
 for (j = 0; j < term_count; ++j)
 FIRST[i][j] = 0;
 /* FIRST of terminal contains itself */
 for (i = 0; i < term_count; ++i) {
 int s = terminals[i];
 FIRST[s][i] = 1;
 }
 int changed = 1;
 while (changed) {
 changed = 0;
 for (i = 0; i < prod_count; ++i) {
 int A = prods[i].lhs;
 if (prods[i].rhs_len == 0) continue;
 int X = prods[i].rhs[0];
 for (j = 0; j < term_count; ++j) {
 if (!FIRST[A][j] && FIRST[X][j]) {
 FIRST[A][j] = 1;
 changed = 1;
 }
 }
 }
 }
}
/* lookahead helpers */
void look_clear(Item *it) {
 int i;
 for (i = 0; i < term_count; ++i) it->look[i] = 0;
}
void look_copy(Item *dst, Item *src) {
 int i;
 for (i = 0; i < term_count; ++i) dst->look[i] = src->look[i];
}
/* compare item core (prod,dot) only */
int item_core_eq(Item *a, Item *b) { return a->prod == b->prod && a->dot == b->dot; }
/* find item index by core in state, returns index or -1 */
int state_find_item_core(State *st, Item *it) {
 int i;
 for (i = 0; i < st->item_count; ++i)
 if (st->items[i].prod == it->prod && st->items[i].dot == it->dot) return i;
 return -1;
}
/* add item to state; if core exists, union lookahead; return 1 if added or lookahead changed */
int state_add_item(State *st, Item *it) {
 int idx = state_find_item_core(st, it);
 if (idx == -1) {
 if (st->item_count >= MAX_ITEMS_PER_STATE) { fprintf(stderr, "Exceeded items per state\n"); exit(1); }
 st->items[st->item_count++] = *it;
 return 1;
 } else {
 int changed = 0;
 int t;
 for (t = 0; t < term_count; ++t) {
 if (it->look[t] && !st->items[idx].look[t]) {
 st->items[idx].look[t] = 1;
 changed = 1;
 }
 }
 return changed;
 }
}
/* closure operation */
void closure(State *st) {
 int added = 1;
 while (added) {
 added = 0;
 {
 int i;
 for (i = 0; i < st->item_count; ++i) {
 Item *it = &st->items[i];
 Prod *p = &prods[it->prod];
 if (it->dot >= p->rhs_len) continue;
 int B = p->rhs[it->dot];
 if (is_terminal[B]) continue; /* only expand nonterminals */
 /* For each production B -> gamma, create item (B -> . gamma) with lookahead computed
 as FIRST(beta) U lookahead (conservative approximation: FIRST of first symbol of beta)
 */
 {
 int pr;
 for (pr = 0; pr < prod_count; ++pr) {
 if (prods[pr].lhs != B) continue;
 Item newit;
 newit.prod = pr;
 newit.dot = 0;
 look_clear(&newit);
 /* compute lookahead: if beta empty -> use it->look, else use FIRST of first symbol of beta */
 if (it->dot + 1 >= p->rhs_len) {
 /* beta empty */
 int la;
 for (la = 0; la < term_count; ++la) if (it->look[la]) newit.look[la] = 1;
 } else {
 int X = p->rhs[it->dot + 1];
 int tt;
 for (tt = 0; tt < term_count; ++tt) if (FIRST[X][tt]) newit.look[tt] = 1;
 }
 if (state_add_item(st, &newit)) added = 1;
 }
 }
 }
 }
 }
}
/* goto operation: compute items with dot advanced over X */
void go_to(State *I, int X, State *out) {
 int i;
 out->item_count = 0;
 for (i = 0; i < I->item_count; ++i) {
 Item *it = &I->items[i];
 Prod *p = &prods[it->prod];
 if (it->dot < p->rhs_len && p->rhs[it->dot] == X) {
 Item nit;
 nit.prod = it->prod;
 nit.dot = it->dot + 1;
 look_clear(&nit);
 look_copy(&nit, it);
 state_add_item(out, &nit);
 }
 }
 closure(out);
}
/* compare two states for exact LR(1) equality (items + lookahead) */
int state_equal(State *A, State *B) {
 int i, j, t;
 if (A->item_count != B->item_count) return 0;
 for (i = 0; i < A->item_count; ++i) {
 int found = 0;
 for (j = 0; j < B->item_count; ++j) {
 if (A->items[i].prod == B->items[j].prod && A->items[i].dot == B->items[j].dot) {
 int same = 1;
 for (t = 0; t < term_count; ++t) if (A->items[i].look[t] != B->items[j].look[t]) { same = 0; break; }
 if (same) { found = 1; break; }
 }
 }
 if (!found) return 0;
 }
 return 1;
}
/* core signature for LR(0) core: textual */
void core_signature(State *st, char *buf, int buf_sz) {
 int i;
 buf[0] = '\0';
 for (i = 0; i < st->item_count; ++i) {
 char tmp[128];
 /* snprintf is usually available; using it here */
 snprintf(tmp, sizeof(tmp), "%d:%d;", st->items[i].prod, st->items[i].dot);
 /* safe strcat */
 strncat(buf, tmp, buf_sz - strlen(buf) - 1);
 }
}
/* build CLR(1) canonical collection */
void build_CLR_collection() {
 CLRstates = (State *) malloc(sizeof(State) * MAX_STATES);
 if (!CLRstates) { fprintf(stderr, "malloc failed\n"); exit(1); }
 CLRstate_count = 0;
 /* initial item: augmented production should be prods[0] */
 {
 State I0;
 I0.item_count = 0;
 Item it0; it0.prod = 0; it0.dot = 0; look_clear(&it0);
 /* find terminal index of $ */
 int doll_sym = get_sym("$");
 is_terminal[doll_sym] = 1;
 add_terminal_sym(doll_sym); /* ensure terminal index exists */
 /* we need the terminal index for $ */
 /* find it */
 {
 int dollar_term_idx = term_index_of_sym[doll_sym];
 if (dollar_term_idx >= 0) it0.look[dollar_term_idx] = 1;
 }
 state_add_item(&I0, &it0);
 closure(&I0);
 CLRstates[CLRstate_count++] = I0;
 }
 {
 int changed = 1;
 while (changed) {
 changed = 0;
 {
 int s;
 for (s = 0; s < CLRstate_count; ++s) {
 int X;
 for (X = 0; X < sym_count; ++X) {
 State J; go_to(&CLRstates[s], X, &J);
 if (J.item_count == 0) continue;
 /* check if J already present */
 int found = -1;
 {
 int k;
 for (k = 0; k < CLRstate_count; ++k) if (state_equal(&CLRstates[k], &J)) { found = k; break; }
 }
 if (found == -1) {
 if (CLRstate_count >= MAX_STATES) { fprintf(stderr, "Exceeded MAX_STATES\n"); exit(1); }
 CLRstates[CLRstate_count++] = J;
 changed = 1;
 }
 }
 }
 }
 }
 }
}
/* Allocate ACTION/GOTO for CLR dynamically */
void alloc_CLR_tables() {
 int i, j;
 ACTION_CLR = (int **) malloc(sizeof(int *) * CLRstate_count);
 GOTO_CLR = (int **) malloc(sizeof(int *) * CLRstate_count);
 if (!ACTION_CLR || !GOTO_CLR) { fprintf(stderr, "malloc failed\n"); exit(1); }
 for (i = 0; i < CLRstate_count; i++) {
 ACTION_CLR[i] = (int *) malloc(sizeof(int) * term_count);
 GOTO_CLR[i] = (int *) malloc(sizeof(int) * sym_count);
 if (!ACTION_CLR[i] || !GOTO_CLR[i]) { fprintf(stderr, "malloc failed\n"); exit(1); }
 for (j = 0; j < term_count; j++) ACTION_CLR[i][j] = -1;
 for (j = 0; j < sym_count; j++) GOTO_CLR[i][j] = -1;
 }
}
/* Build CLR ACTION/GOTO tables */
void build_CLR_tables() {
 alloc_CLR_tables();
 /* We'll use a flattened temp_goto: temp_goto[s * sym_count + X] = target CLR state or -1 */
 int *temp_goto = (int *) malloc(sizeof(int) * CLRstate_count * sym_count);
 if (!temp_goto) { fprintf(stderr, "malloc failed\n"); exit(1); }
 {
 int i;
 for (i = 0; i < CLRstate_count * sym_count; ++i) temp_goto[i] = -1;
 }
 {
 int s, X;
 for (s = 0; s < CLRstate_count; ++s) {
 for (X = 0; X < sym_count; ++X) {
 State J; go_to(&CLRstates[s], X, &J);
 if (J.item_count == 0) continue;
 int found = -1;
 {
 int k;
 for (k = 0; k < CLRstate_count; ++k) if (state_equal(&CLRstates[k], &J)) { found = k; break; }
 }
 if (found != -1) temp_goto[s * sym_count + X] = found;
 }
 }
 }
 /* Populate ACTION_CLR and GOTO_CLR using LR(1) items */
 {
 int s, i2, t;
 for (s = 0; s < CLRstate_count; ++s) {
 State *st = &CLRstates[s];
 for (i2 = 0; i2 < st->item_count; ++i2) {
 Item *it = &st->items[i2];
 Prod *p = &prods[it->prod];
 if (it->dot < p->rhs_len) {
 int a = p->rhs[it->dot];
 if (is_terminal[a]) {
 int tidx = term_index_of_sym[a];
 int to = temp_goto[s * sym_count + a];
 if (tidx >= 0 && to != -1) ACTION_CLR[s][tidx] = to + 1; /* shift */
 }
 } else {
 if (it->prod == 0) {
 for (t = 0; t < term_count; ++t) if (it->look[t]) ACTION_CLR[s][t] = 100000; /* accept */
 } else {
 for (t = 0; t < term_count; ++t) if (it->look[t]) ACTION_CLR[s][t] = -(it->prod + 2); /* reduce */
 }
 }
 }
 {
 int X;
 for (X = 0; X < sym_count; ++X) {
 if (temp_goto[s * sym_count + X] != -1) GOTO_CLR[s][X] = temp_goto[s * sym_count + X];
 }
 }
 }
 }
 free(temp_goto);
}
/* Build LALR by merging states with same LR(0) core (merge lookahead sets) */
void build_LALR_from_CLR() {
 /* compute signature for each CLR state */
 char **signatures = (char **) malloc(sizeof(char *) * CLRstate_count);
 if (!signatures) { fprintf(stderr, "malloc failed\n"); exit(1); }
 {
 int s;
 for (s = 0; s < CLRstate_count; ++s) {
 signatures[s] = (char *) malloc(8192);
 if (!signatures[s]) { fprintf(stderr, "malloc failed\n"); exit(1); }
 core_signature(&CLRstates[s], signatures[s], 8192);
 }
 }
 int *mapping = (int *) malloc(sizeof(int) * CLRstate_count);
 if (!mapping) { fprintf(stderr, "malloc failed\n"); exit(1); }
 {
 int i;
 for (i = 0; i < CLRstate_count; ++i) mapping[i] = -1;
 }
 {
 int groups = 0;
 int s, t;
 for (s = 0; s < CLRstate_count; ++s) {
 if (mapping[s] != -1) continue;
 mapping[s] = groups;
 for (t = s + 1; t < CLRstate_count; ++t)
 if (strcmp(signatures[s], signatures[t]) == 0) mapping[t] = mapping[s];
 groups++;
 }
 LALRstate_count = groups;
 }
 LALRstates = (State *) malloc(sizeof(State) * LALRstate_count);
 if (!LALRstates) { fprintf(stderr, "malloc failed\n"); exit(1); }
 {
 int g;
 for (g = 0; g < LALRstate_count; ++g) LALRstates[g].item_count = 0;
 }
 /* merge items (union lookaheads for same cores) */
 {
 int s, it;
 for (s = 0; s < CLRstate_count; ++s) {
 int g = mapping[s];
 for (it = 0; it < CLRstates[s].item_count; ++it) {
 Item *ci = &CLRstates[s].items[it];
 int found = -1;
 {
 int j;
 for (j = 0; j < LALRstates[g].item_count; ++j) {
 if (LALRstates[g].items[j].prod == ci->prod && LALRstates[g].items[j].dot == ci->dot) { found = j; break; }
 }
 }
 if (found == -1) {
 if (LALRstates[g].item_count >= MAX_ITEMS_PER_STATE) { fprintf(stderr, "Exceeded items per LALR state\n"); exit(1); }
 LALRstates[g].items[LALRstates[g].item_count++] = *ci;
 } else {
 int la;
 for (la = 0; la < term_count; ++la) if (ci->look[la]) LALRstates[g].items[found].look[la] = 1;
 }
 }
 }
 }
 {
 int s;
 for (s = 0; s < CLRstate_count; ++s) free(signatures[s]);
 }
 free(signatures);
 free(mapping);
}
/* Allocate ACTION/GOTO tables for LALR */
void alloc_LALR_tables() {
 int i, j;
 ACTION_LALR = (int **) malloc(sizeof(int *) * LALRstate_count);
 GOTO_LALR = (int **) malloc(sizeof(int *) * LALRstate_count);
 if (!ACTION_LALR || !GOTO_LALR) { fprintf(stderr, "malloc failed\n"); exit(1); }
 for (i = 0; i < LALRstate_count; ++i) {
 ACTION_LALR[i] = (int *) malloc(sizeof(int) * term_count);
 GOTO_LALR[i] = (int *) malloc(sizeof(int) * sym_count);
 if (!ACTION_LALR[i] || !GOTO_LALR[i]) { fprintf(stderr, "malloc failed\n"); exit(1); }
 for (j = 0; j < term_count; ++j) ACTION_LALR[i][j] = -1;
 for (j = 0; j < sym_count; ++j) GOTO_LALR[i][j] = -1;
 }
}
/* Recompute CLR goto mapping temporarily and build LALR tables using state core mapping */
void build_LALR_tables_from_CLR() {
 /* First compute CLR goto mapping temp_goto[CLRstate][symbol] -> CLR target (flattened) */
 int *temp_goto = (int *) malloc(sizeof(int) * CLRstate_count * sym_count);
 if (!temp_goto) { fprintf(stderr, "malloc failed\n"); exit(1); }
 {
 int i;
 for (i = 0; i < CLRstate_count * sym_count; ++i) temp_goto[i] = -1;
 }
 {
 int s, X;
 for (s = 0; s < CLRstate_count; ++s) {
 for (X = 0; X < sym_count; ++X) {
 State J; go_to(&CLRstates[s], X, &J);
 if (J.item_count == 0) continue;
 int found = -1;
 {
 int k;
 for (k = 0; k < CLRstate_count; ++k) if (state_equal(&CLRstates[k], &J)) { found = k; break; }
 }
 if (found != -1) temp_goto[s * sym_count + X] = found;
 }
 }
 }
 /* Build mapping CLR state -> LALR state by core signature */
 char **signatures = (char **) malloc(sizeof(char *) * CLRstate_count);
 if (!signatures) { fprintf(stderr, "malloc failed\n"); exit(1); }
 {
 int s;
 for (s = 0; s < CLRstate_count; ++s) {
 signatures[s] = (char *) malloc(8192);
 if (!signatures[s]) { fprintf(stderr, "malloc failed\n"); exit(1); }
 core_signature(&CLRstates[s], signatures[s], 8192);
 }
 }
 int *map = (int *) malloc(sizeof(int) * CLRstate_count);
 if (!map) { fprintf(stderr, "malloc failed\n"); exit(1); }
 {
 int i;
 for (i = 0; i < CLRstate_count; ++i) map[i] = -1;
 }
 {
 int gcount = 0;
 int s, t;
 for (s = 0; s < CLRstate_count; ++s) {
 if (map[s] != -1) continue;
 map[s] = gcount;
 for (t = s + 1; t < CLRstate_count; ++t) if (strcmp(signatures[s], signatures[t]) == 0) map[t] = map[s];
 gcount++;
 }
 if (gcount != LALRstate_count) {
 /* adjust if needed */
 LALRstate_count = gcount;
 }
 }
 alloc_LALR_tables();
 /* Build GOTO_LALR */
 {
 int s, X;
 for (s = 0; s < CLRstate_count; ++s) {
 int L = map[s];
 for (X = 0; X < sym_count; ++X) {
 int t = temp_goto[s * sym_count + X];
 if (t == -1) continue;
 int T = map[t];
 GOTO_LALR[L][X] = T;
 }
 }
 }
 /* Build ACTION_LALR: map CLR actions into merged LALR states */
 {
 int s, i2;
 for (s = 0; s < CLRstate_count; ++s) {
 int L = map[s];
 State *st = &CLRstates[s];
 for (i2 = 0; i2 < st->item_count; ++i2) {
 Item *it = &st->items[i2];
 Prod *p = &prods[it->prod];
 if (it->dot < p->rhs_len) {
 int a = p->rhs[it->dot];
 if (is_terminal[a]) {
 int t = term_index_of_sym[a];
 int to = temp_goto[s * sym_count + a];
 if (t >= 0 && to != -1) {
 int T = map[to];
 ACTION_LALR[L][t] = T + 1; /* shift */
 }
 }
 } else {
 int la;
 if (it->prod == 0) {
 for (la = 0; la < term_count; ++la) if (it->look[la]) ACTION_LALR[L][la] = 100000;
 } else {
 for (la = 0; la < term_count; ++la) if (it->look[la]) ACTION_LALR[L][la] = -(it->prod + 2);
 }
 }
 }
 }
 }
 {
 int s;
 for (s = 0; s < CLRstate_count; ++s) free(signatures[s]);
 }
 free(signatures);
 free(map);
 free(temp_goto);
}
/* printing helpers */
void print_prod(int idx) {
 int i;
 printf("%s ->", sym_name[prods[idx].lhs]);
 for (i = 0; i < prods[idx].rhs_len; ++i) printf(" %s", sym_name[prods[idx].rhs[i]]);
}
/* Parse input using LALR table */
void parse_input_with_LALR(char *input_line) {
 char tokens[4096][MAX_TOKLEN];
 int tn = 0;
 char tmp[16384];
 strncpy(tmp, input_line, sizeof(tmp) - 1);
 tmp[sizeof(tmp) - 1] = 0;
 {
 char *p = strtok(tmp, " \t\r\n");
 while (p && tn < 4096) { strncpy(tokens[tn++], p, MAX_TOKLEN - 1); tokens[tn - 1][MAX_TOKLEN - 1] = 0; p = strtok(NULL, " \t\r\n"); }
 }
 if (tn == 0) { printf("No input provided\n"); return; }
 if (strcmp(tokens[tn - 1], "$") != 0) { strncpy(tokens[tn++], "$", MAX_TOKLEN - 1); }
 {
 int input_term_idx[4096];
 int i, s;
 for (i = 0; i < tn; ++i) {
 int sym = -1;
 for (s = 0; s < sym_count; ++s) if (strcmp(sym_name[s], tokens[i]) == 0) { sym = s; break; }
 if (sym == -1) { printf("Token '%s' not recognized in grammar symbols. Aborting parse.\n", tokens[i]); return; }
 if (!is_terminal[sym]) { printf("Token '%s' is not a terminal according to grammar. Aborting parse.\n", tokens[i]); return; }
 {
 int tidx = term_index_of_sym[sym];
 if (tidx == -1) { printf("Token '%s' not in terminal index. Aborting.\n", tokens[i]); return; }
 input_term_idx[i] = tidx;
 }
 }
 int state_stack[16384];
 int top = 0;
 state_stack[top++] = 0;
 int ip = 0;
 printf("\nParsing steps:\n");
 while (1) {
 int state = state_stack[top - 1];
 int a = input_term_idx[ip];
 int act = ACTION_LALR[state][a];
 int k;
 printf("Stack:");
 for (k = 0; k < top; ++k) printf(" %d", state_stack[k]);
 printf(" Next token: %s Action: ", sym_name[terminals[a]]);
 if (act == -1) { printf("error\n"); break; }
 if (act == 100000) { printf("accept\n"); printf("Input accepted.\n"); break; }
 if (act > 0) {
 printf("shift to %d\n", act - 1);
 state_stack[top++] = act - 1;
 ip++;
 } else if (act <= -2) {
 int prod_idx = -(act) - 2;
 printf("reduce by prod %d: ", prod_idx); print_prod(prod_idx); printf("\n");
 {
 int pop = prods[prod_idx].rhs_len;
 top -= pop;
 int stt = state_stack[top - 1];
 int A = prods[prod_idx].lhs;
 int gotoState = GOTO_LALR[stt][A];
 if (gotoState == -1) { printf("Goto error after reduction\n"); break; }
 state_stack[top++] = gotoState;
 }
 } else {
 printf("unknown action\n"); break;
 }
 }
 }
}
/* MAIN */
int main() {
 int i;
 for (i = 0; i < MAX_SYMS; ++i) is_terminal[i] = 1;
 for (i = 0; i < MAX_SYMS; ++i) term_index_of_sym[i] = -1;
 printf("Enter number of productions (not counting augmented production): ");
 {
 int n;
 if (scanf("%d", &n) != 1) return 0;
 getchar();
 if (n <= 0) { printf("Need at least one production.\n"); return 0; }
 printf("Enter productions, one per line, tokens separated by spaces. Example: E -> E + T\n");
 char line[4096];
 char tokens[256][MAX_TOKLEN];
 for (i = 0; i < n; ++i) {
 if (!fgets(line, sizeof(line), stdin)) { fprintf(stderr, "Unexpected EOF\n"); return 1; }
 if (line[0] == '\n' || line[0] == '\0') { i--; continue; }
 line[strcspn(line, "\n")] = '\0';
 char copy[4096];
 strncpy(copy, line, sizeof(copy) - 1);
 copy[sizeof(copy) - 1] = 0;
 {
 int tn = tokenize_line(copy, tokens);
 if (tn < 3) { printf("Bad production format: %s\n", line); return 1; }
 if (strcmp(tokens[1], "->") != 0 && strcmp(tokens[1], "?") != 0) { printf("Bad production arrow in: %s\n", line); return 1; }
 int lhs_sym = get_sym(tokens[0]);
 is_terminal[lhs_sym] = 0;
 Prod p; p.lhs = lhs_sym; p.rhs_len = 0;
 {
 int k;
 for (k = 2; k < tn; ++k) {
 int s = get_sym(tokens[k]);
 p.rhs[p.rhs_len++] = s;
 }
 }
 if (prod_count >= MAX_PRODS) { fprintf(stderr, "Too many productions\n"); return 1; }
 prods[prod_count++] = p;
 }
 }
 /* Create augmented production S' -> start (put at prods[0]) */
 {
 int start_sym = prods[0].lhs;
 int aug_sym = get_sym("S'");
 is_terminal[aug_sym] = 0;
 if (prod_count + 1 >= MAX_PRODS) { fprintf(stderr, "Too many productions\n"); return 1; }
 {
 int j;
 for (j = prod_count; j > 0; --j) prods[j] = prods[j - 1];
 }
 prods[0].lhs = aug_sym;
 prods[0].rhs_len = 1;
 prods[0].rhs[0] = start_sym;
 prod_count++;
 }
 /* determine terminals set */
 {
 int s;
 for (s = 0; s < sym_count; ++s) {
 if (is_terminal[s]) add_terminal_sym(s);
 }
 }
 compute_FIRST();
 build_CLR_collection();
 printf("\nConstructed CLR(1) states: %d\n", CLRstate_count);
 build_CLR_tables();
 /* print CLR states */
 {
 int s, it;
 for (s = 0; s < CLRstate_count; ++s) {
 printf("\nState %d:\n", s);
 State *st = &CLRstates[s];
 for (it = 0; it < st->item_count; ++it) {
 Item *item = &st->items[it];
 print_prod(item->prod);
 printf(" . at %d look={", item->dot);
 {
 int first = 1;
 int t;
 for (t = 0; t < term_count; ++t) if (item->look[t]) {
 if (!first) printf(", ");
 printf("%s", sym_name[terminals[t]]);
 first = 0;
 }
 }
 printf("}\n");
 }
 }
 }
 build_LALR_from_CLR();
 build_LALR_tables_from_CLR();
 printf("\nConstructed LALR(1) states: %d\n", LALRstate_count);
 /* print LALR states */
 {
 int s, it;
 for (s = 0; s < LALRstate_count; ++s) {
 printf("\nLALR State %d:\n", s);
 State *st = &LALRstates[s];
 for (it = 0; it < st->item_count; ++it) {
 Item *item = &st->items[it];
 print_prod(item->prod);
 printf(" . at %d look={", item->dot);
 {
 int first = 1;
 int t;
 for (t = 0; t < term_count; ++t) if (item->look[t]) {
 if (!first) printf(", ");
 printf("%s", sym_name[terminals[t]]);
 first = 0;
 }
 }
 printf("}\n");
 }
 }
 }
 /* Print LALR ACTION/GOTO tables */
 printf("\nLALR ACTION table (rows: states, cols: terminals):\n ");
 {
 int t;
 for (t = 0; t < term_count; ++t) printf("%8s", sym_name[terminals[t]]);
 }
 printf("\n");
 {
 int s, t;
 for (s = 0; s < LALRstate_count; ++s) {
 printf("%3d ", s);
 for (t = 0; t < term_count; ++t) {
 int a = ACTION_LALR[s][t];
 if (a == -1) printf("%8s", ".");
 else if (a == 100000) printf("%8s", "acc");
 else if (a > 0) { char buf[32]; sprintf(buf, "s%d", a - 1); printf("%8s", buf); }
 else { int pidx = -(a) - 2; char buf[32]; sprintf(buf, "r%d", pidx); printf("%8s", buf); }
 }
 printf("\n");
 }
 }
 printf("\nLALR GOTO table (rows: states, cols: nonterminals):\n ");
 {
 int s;
 for (s = 0; s < sym_count; ++s) if (!is_terminal[s]) printf("%8s", sym_name[s]);
 }
 printf("\n");
 {
 int i2, s;
 for (i2 = 0; i2 < LALRstate_count; ++i2) {
 printf("%3d ", i2);
 for (s = 0; s < sym_count; ++s) if (!is_terminal[s]) {
 int g = GOTO_LALR[i2][s];
 if (g == -1) printf("%8s", ".");
 else { char buf[16]; sprintf(buf, "%d", g); printf("%8s", buf); }
 }
 printf("\n");
 }
 }
 /* parse input */
 printf("\nEnter input tokens separated by spaces (end with $ or it will be appended):\n");
 if (!fgets(line, sizeof(line), stdin)) { fprintf(stderr, "Unexpected EOF\n"); return 1; }
 line[strcspn(line, "\n")] = 0;
 parse_input_with_LALR(line);
 }
 return 0;
}