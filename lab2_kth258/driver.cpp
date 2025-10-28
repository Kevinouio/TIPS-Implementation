// ============================================================================
// driver.cpp - Entry point that wires the pieces together
// ----------------------------------------------------------------------------
// Author: Derek Willis (Fall 2025)
// Students: you usually do NOT need to edit this file.
// ----------------------------------------------------------------------------
//
// WHAT THIS PROGRAM DOES
//   1) Chooses the input source for the lexer (a file or stdin).
//   2) Calls the parser to build exactly one <sentence> as an AST.
//   3) Runs the Printer visitor to print a FULLY EXPANDED tree to stdout.
//
// HOW TO RUN
//   ./parse                      // read from stdin
//   ./parse input1.in            // read from the file "input1.in"
//   ./parse --debug input1.in    // enable extra debug tracing to stderr
//
// WHERE OUTPUT GOES
//   - Pretty tree  -> stdout
//   - Debug/errors -> stderr
//
// ABOUT --debug
//   - Turns on parser tracing (debug.h).
//   - Turns on Flex rule tracing (yy_flex_debug).
//   Both print to stderr instead of stdout.
// ============================================================================

#include <iostream>
#include <cstdio>      // fopen
#include <stdexcept>
#include <string>
#include "lexer.h"     // yyin (and FILE), token defs
#include "parser.h"    // parseStart()
#include "printer.h"   // Printer visitor
#include "debug.h"     // gDebug + debug helpers (stderr only)

using namespace std;

// Flex's internal debug flag (1 = on). We set this only when --debug is used.
extern int yy_flex_debug;


int main(int argc, char* argv[]) {
    const char* fileArg = nullptr;  // optional input filename (at most one)

    // Parse flags first, then an optional filename.
    // Accepted flags:
    //   --debug  or  -d   : enable parser + lexer debug tracing (to stderr)
    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        if (a == "--debug" || a == "-d") {
            gDebug = true;            // enable our parser debug (stderr)
        } else if (!fileArg) {
            fileArg = argv[i];        // remember the first non-flag as input file
        } else {
            cerr << "Usage: " << argv[0] << " [--debug|-d] [input_file]\n";
            return 1;
        }
    }

    // If --debug is on, also enable Flex's own rule tracing (stderr).
    yy_flex_debug = 0;          // force OFF by default
    if (gDebug) yy_flex_debug = 1;   // turn ON only with --debug / -d

    // Choose the input source for the lexer.
    if (fileArg && string(fileArg) != "-") {
        yyin = fopen(fileArg, "r");
        if (!yyin) {
            cerr << "Could not open input file: " << fileArg << "\n";
            return 1;
        }
    } else {
        yyin = stdin; // default: read from standard input (or '-' explicitly)
    }

    try {
        // Parse exactly one <sentence>. The parser will throw on the first error.
        auto root = parseStart();

        // Print the fully expanded tree to stdout (this is the graded output).
        Printer pp(cout);
        root->accept(pp);
        cout.flush();
    } catch (const runtime_error& e) {
        // Do not change error strings; they come from the parser.
        cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
