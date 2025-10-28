// ============================================================================
// ast.cpp - accept(...) definitions (double dispatch glue) [SOLUTION]
// ----------------------------------------------------------------------------
 // Author: Derek Willis (Fall 2025)
// ============================================================================

#include "ast.h"
#include "printer.h"

using namespace std;

// TODO: implement accept function for each grammar
void NounPhrase::accept(Printer& p) { p.visit(*this); }
void VerbPhrase::accept(Printer& p) { p.visit(*this); }
void AdjectivePhrase::accept(Printer& p) { p.visit(*this); }
void Sentence::accept(Printer& p) { p.visit(*this); }