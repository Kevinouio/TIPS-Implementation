#ifndef PARSER_H
#define PARSER_H

// ============================================================================
// parser.h - Recursive descent parser interface (student checklist)
// ----------------------------------------------------------------------------
// Author: Derek Willis (Fall 2025)
// Students: you usually do NOT need to edit this file. Implement in parser.cpp.
// ----------------------------------------------------------------------------

// PURPOSE
//   Declarations for the parsing functions that build the AST from tokens.
//
// GRAMMAR TO IMPLEMENT
//   <sentence>         -> <noun phrase> <verb phrase> <noun phrase>
//   <noun phrase>      -> <adjective phrase> NOUN
//   <adjective phrase> -> (ARTICLE | POSSESSIVE) ADJECTIVE
//   <verb phrase>      -> VERB | ADVERB <verb phrase>
//   We will STORE verb phrases as:  ADVERB*  then  VERB.
//
// TOKENS (from the lexer)
//   ARTICLE, POSSESSIVE, ADJECTIVE, NOUN, VERB, ADVERB, TOK_EOF
//
// REQUIRED ERROR STRINGS (exact text)
//   "<sentence> did not start with an article or possessive."
//   "<noun phrase> did not start with an article or possessive."
//   "<noun phrase> did not have a noun."
//   "<adjective phrase> did not start with an article or possessive."
//   "<adjective phrase> did not have an adjective."
//   "<verb phrase> did not start with a verb or an adverb."
//
// HOW TO USE (in main)
//   auto root = parseStart();   // builds and returns a Sentence*
//   Printer pp(cout); root->accept(pp);
// ============================================================================

#include <memory>
#include "ast.h"

using namespace std;

// Entry: initializes scanning, parses one sentence, enforces EOF.
unique_ptr<Sentence> parseStart();

// One function per nonterminal (you will implement bodies in parser.cpp).
unique_ptr<Sentence>        parseSentence();
unique_ptr<NounPhrase>      parseNounPhrase();
unique_ptr<AdjectivePhrase> parseAdjectivePhrase();
unique_ptr<VerbPhrase>      parseVerbPhrase();

#endif // PARSER_H
