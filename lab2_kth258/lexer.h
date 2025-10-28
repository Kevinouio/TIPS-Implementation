#ifndef LEXER_H
#define LEXER_H

// ============================================================================
// lexer.h - Token codes shared by the lexer (rules.l) and the parser
// ----------------------------------------------------------------------------
// Author: Derek Willis (Fall 2025)
//
// IMPORTANT (read me):
//   Students: you normally do NOT need to edit this file.
//   If you ever add a new token here, you must also:
//     (1) add a matching rule in rules.l, and
//     (2) update the parser to handle it.
//
// Purpose:
//   - Parser: knows which integer codes yylex() returns.
//   - Lexer : can 'return ARTICLE;' etc. using these names.
//
// Notes:
//   - TOK_EOF must be 0 (Flex returns 0 at end-of-file).
//   - Named tokens start at 256 to avoid collisions with raw character codes (0..255).
// ============================================================================

#include <cstdio>   // for std::FILE
using std::FILE;

// Token codes (distinct integers). Keep named tokens >= 256.
enum Token : int {
    TOK_EOF = 0,
    ARTICLE = 256,
    POSSESSIVE,
    ADJECTIVE,
    NOUN,
    VERB,
    ADVERB,
    UNKNOWN
};

// Scanner interface (provided by Flex in lex.yy.c). Compiled as C++ here.
int   yylex();       // returns one of the Token values above (or a char code)
extern char* yytext; // matched lexeme text
extern FILE* yyin;   // input stream for the lexer (set in driver.cpp)


#endif // LEXER_H
