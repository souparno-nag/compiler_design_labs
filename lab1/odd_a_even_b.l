%{
/* C declarations here */
#include <stdio.h>
%}

%option noyywrap
%x EE OE EO OO

%%

<INITIAL>[ab] {
    // On first valid input character, transition to EE state and reprocess character
    unput(yytext[0]); // push character back to input
    BEGIN EE;
}

<EE>a       { BEGIN OE; }
<EE>b       { BEGIN EO; }

<OE>a       { BEGIN EE; }
<OE>b       { BEGIN OO; }

<EO>a       { BEGIN OO; }
<EO>b       { BEGIN EE; }

<OO>a       { BEGIN EO; }
<OO>b       { BEGIN OE; }

<*>\n       {
                if (YY_START == OE) {
                    printf("Odd number of a's and even number of b's. ACCEPTED\n");
                } else {
                    printf("REJECTED\n");
                }
                BEGIN EE;
            }

<*>.        ;

%%

int main() {
    printf("Enter strings (Ctrl+D to end):\n");
    yylex();
    return 0;
}