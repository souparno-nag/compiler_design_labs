%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex(void);
void yyerror(const char *s);
void emit(const char* format, ...);

int tempCount = 0;
int labelCount = 0;

char *newTemp() {
    char buf[16];
    sprintf(buf, "t%d", ++tempCount);
    return strdup(buf);
}
char *newLabel() {
    char buf[16];
    sprintf(buf, "L%d", ++labelCount);
    return strdup(buf);
}

// label stack
#define LPSTACK_MAX 1024
static char* label_stack[LPSTACK_MAX];
static int label_top = 0;

static void pushL(char *label) {
    if (label_top < LPSTACK_MAX)
        label_stack[label_top++] = label;
    else {
        fprintf(stderr, "label stack overflow\n");
        exit(1);
    }
}
static char* popL(void) {
    if (label_top <= 0) {
        fprintf(stderr, "label stack underflow\n");
        exit(1);
    }
    return label_stack[--label_top];
}
%}

%code requires {
typedef struct {
    char *place;
} node;
}

%union {
    int intval;
    char *id;
    node *nptr;
}

%token <id> ID
%token <intval> NUMBER
%token IF ELSE WHILE
%token EQ NE LE GE

%type <nptr> expr term factor b_expr

%left '+' '-'
%left '*' '/'
%nonassoc IF_NO_ELSE
%nonassoc ELSE

%%

program:
    stmt_list
;

stmt_list:
    | stmt_list stmt
;

stmt:
      expr_stmt
    | compound_stmt
    | if_stmt
    | while_stmt
;

compound_stmt:
    '{' stmt_list '}'
;

expr_stmt:
      assignment ';'
    | ';'
;

assignment:
    ID '=' expr {
        emit("%s = %s", $1, $3->place);
        free($1);
        free($3->place);
        free($3);
    }
;

expr:
      term { $$ = $1; }
    | expr '+' term {
        $$ = malloc(sizeof(node));
        $$->place = newTemp();
        emit("%s = %s + %s", $$->place, $1->place, $3->place);
        free($1->place); free($1);
        free($3->place); free($3);
    }
    | expr '-' term {
        $$ = malloc(sizeof(node));
        $$->place = newTemp();
        emit("%s = %s - %s", $$->place, $1->place, $3->place);
        free($1->place); free($1);
        free($3->place); free($3);
    }
;

term:
      factor { $$ = $1; }
    | term '*' factor {
        $$ = malloc(sizeof(node));
        $$->place = newTemp();
        emit("%s = %s * %s", $$->place, $1->place, $3->place);
        free($1->place); free($1);
        free($3->place); free($3);
    }
    | term '/' factor {
        $$ = malloc(sizeof(node));
        $$->place = newTemp();
        emit("%s = %s / %s", $$->place, $1->place, $3->place);
        free($1->place); free($1);
        free($3->place); free($3);
    }
;

factor:
      NUMBER {
        $$ = malloc(sizeof(node));
        char buf[32];
        sprintf(buf, "%d", $1);
        $$->place = strdup(buf);
    }
    | ID {
        $$ = malloc(sizeof(node));
        $$->place = strdup($1);
        free($1);
    }
    | '(' expr ')' { $$ = $2; }
;

if_stmt:
    /* if with else */
    IF '(' b_expr ')' {
        char *l_else = newLabel();
        char *l_end  = newLabel();
        emit("if %s == 0 goto %s", $3->place, l_else);
        pushL(l_end);
        pushL(l_else);
        free($3->place); free($3);
    }
    stmt ELSE {
        char* l_else = popL();
        char* l_end  = popL();
        emit("goto %s", l_end);
        emit("%s:", l_else);
        pushL(l_end);
    }
    stmt {
        char* l_end = popL();
        emit("%s:", l_end);
    }
|
    /* if without else */
    IF '(' b_expr ')' {
        char *l_after = newLabel();
        emit("if %s == 0 goto %s", $3->place, l_after);
        pushL(l_after);
        free($3->place); free($3);
    }
    stmt %prec IF_NO_ELSE {
        char* l_after = popL();
        emit("%s:", l_after);
    }
;

while_stmt:
{
    char* l_start = newLabel();
    emit("%s:", l_start);
    pushL(l_start);
}
WHILE '(' b_expr ')' {
    char* l_end = newLabel();
    emit("if %s == 0 goto %s", $4->place, l_end);
    pushL(l_end);
    free($4->place); free($4);
}
stmt {
    char* l_end   = popL();
    char* l_start = popL();
    emit("goto %s", l_start);
    emit("%s:", l_end);
}
;

b_expr:
      expr EQ expr { $$ = makeBoolTemp("==", $1, $3); }
    | expr NE expr { $$ = makeBoolTemp("!=", $1, $3); }
    | expr '<' expr { $$ = makeBoolTemp("<",  $1, $3); }
    | expr '>' expr { $$ = makeBoolTemp(">",  $1, $3); }
    | expr LE expr { $$ = makeBoolTemp("<=", $1, $3); }
    | expr GE expr { $$ = makeBoolTemp(">=", $1, $3); }
;
%%

void yyerror(const char *s) {
    fprintf(stderr,"Error: %s\n",s);
}

#include <stdarg.h>
void emit(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

int main(void) {
    yyparse();
    return 0;
}
