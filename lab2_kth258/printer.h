#ifndef PRINTER_H
#define PRINTER_H

// ============================================================================
// printer.h - Visitor that prints a FULLY EXPANDED parse tree
// ----------------------------------------------------------------------------
// Author: Derek Willis (Fall 2025)
// Students: declarations only; you usually do NOT need to edit this file.
// ----------------------------------------------------------------------------
//
// WHAT THIS FILE IS
//   Declarations for a "Printer" visitor. A visitor walks the AST and performs
//   an operation on each concrete node. Here, we print one line per grammar
//   symbol, with Unicode branches.
//
// VISITOR (DOUBLE DISPATCH) IN PLAIN WORDS
//   - Each AST node type defines:  void accept(Printer&)
//   - The Printer defines:         void visit(ConcreteNodeType&)
//   - At runtime:  node.accept(printer)  calls  printer.visit(actual_node_type)
//
// REQUIRED TREE SHAPE (Unicode)
//   <sentence>
//   ├─ <noun phrase>
//   │  ├─ <adjective phrase>
//   │  │  ├─ ARTICLE    (the)     or   POSSESSIVE (my)
//   │  │  └─ ADJECTIVE  (green)
//   │  └─ NOUN          (dog)
//   ├─ <verb phrase>
//   │  ├─ ADVERB        (quickly)               [zero or more]
//   │  └─ VERB          (eats)
//   └─ <noun phrase>
//      ├─ <adjective phrase>
//      │  ├─ ARTICLE/POSSESSIVE (...)
//      │  └─ ADJECTIVE (...)
//      └─ NOUN (...)
//
// INDENTATION MODEL (implemented in printer.cpp)
//   - Keep one boolean per depth level in vector<bool> branchStack.
//     * branchStack.back() is about the PARENT level of whatever you print next.
//     * true  -> more siblings after this child: draw the vertical pipe "│  " under it
//     * false -> this child is the last at that depth: no pipe below
//   - For the current line: use "├─ " for not-last, "└─ " for last.
//
// NOTE: Requires a UTF-8 capable terminal/font. If your terminal shows boxes,
//       swap "│  ", "├─ ", "└─ " for ASCII " | ", "|- ", "`- " in your own code.
// ============================================================================

#include <iosfwd>
#include <string>
#include <vector>
using namespace std;

struct AdjectivePhrase;
struct NounPhrase;
struct VerbPhrase;
struct Sentence;

struct Printer {
    explicit Printer(ostream& out);

    void visit(AdjectivePhrase& node);
    void visit(NounPhrase&      node);
    void visit(VerbPhrase&      node);
    void visit(Sentence&        node);

private:
    ostream&     os;
    vector<bool> branchStack;

    void printLine(const string& tag, const string& lexeme, bool isLast);
    void beginChildren();
    void endChildren();
    void printNonterminal(const string& tag);

    void printARTICLE   (const string& word, bool isLast);
    void printPOSSESSIVE(const string& word, bool isLast);
    void printADJECTIVE (const string& word, bool isLast);
    void printNOUN      (const string& word, bool isLast);
    void printADVERB    (const string& word, bool isLast);
    void printVERB      (const string& word, bool isLast);
};

#endif // PRINTER_H
