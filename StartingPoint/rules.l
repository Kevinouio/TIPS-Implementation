/*****************************************************************************
 * rules.l - Flex Rules for TIPS Language Subset
 * MSU CSE 4714/6714 Capstone Project (Fall 2025)
 * Author: Kevin Ho
*****************************************************************************/
%option noyywrap nodefault nounput
%option yylineno

%{
#include "lexer.h"
%}

/* Definitions (Macros) */
WHITESPACE        [ \t\r\n]*
SCHAR             [^'\n]
S80               {SCHAR}{0,80}
S81PLUS           {SCHAR}{81,}
ID_SHORT          [A-Z][A-Z0-9]{0,7}
ID_LONG           [A-Z][A-Z0-9]{8,}
NUM               [0-9]
NUMS              {NUM}{1,}


%%

{WHITESPACE}                            { }

PROGRAM                               { return PROGRAM; }
BEGIN                                 { return TOK_BEGIN; }
END                                   { return END; }
WRITE                                 { return WRITE; }
READ                                  { return READ; }

VAR                                   { return VAR; }


INTEGER                               { return INTEGER; }

REAL                                  { return REAL; }
{NUMS}"."{NUMS}                           { return FLOATLIT; }
{NUMS}                                 { return INTLIT; }

"("                                   { return OPENPAREN; }
")"                                     { return CLOSEPAREN; }
";"                                     { return SEMICOLON; }
":="                                    { return ASSIGN; }
":"                                     { return COLON; }


{ID_LONG}                               { return UNKNOWN; }
{ID_SHORT}                              { return IDENT; }

"\'"{S81PLUS}"\'"                       { return UNKNOWN; }
"\'"{SCHAR}* { return UNKNOWN; }
"\'"{S80}"\'"                           { return STRINGLIT; }

.                                       { return UNKNOWN; }

%%