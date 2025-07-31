%{
#include <stdio.h>
%}

%option noyywrap

%x S0 S1 S2 S3

%%

<INITIAL>[abc] {
    // First input character: go to DFA start state S0
    unput(yytext[0]); // reprocess this character in S0
    BEGIN S0;
}

<S0>b       { BEGIN S1; }
<S0>[ac]    { BEGIN S0; }

<S1>c       { BEGIN S2; }
<S1>b       { BEGIN S1; }
<S1>a       { BEGIN S0; }

<S2>a       { BEGIN S3; }
<S2>b       { BEGIN S1; }
<S2>c       { BEGIN S0; }

<S3>[abc]   { BEGIN S3; }  // Stay in accepting state

<*>\n {
    if (YY_START == S3) {
        printf("ACCEPTED\n");
    } else {
        printf("REJECTED\n");
    }
    BEGIN INITIAL;  // Reset for next line
}

<*>. ;  // Ignore other characters

%%

int main() {
    printf("Enter strings over {a,b,c}, one per line (Ctrl+D to end):\n");
    yylex();
    return 0;
}
