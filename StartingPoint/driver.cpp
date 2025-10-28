// =============================================================================
// driver.cpp - Interpreter driver for TIPS Language Subset
// =============================================================================
// MSU CSE 4714/6714 Capstone Project (Fall 2025)
// Author: Derek Willis
//
// Purpose
// -------
// A small driver that wires together the classic compiler phases
// used in this course project:
//   (1) Lexing  - optional token dump (-t)
//   (2) Parsing - optional AST print (-p)
//   (3) Interpreting the parsed Program
//   (4) Optional symbol table printing (-s) [Part 2]
//
// Note: Flex returns 0 on EOF; we map this to TOK_EOF so token dumps
// are consistent and easy to interpret.
//
// TODO [Part 2]:
//   - Decide and document the symbol table format printed by Program::print_symbols().
//   - Example target format:
//       name : TYPE = value
//       counter : INTEGER = 42
//
// TODO [Part 4]:
//   - Validate --skin=NAME against a known list from the scanner (e.g. default,
//     INITIAL, pirate, cat). If unknown, show suggestions.
//   - Consider a --list-skins flag that queries the scanner for available skins.
// =============================================================================
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include "lexer.h"  // Scanner functions: yylex, yyin, yylineno, yytext, tokName()
#include "debug.h"  // Debug flag support: dbg::set(bool)
#include "ast.h"    // Program AST type with interpret() and print_symbols()
using namespace std;
// -----------------------------------------------------------------------------
// Scanner Skin Bridge
// -----------------------------------------------------------------------------
// gSkinC is a global pointer used by the scanner to select a keyword “skin.”
// Example: --skin=pirate switches keywords to their pirate equivalents.
// gSkinStorage must stay alive so gSkinC remains a valid C-style string.
// -----------------------------------------------------------------------------
extern "C" const char* gSkinC;
string gSkinStorage = "default";
const char* gSkinC = gSkinStorage.c_str();

// Forward declaration of entry point into the parser, provided by parser.cpp
unique_ptr<Program> parseProgram();

// -----------------------------------------------------------------------------
// Command-line flags
// -----------------------------------------------------------------------------
bool FLAG_TOKENS=false, FLAG_PRINT_AST=false, FLAG_SYMBOLS=false; // -t, -p, -s

// -----------------------------------------------------------------------------
// ANSI color codes for nicer output 
// -----------------------------------------------------------------------------
constexpr const char* C_RESET = "\033[0m";
constexpr const char* C_YBOLD = "\033[1;33m";
constexpr const char* C_MBOLD = "\033[1;35m";
constexpr const char* C_GREEN = "\033[32m";
constexpr const char* C_CYAN  = "\033[36m";

// Helper to print section banners
void banner(const char* title, const char* color)
{
    cout << "\n" << color << "===== " << title << " =====" << C_RESET << "\n\n";
}

// Usage message for correct CLI usage
void usage(const char* prog)
{
    banner("USAGE", C_CYAN);
    cout << "Usage: " << prog << " [options] [file]\n"
         << "Options:\n"
         << "  -p            Print AST after parse\n"
         << "  -t            Tokenize only (dump tokens) and exit\n"
         << "  -s            Print symbol table after interpretation\n"
         << "  -d            Enable debug traces to stderr\n"
         << "  --skin=NAME   Select keyword skin (default, INITIAL, pirate, cat)\n"
         << "  --help        Show this help\n\n"
         << "Example: " << prog << " --skin=pirate samples/hello.tips -p\n";
}

// -----------------------------------------------------------------------------
// Token dump routine for -t mode
// -----------------------------------------------------------------------------
// Repeatedly calls yylex() to get tokens, then prints them with line numbers
// and lexemes. Only IDENT and STRINGLIT show their lexeme to keep output compact.
// If UNKNOWN appears, we exit immediately with a nonzero code.
// -----------------------------------------------------------------------------
int dumpTokens()
{
    banner("BEGIN TOKENIZE", C_YBOLD);
    while (true)
    {
        int t = yylex();
        if (t == 0) t = TOK_EOF;
        cout << yylineno << " " << tokName(t);
        if (t == IDENT || t == STRINGLIT)
            cout << " " << (yytext ? yytext : "");
        cout << "\n";
        if (t == UNKNOWN)
        {
            cerr << "Lexical error near: '" << (yytext?yytext:"") << "'\n";
            return 2;
        }
        if (t == TOK_EOF) break;
    }
    banner("TOKENIZE COMPLETE", C_YBOLD);
    return 0;
}

// -----------------------------------------------------------------------------
// main()
// -----------------------------------------------------------------------------
// Parses command-line arguments, opens the input file (or stdin), and runs the
// requested phases. All phases are wrapped in try/catch for clean error output.
// -----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    const char* infile = nullptr;

    // Parse command-line args
    for (int i = 1; i < argc; ++i)
    {
        const char* a = argv[i];
        if (!strcmp(a, "-p")) FLAG_PRINT_AST = true;
        else if (!strcmp(a, "-t")) FLAG_TOKENS = true;
        else if (!strcmp(a, "-s")) FLAG_SYMBOLS = true;
        else if (!strcmp(a, "-d")) dbg::set(true);
        else if (!strncmp(a, "--skin=", 8))
        {
            gSkinStorage = string(a + 8);
            gSkinC = gSkinStorage.c_str();
        }
        else if (!strcmp(a, "--help")) { usage(argv[0]); return 0; }
        else if (a[0] == '-') { cerr << "Unknown option: " << a << "\n"; return 1; }
        else if (!infile) infile = a;
        else { cerr << "Only one input file is supported.\n"; return 1; }
    }

    // Open input file or use stdin
    FILE* in = stdin;
    if (infile){ in = fopen(infile, "r"); if (!in){ perror("open"); return 1; } }
    yyin = in; // give FILE* to the scanner
    extern int yylineno; yylineno = 1; // reset line number at start

    try
    {
        // Mode: tokenize only
        if (FLAG_TOKENS)
        { 
            int rc = dumpTokens(); 
            if (in && in!=stdin) fclose(in);
            return rc; 
        }

        // Parse
        if (FLAG_PRINT_AST) banner("BEGIN PARSING", C_MBOLD);
        unique_ptr<Program> root = parseProgram();
        // operator<<(ostream&, Program*) must be defined in ast.h
        if (FLAG_PRINT_AST) cout << root;
        if (FLAG_PRINT_AST) banner("PARSING COMPLETE", C_MBOLD);

        // Interpret
        banner("BEGIN INTERPRETATION", C_YBOLD);
        // WRITE statements should print to stdout by spec
        root->interpret(cout);
        banner("INTERPRETATION COMPLETE", C_YBOLD);

        // Display success
        banner("Program executed successfully", C_GREEN);
    }
    catch (const exception& e)
    {
        // Exceptions may come from parser (syntax errors) or interpreter (runtime errors)
        cerr << e.what() << "\n";
        if (in && in!=stdin) fclose(in);
        return 2;
    }

    if (in && in!=stdin) fclose(in);
    return 0;
}
