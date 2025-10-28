/* ============================================================================
   rules.l - Flex scanner (lexer) for the Parsing + OO Lab
   ----------------------------------------------------------------------------
   Author: Derek Willis (Fall 2025)

   IMPORTANT:
     Students: you MAY edit the regex groups below (ARTICLE_RX, NOUN_RX, etc.)
     to add vocabulary. Do NOT rename tokens without also updating:
       (1) lexer.h (add token to enum), and
       (2) parser.cpp (teach the parser how to use it).

   PURPOSE
     - Read characters and return token codes to the parser (see lexer.h).
     - Whitespace is ignored.
     - End-of-file returns TOK_EOF.

   FLEX OPTIONS
     caseless : match words case-insensitively ("The" == "the").
     noyywrap : no yywrap() needed at EOF.
     noinput/nounput : avoid generating unused functions (no warnings).
     nodefault : force us to handle all chars (we include a catch-all rule).
     debug : compile in Flex's internal tracing; we'll toggle it at runtime
             when --debug is used so normal runs stay quiet.
   ============================================================================ */

%option caseless
%option noyywrap
%option noinput
%option nounput
%option nodefault
%option debug

%{
#include "lexer.h"    // token codes
 /* Flex debug control (set at runtime in driver.cpp when --debug is passed). */
extern int yy_flex_debug;
%}

 /* ----------------------- Regex macros (you may edit these) ------------------------ */
 /* Add words as desired. Keep it PG-13*/

WS              [ \t\r\n]+

ARTICLE_RX      (the|a|an)
POSSESSIVE_RX   (my|your|his|her|our|their)
ADJECTIVE_RX    (green|blue|little|small|happy)
NOUN_RX         (dog|trousers|pony|nose|homework)
VERB_RX         (eat|eats|ate)
ADVERB_RX       (quickly|slowly|really)

 /* =============================== RULES ==================================== */
%%
{WS}                { /* skip whitespace; produce no token */ }

{ARTICLE_RX}        { return ARTICLE; }
{POSSESSIVE_RX}     { return POSSESSIVE; }
{ADJECTIVE_RX}      { return ADJECTIVE; }
{NOUN_RX}           { return NOUN; }
{VERB_RX}           { return VERB; }
{ADVERB_RX}         { return ADVERB; }

 /* Optional: allow a trailing period ('.') */
[.,!?:;]                 { /* ignore punctuation */ }

 /* Everything else becomes UNKNOWN (words, punctuation, etc.). */
.                   { return UNKNOWN; }

 /* End-of-file */
<<EOF>>             { return TOK_EOF; }
%%
/* ============================ END OF RULES ================================ */

/* NOTES FOR STUDENTS
   1) Extend vocabulary by appending words to the *_RX groups above.
   2) Case-insensitive matching is ON by default (The == the).
   3) Debugging:
      - Run with:  ./parse --debug input1.in
      - You will see parser traces (our debug.h) and, because %option debug is set,
        Flex's own "which rule matched" messages. Both go to stderr.
*/
