/* clr_lr1.c
CLR (LR(1)) parser generator + parser in C (single-char tokens)
- Productions: A->alpha (no spaces required). Use '#' for epsilon.
- Nonterminals: uppercase letters A-Z.
- Terminals: other single chars (use 'i' for id).
- The program augments grammar with S' -> start and uses $ as end-marker.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_PRODS 200
#define MAX_RHS 64
#define MAX_STATES 600
#define MAX_ITEMS_PER_STATE 512
#define ASCII 256
typedef struct {
char lhs;
char rhs[MAX_RHS];
int rhs_len;
} Prod;
/* Global grammar */
static Prod prods[MAX_PRODS];
static int prod_count = 0;
/* which characters appear in grammar */
static int used_sym[ASCII];
static int is_nonterm[ASCII];
/* FIRST sets: FIRST[X][t] == 1 if terminal t in FIRST(X) */
static unsigned char FIRST[ASCII][ASCII];
/* CLR states */
typedef struct {
int prod[MAX_ITEMS_PER_STATE];
int dot[MAX_ITEMS_PER_STATE];
unsigned char look[MAX_ITEMS_PER_STATE][ASCII]; /* lookahead bitset */
int n;
} State;
static State *CLR = NULL;
static int CLR_count = 0;
/* ACTION, GOTO tables */
static int ACTION[MAX_STATES][ASCII]; /* -999 empty; >0 shift (state+1); <= -2 reduce
-(prod+2); 100000 accept */
static int GOTO_TABLE[MAX_STATES][ASCII]; /* -999 empty, else state */
/* Utility helpers */
static void die(const char *msg) { fprintf(stderr, "%s\n", msg); exit(1); }
/* Trim spaces */
static void trim_whitespace(char *s) {
char *p = s;
while (*p && isspace((unsigned char)*p)) p++;
if (p != s) memmove(s, p, strlen(p)+1);
int L = strlen(s);
while (L>0 && isspace((unsigned char)s[L-1])) s[--L] = '\0';
}
/* Remove spaces from a copy (useful to parse A->alpha when user may write spaces) */
static void remove_spaces(const char *in, char *out) {
int i,j = 0;
for (i=0; in[i]; ++i) {
if (!isspace((unsigned char)in[i])) out[j++] = in[i];
}
out[j] = '\0';
}
/* Read grammar lines. Accepts A->alpha with optional spaces around tokens. */
static void read_grammar() {
int n;
printf("Enter number of productions: ");
if (scanf("%d", &n) != 1) die("bad number");
getchar();
if (n <= 0) die("need at least one production");
int i;
for (i = 0; i < n; ++i) {
char line[1024];
if (!fgets(line, sizeof(line), stdin)) die("unexpected EOF");
/* allow blank lines */
if (line[0] == '\n') { i--; continue; }
line[strcspn(line, "\n")] = '\0';
trim_whitespace(line);
if (strlen(line) == 0) { i--; continue; }
char compact[1024];
remove_spaces(line, compact);
char *arrow = strstr(compact, "->");
if (!arrow || arrow == compact) {
fprintf(stderr, "Invalid production: %s\n", line);
exit(1);
}
/* LHS must be single uppercase char */
char lhs = compact[0];
if (!isupper((unsigned char)lhs)) {
fprintf(stderr, "LHS must be uppercase nonterminal (single char), got '%c' in: %s\n", lhs,
line);
exit(1);
}
char *rhsstart = arrow + 2;
if (rhsstart[0] == '\0') {
/* epsilon */
rhsstart = "#";
}
if (prod_count >= MAX_PRODS-2) die("too many productions");
prods[prod_count].lhs = lhs;
prods[prod_count].rhs_len = 0;
/* rhsstart is compact (no spaces). Each character is a symbol. '#' stands for epsilon. */
if (strcmp(rhsstart, "#") == 0) {
prods[prod_count].rhs_len = 0;
} else {
int k;
for (k = 0; rhsstart[k] != '\0'; ++k) {
prods[prod_count].rhs[prods[prod_count].rhs_len++] = rhsstart[k];
used_sym[(unsigned char)rhsstart[k]] = 1;
}
}
used_sym[(unsigned char)lhs] = 1;
is_nonterm[(unsigned char)lhs] = 1;
prod_count++;
}
}
/* Build symbol info and ensure '#' and '$' present */
static void finalize_symbols() {
used_sym[(unsigned char)'#'] = 1; /* epsilon symbol */
used_sym[(unsigned char)'$'] = 1; /* end marker */
is_nonterm[(unsigned char)'#'] = 0;
is_nonterm[(unsigned char)'$'] = 0;
}
/* FIRST helpers */
/* Add a terminal t to FIRST[X]; returns 1 if changed */
static int FIRST_add(int X, int t) {
if (!FIRST[X][t]) { FIRST[X][t] = 1; return 1; }
return 0;
}
/* Compute FIRST sets (classic fixed-point). '#' used for epsilon. */
static void compute_FIRST() {
/* clear */
int i,j,ch;
for (i = 0; i < ASCII; ++i) for (j = 0; j < ASCII; ++j) FIRST[i][j] = 0;
/* terminals: FIRST[t] contains t */
for (ch = 0; ch < ASCII; ++ch) {
if (used_sym[ch] && !is_nonterm[ch]) {
FIRST[ch][ch] = 1;
}
}
/* epsilon '#' first contains '#' */
FIRST['#']['#'] = 1;
int changed = 1;
while (changed) {
changed = 0;
int p,pos,t;
for (p = 0; p < prod_count; ++p) {
int A = (unsigned char)prods[p].lhs;
/* compute FIRST(rhs) */
int all_nullable = 1;
for (pos = 0; pos < prods[p].rhs_len; ++pos) {
int X = (unsigned char)prods[p].rhs[pos];
/* add FIRST[X] \ {#} to FIRST[A] */
for (t = 0; t < ASCII; ++t) {
if (FIRST[X][t] && t != (unsigned char)'#') {
if (FIRST_add(A, t)) changed = 1;
}
}
if (!FIRST[X][(unsigned char)'#']) { all_nullable = 0; break; }
}
if (prods[p].rhs_len == 0) all_nullable = 1;
if (all_nullable) {
if (FIRST_add(A, (unsigned char)'#')) changed = 1;
}
}
}
}
/* Compute FIRST of sequence beta followed by lookahead la (la is a char code), result in out[]
bitset */
static void FIRST_of_sequence(char beta[], int blen, int la_char, unsigned char out[ASCII]) {
int i,t;
for (i = 0; i < ASCII; ++i) out[i] = 0;
if (blen == 0) { out[la_char] = 1; return; }
int all_nullable = 1;
for (i = 0; i < blen; ++i) {
int X = (unsigned char)beta[i];
/* add FIRST[X] \ {#} */
for (t = 0; t < ASCII; ++t) {
if (FIRST[X][t] && t != (unsigned char)'#') out[t] = 1;
}
if (!FIRST[X][(unsigned char)'#']) { all_nullable = 0; break; }
}
if (all_nullable) out[la_char] = 1;
}
/* State item operations */
static int add_item(State *S, int prod_idx, int dotpos, unsigned char *lookset) {
/* find core match */
int i,t;
for (i = 0; i < S->n; ++i) {
if (S->prod[i] == prod_idx && S->dot[i] == dotpos) {
/* union looksets; return 1 if changed */
int changed = 0;
for (t = 0; t < ASCII; ++t) {
if (lookset[t] && !S->look[i][t]) { S->look[i][t] = 1; changed = 1; }
}
return changed;
}
}
if (S->n >= MAX_ITEMS_PER_STATE) die("Exceeded items per state");
S->prod[S->n] = prod_idx;
S->dot[S->n] = dotpos;
memcpy(S->look[S->n], lookset, ASCII);
S->n++;
return 1;
}
/* Closure: operate in-place on state; returns nothing */
static void closure(State *S) {
int changed = 1;
while (changed) {
changed = 0;
/* iterate items */
int i;
for (i = 0; i < S->n; ++i) {
int p = S->prod[i];
int d = S->dot[i];
if (d < prods[p].rhs_len) {
char B = prods[p].rhs[d];
if (is_nonterm[(unsigned char)B]) {
/* build beta (symbols after B) */
char beta[MAX_RHS];
int blen = 0;
int k;
for (k = d + 1; k < prods[p].rhs_len; ++k) beta[blen++] = prods[p].rhs[k];
/* for each lookahead symbol 'a' in S->look[i], compute FIRST(beta + a) and add
items */
int la;
for (la = 0; la < ASCII; ++la) if (S->look[i][la]) {
unsigned char fseq[ASCII];
FIRST_of_sequence(beta, blen, la, fseq);
/* for every production B->gamma */
int q,b,z;
for (q = 0; q < prod_count; ++q) {
if (prods[q].lhs == B) {
/* for every terminal b in fseq (except '#') add [B->.gamma, b] */
for (b = 0; b < ASCII; ++b) {
if (fseq[b] && (unsigned char)b != (unsigned char)'#') {
unsigned char tmplook[ASCII];
for (z=0; z<ASCII; ++z) tmplook[z]=0;
tmplook[b] = 1;
if (add_item(S, q, 0, tmplook)) changed = 1;
}
}
}
}
}
}
}
}
}
}
/* goto operation: produce new state J from state I on symbol X */
static State goto_on(const State *I, char X) {
State J; J.n = 0;
int i;
for (i = 0; i < I->n; ++i) {
int p = I->prod[i];
int d = I->dot[i];
if (d < prods[p].rhs_len && prods[p].rhs[d] == X) {
/* copy lookset */
unsigned char tmp[ASCII];
memcpy(tmp, I->look[i], ASCII);
add_item(&J, p, d+1, tmp);
}
}
if (J.n == 0) return J;
closure(&J);
return J;
}
/* Compare two states for LR(1) equality (items + looksets) */
static int states_equal(const State *A, const State *B) {
if (A->n != B->n) return 0;
int i;
for (i = 0; i < A->n; ++i) {
int j,found = 0;
for (j = 0; j < B->n; ++j) {
if (A->prod[i] == B->prod[j] && A->dot[i] == B->dot[j] && memcmp(A->look[i], B->look[j],
ASCII) == 0) { found = 1; break; }
}
if (!found) return 0;
}
return 1;
}
/* Find state index in CLR[] that equals S; return -1 if not found */
static int find_state(const State *S) {
int i;
for (i = 0; i < CLR_count; ++i) {
if (states_equal(S, &CLR[i])) return i;
}
return -1;
}
/* Build canonical LR(1) collection */
static void build_CLR_collection() {
CLR = (State *) malloc(sizeof(State) * MAX_STATES);
if (!CLR) die("malloc CLR");
CLR_count = 0;
/* initial item [S' -> .start, $] */
State I0; I0.n = 0;
int t;
unsigned char init_look[ASCII]; for (t=0;t<ASCII;t++) init_look[t]=0;
init_look[(unsigned char)'$'] = 1;
add_item(&I0, 0, 0, init_look);
closure(&I0);
CLR[CLR_count++] = I0;
int changed = 1;
/* list of used symbols for transitions (only symbols that appear in grammar) */
char syms[ASCII]; int syms_n = 0;
int c,s,si;
for (c = 0; c < ASCII; ++c) if (used_sym[c]) syms[syms_n++] = (char)c;
while (changed) {
changed = 0;
for (s = 0; s < CLR_count; ++s) {
for (si = 0; si < syms_n; ++si) {
char X = syms[si];
State J = goto_on(&CLR[s], X);
if (J.n == 0) continue;
if (find_state(&J) == -1) {
if (CLR_count >= MAX_STATES) die("too many CLR states");
CLR[CLR_count++] = J;
changed = 1;
}
}
}
}
}
/* Allocate/initialize ACTION and GOTO table */
static void init_tables() {
int i,c;
for (i = 0; i < MAX_STATES; ++i) {
for (c = 0; c < ASCII; ++c) {
ACTION[i][c] = -999;
GOTO_TABLE[i][c] = -999;
}
}
}
/* Build ACTION & GOTO from CLR states */
static void build_tables() {
init_tables();
/* compute temp goto mapping by computing goto for each CLR state and symbol, to avoid
recomputing */
/* but simpler: for each state s and symbol X compute goto_on and find index j. Use
find_state. */
int s;
for (s = 0; s < CLR_count; ++s) {
State *st = &CLR[s];
int i;
for (i = 0; i < st->n; ++i) {
int p = st->prod[i];
int d = st->dot[i];
if (d < prods[p].rhs_len) {
char a = prods[p].rhs[d];
if (!is_nonterm[(unsigned char)a]) {
State J = goto_on(st, a);
if (J.n == 0) continue;
int j = find_state(&J);
if (j == -1) die("goto state not found");
int tindex = (unsigned char)a;
/* shift to j: encode as j+1 */
if (ACTION[s][tindex] == -999) ACTION[s][tindex] = j + 1;
else if (ACTION[s][tindex] != j + 1) {
printf("Shift/other conflict at ACTION[%d]['%c'] existing=%d new=%d\n",
s, a, ACTION[s][tindex], j+1);
}
} else {
/* nonterminal -> GOTO entry */
State J = goto_on(st, a);
if (J.n == 0) continue;
int j = find_state(&J);
if (j == -1) die("goto state not found for nonterminal");
GOTO_TABLE[s][(unsigned char)a] = j;
}
} else {
/* dot at end -> reduce (or accept) */
if (p == 0) {
/* augmented production -> accept on $ */
int dollar = (unsigned char)'$';
if (ACTION[s][dollar] == -999) ACTION[s][dollar] = 100000;
else if (ACTION[s][dollar] != 100000)
printf("Conflict at ACTION[%d]['$']\n", s);
} else {
/* reduce by production p on each lookahead in item i */
int la;
for (la = 0; la < ASCII; ++la) if (st->look[i][la]) {
if (ACTION[s][la] == -999) ACTION[s][la] = -(p + 2);
else if (ACTION[s][la] != -(p + 2)) {
printf("Reduce/other conflict at ACTION[%d]['%c'] existing=%d new=%d\n", s,
(char)la, ACTION[s][la], -(p + 2));
}
}
}
}
}
}
}
/* Print FIRST sets (nonterminals) - debugging */
static void print_FIRST() {
printf("\nFIRST sets (showing terminals):\n");
int c,t;
for (c = 0; c < ASCII; ++c) if (used_sym[c] && is_nonterm[c]) {
printf("FIRST(%c) = { ", c);
for (t = 0; t < ASCII; ++t) if (FIRST[c][t]) printf("%c ", t);
printf("}\n");
}
}
/* Print CLR states (items) */
static void print_CLR_states() {
printf("\nCLR(1) canonical collection (%d states):\n", CLR_count);
int s;
for (s = 0; s < CLR_count; ++s) {
printf("I%d:\n", s);
State *st = &CLR[s];
int i;
for (i = 0; i < st->n; ++i) {
int p = st->prod[i], d = st->dot[i];
printf(" (%d) %c -> ", p, prods[p].lhs);
int k;
for (k = 0; k <= prods[p].rhs_len; ++k) {
if (k == d) printf(".");
if (k < prods[p].rhs_len) printf("%c", prods[p].rhs[k]);
}
printf(" , {");
int first = 1;
int la;
for (la = 0; la < ASCII; ++la) if (st->look[i][la]) {
if (!first) printf(", "); first = 0;
printf("%c", la);
}
printf("}\n");
}
}
}
/* Print ACTION and GOTO table neatly */
static void print_parsing_table() {
/* build list of terminal symbols to print (exclude '#', include '$') */
char term_list[ASCII]; int term_n = 0;
int c;
for (c = 0; c < ASCII; ++c) {
if (used_sym[c] && !is_nonterm[c]) {
if (c == (unsigned char)'#') continue;
term_list[term_n++] = (char)c;
}
}
/* build list of nonterminals */
char nonterm_list[ASCII]; int nonterm_n = 0;
for (c = 0; c < ASCII; ++c) if (used_sym[c] && is_nonterm[c]) nonterm_list[nonterm_n++] =
(char)c;
printf("\nParsing TABLE (CLR):\n ");
int t;
for (t = 0; t < term_n; ++t) printf("%6c", term_list[t]);
printf(" |");
int nt;
for (nt = 0; nt < nonterm_n; ++nt) printf("%6c", nonterm_list[nt]);
printf("\n");
int s;
for (s = 0; s < CLR_count; ++s) {
printf("%3d ", s);
for (t = 0; t < term_n; ++t) {
int col = (unsigned char)term_list[t];
int a = ACTION[s][col];
if (a == -999) printf("%6s", ".");
else if (a == 100000) printf("%6s", "acc");
else if (a > 0) { char buf[16]; snprintf(buf, sizeof(buf), "s%d", a-1); printf("%6s", buf); }
else { int pidx = -(a) - 2; char buf[16]; snprintf(buf, sizeof(buf), "r%d", pidx); printf("%6s",
buf); }
}
printf(" |");
for (nt = 0; nt < nonterm_n; ++nt) {
int col = (unsigned char)nonterm_list[nt];
int g = GOTO_TABLE[s][col];
if (g == -999) printf("%6s", ".");
else { char buf[16]; snprintf(buf, sizeof(buf), "%d", g); printf("%6s", buf); }
}
printf("\n");
}
}
/* Preprocess input: if token "id" appears, map to 'i'. If input contains spaces, treat tokens,
otherwise char-by-char. */
static int build_input_tokens(const char *line, char tokens[], int *tcount) {
char tmp[4096];
strncpy(tmp, line, sizeof(tmp)-1); tmp[sizeof(tmp)-1]=0;
/* if spaces present, split by spaces */
int count = 0;
char *p = strtok(tmp, " \t\r\n");
if (p == NULL) { *tcount = 0; return 0; }
/* if only one token and no spaces => also accept char-by-char */
int multi = strchr(line, ' ') != NULL || strchr(line, '\t') != NULL;
if (!multi && strlen(p) > 1) {
/* treat string as sequence of characters */
int i;
for (i=0;i<strlen(p);++i) {
/* map "id" sequence -> 'i' (if appears), otherwise each char */
if (i+1 < strlen(p) && p[i]=='i' && p[i+1]=='d') { tokens[count++] = 'i'; ++i; }
else tokens[count++] = p[i];
}
} else {
/* tokens separated by spaces */
while (p) {
if (strcmp(p, "id") == 0) tokens[count++] = 'i';
else if (strlen(p) == 1) tokens[count++] = p[0];
else {
/* if user typed "id" as single token or something unknown, attempt to compress to first
char */
if (strcmp(p, "i") == 0) tokens[count++] = 'i';
else {
//fprintf(stderr, "Unknown token '%s' in input; please use single-char tokens (or 'id' for
//identifier).\n", p);
return 0;
}
}
p = strtok(NULL, " \t\r\n");
}
}
*tcount = count;
return 1;
}
/* Parse using CLR ACTION/GOTO */
static void parse_input(const char *line) {
char tokens[4096];
int tn = 0;
if (!build_input_tokens(line, tokens, &tn)) return;
/* append $ if not present */
if (tn == 0) { printf("No tokens provided\n"); return; }
if (tokens[tn-1] != '$') tokens[tn++] = '$';
/* check tokens are terminals */
int i;
for (i = 0; i < tn; ++i) {
int ch = (unsigned char)tokens[i];
if (!used_sym[ch] || is_nonterm[ch]) {
fprintf(stderr, "Token '%c' not recognized as terminal in grammar. Aborting parse.\n",
tokens[i]);
return;
}
}
int stack[16384]; int top = 0;
stack[top++] = 0;
int ip = 0;
printf("\nParsing steps:\n");
printf("%-30s %-20s %s\n", "Stack(states)", "Remaining", "Action");
printf("---------------------------------------------------------------\n");
while (1) {
/* stack display */
char sd[512] = ""; char rem[512] = "";
int j;
for (j = 0; j < top; ++j) { char buf[16]; snprintf(buf, sizeof(buf), "%d ", stack[j]); strncat(sd,
buf, sizeof(sd)-strlen(sd)-1); }
for (j = ip; j < tn; ++j) { char buf[4]; snprintf(buf, sizeof(buf), "%c", tokens[j]); strncat(rem,
buf, sizeof(rem)-strlen(rem)-1); if (j+1<tn) strncat(rem, " ", sizeof(rem)-strlen(rem)-1); }
int state = stack[top-1];
int a = (unsigned char)tokens[ip];
int act = ACTION[state][a];
if (act == -999) {
printf("%-30s %-20s ERROR(no action)\n", sd, rem);
break;
}
if (act == 100000) {
printf("%-30s %-20s %s\n", sd, rem, "Accept");
printf("Input accepted.\n");
break;
}
if (act > 0) {
int to = act - 1;
printf("%-30s %-20s Shift %d\n", sd, rem, to);
stack[top++] = to;
ip++;
continue;
}
if (act <= -2) {
int pidx = -(act) - 2;
/* perform reduction by pidx */
Prod *pr = &prods[pidx];
char rhsstr[MAX_RHS + 1] = ""; // FIX: Sized appropriately
if (pr->rhs_len == 0) strcpy(rhsstr, "#");
else {
    int k;
for (k=0;k<pr->rhs_len;++k) { char b[2] = { pr->rhs[k], '\0' }; strncat(rhsstr, b,
sizeof(rhsstr)-strlen(rhsstr)-1); }
}
char actmsg[256]; // FIX: Increased buffer size
snprintf(actmsg, sizeof(actmsg), "Reduce by %c->%s (prod %d)", pr->lhs, rhsstr, pidx);
printf("%-30s %-20s %s\n", sd, rem, actmsg);
/* pop */
int popc = pr->rhs_len;
/* if rhs is epsilon (#) then pop 0 */
if (pr->rhs_len == 1 && pr->rhs[0] == '#') popc = 0;
top -= popc;
if (top <= 0) { printf("Parse stack underflow\n"); return; }
int stt = stack[top-1];
int g = GOTO_TABLE[stt][(unsigned char)pr->lhs];
if (g == -999) { printf("Goto missing after reduce: GOTO[%d][%c]\n", stt, pr->lhs); return; }
stack[top++] = g;
continue;
}
}
}
/* ------------------- Main ------------------- */
int main() {
/* read grammar */
memset(used_sym, 0, sizeof(used_sym));
memset(is_nonterm, 0, sizeof(is_nonterm));
prod_count = 0;
read_grammar();
finalize_symbols();
/* augment grammar: insert S' -> S (at prods[0]) */
if (prod_count + 1 >= MAX_PRODS) die("Too many productions");
int i;
for (i = prod_count; i >= 1; --i) prods[i] = prods[i-1];
/* choose augmented symbol S' stored as character '!' (rarely used) or "S'"? We need
single-char; choose '`' if safe.
To keep single-char, pick uppercase not used; else use 'Z'+1 not possible.
Simpler: use character 1 (ASCII SOH) as augmented lhs, but printing would be odd.
We'll use character '@' as augmented lhs (unlikely to be in grammar). */
char aug = '@';
/* ensure '@' not already used; if used, pick non-used ASCII */
if (used_sym[(unsigned char)aug]) {
int c;
for (c = 'A'; c <= 'Z'; ++c) if (!used_sym[c]) { aug = (char)c; break; }
}
prods[0].lhs = aug;
prods[0].rhs_len = 1;
prods[0].rhs[0] = prods[1].lhs; /* original start symbol */
used_sym[(unsigned char)aug] = 1;
is_nonterm[(unsigned char)aug] = 1;
prod_count++;
/* compute FIRST sets */
compute_FIRST();
/* Build canonical LR(1) collection */
build_CLR_collection();
/* print debug info */
//print_FIRST();
//print_CLR_states();
/* Build ACTION and GOTO tables */
build_tables();
/* print parsing table */
print_parsing_table();
/* parse input */
printf("\nEnter input string (tokens single-char, or use 'id' which maps to 'i'): ");
char inbuf[4096];
// Read twice to consume potential leftover newline from previous scanf
if (!fgets(inbuf, sizeof(inbuf), stdin)) { /* ignore empty first read */ }
if (!fgets(inbuf, sizeof(inbuf), stdin)) { printf("No input given. Exiting.\n"); return 0; }
inbuf[strcspn(inbuf, "\n")] = '\0';
parse_input(inbuf);
return 0;
}
