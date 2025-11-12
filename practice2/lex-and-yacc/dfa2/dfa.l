%{
#include <stdio.h>
%}

%option noyywrap

%x A B C D E F

%%

[ab]*b[ab]{4} {printf("Accepted.");}

[ab]*b {
    printf("State: INITIAL -> A on 'b'\n");
    BEGIN A;
}

<A>[ab] {
    printf("State: A -> B on '%s'\n", yytext);
    BEGIN B;
}

<B>[ab] {
    printf("State: B -> C on '%s'\n", yytext);
    BEGIN C;
}

<C>[ab] {
    printf("State: C -> D on '%s'\n", yytext);
    BEGIN D;
}

<D>[ab] {
    printf("State: D -> E on '%s'\n", yytext);
    BEGIN E;
}

<E>\n {
    printf("State: E -> F on '\\n'\n");
    printf("✅ Accepted\n\n");
    BEGIN INITIAL;
}

.|\n {
    printf("❌ Rejected — resetting to INITIAL\n\n");
    BEGIN INITIAL;
}

%%

int main(void) {
    printf("Enter input strings (Ctrl+D to end):\n");
    yylex();
    return 0;
}
