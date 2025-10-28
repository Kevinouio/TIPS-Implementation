#ifndef DEBUG_H
#define DEBUG_H

// ============================================================================
// debug.h - Simple, optional debug helpers for parser development
// ----------------------------------------------------------------------------
// - Toggle with gDebug (see driver.cpp for the --debug flag).
// - All debug prints go to stderr (never pollutes required stdout).
// - dbgLine(...) indents based on a simple nesting depth counter.
// ============================================================================

#include <iostream>
#include <string>
#include "lexer.h"  // for token constants

using namespace std;

// Global toggle; off by default. Enable via --debug or set directly if needed.
inline bool gDebug = false;

// Simple indentation depth for enter/exit tracing.
inline int gDepth = 0;

// RAII helper: increments depth on enter, decrements on scope exit.
struct DebugIndent {
    DebugIndent()  { ++gDepth; }
    ~DebugIndent() { --gDepth; }
};

// Pretty names for token codes during debugging.
inline const char* tokenName(int t) {
    switch (t) {
        case TOK_EOF:    return "TOK_EOF";
        case ARTICLE:    return "ARTICLE";
        case POSSESSIVE: return "POSSESSIVE";
        case ADJECTIVE:  return "ADJECTIVE";
        case NOUN:       return "NOUN";
        case VERB:       return "VERB";
        case ADVERB:     return "ADVERB";
        case UNKNOWN:    return "UNKNOWN";   // <-- add
        default:         return "UNKNOWN_TOKEN";
    }
}


// Indented line to stderr based on current depth.
inline void dbgLine(const string& s) {
    if (!gDebug) return;
    for (int i = 0; i < gDepth; ++i) cerr << "  ";
    cerr << s << "\n";
}

// Non-indented one-liner to stderr.
inline void dbg(const string& s) {
    if (gDebug) cerr << s << "\n";
}

#endif // DEBUG_H
