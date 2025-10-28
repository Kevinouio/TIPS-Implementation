// ============================================================================
// printer.cpp — Visitor that prints a FULLY EXPANDED parse tree (completed)
// ----------------------------------------------------------------------------
// Author: Derek Willis (Fall 2025)
// ============================================================================

#include "printer.h"
#include "ast.h"
#include <string>
#include <ostream>
using namespace std;

// Constructor: store the output stream; indentation state starts empty.
Printer::Printer(ostream& out)
    : os(out), branchStack() {}

// Helper: is the CURRENT header line last among its siblings?
// Parent sets branchStack.back(): true  = more siblings to come
//                                 false = this child is last
static bool currentIsLast(const vector<bool>& stk) {
    if (stk.empty()) return true;   // root (no parent)
    return !stk.back();             // parent said "no more siblings" -> last
}

// Draw ONE line at the current depth, using Unicode branches:
//   ancestor: "│  " (has more siblings) or "   " (last)
//   current : "├─ " (not last) or "└─ " (last)
void Printer::printLine(const string& tag, const string& lexeme, bool isLast) {
    // Indentation for all ancestors except the current level.
    if (!branchStack.empty()) {
        for (size_t i = 0; i + 1 < branchStack.size(); ++i) {
            os << (branchStack[i] ? "│  " : "   ");
        }
        // Branch for the current level.
        os << (isLast ? "└─ " : "├─ ");
    }
    // Text: tag plus optional lexeme.
    if (lexeme.empty()) {
        os << tag << '\n';
    } else {
        os << tag << "  (" << lexeme << ")\n";
    }
}

// We are about to print this node's children at the next depth.
void Printer::beginChildren() {
    // Push placeholder for "child depth". The parent visit(...) sets this
    // before each child to indicate whether more siblings follow at that depth.
    branchStack.push_back(false);
}

// We are done printing this node's children.
void Printer::endChildren() {
    if (!branchStack.empty()) branchStack.pop_back();
}

// Print a nonterminal header (e.g., "<noun phrase>") at the current depth.
void Printer::printNonterminal(const string& tag) {
    const bool isLast = currentIsLast(branchStack);
    printLine(tag, "", isLast);
}

// Terminal leaf helpers (one-liners).
void Printer::printARTICLE   (const string& word, bool isLast) { printLine("ARTICLE",    word, isLast); }
void Printer::printPOSSESSIVE(const string& word, bool isLast) { printLine("POSSESSIVE", word, isLast); }
void Printer::printADJECTIVE (const string& word, bool isLast) { printLine("ADJECTIVE",  word, isLast); }
void Printer::printNOUN      (const string& word, bool isLast) { printLine("NOUN",       word, isLast); }
void Printer::printADVERB    (const string& word, bool isLast) { printLine("ADVERB",     word, isLast); }
void Printer::printVERB      (const string& word, bool isLast) { printLine("VERB",       word, isLast); }

// <sentence> -> <noun phrase> <verb phrase> <noun phrase>
// Root header has no branch prefix; children are managed with branchStack.
void Printer::visit(Sentence& node) {
    os << "<sentence>\n";
    beginChildren();

    // Three children in order: subject, verb phrase, object.
    branchStack.back() = true;   node.subjectNP->accept(*this); // more after this
    branchStack.back() = true;   node.verbP->accept(*this);     // more after this
    branchStack.back() = false;  node.objectNP->accept(*this);  // last at this depth

    endChildren();
}

// <noun phrase> -> <adjective phrase> NOUN
void Printer::visit(NounPhrase& node) {
    printNonterminal("<noun phrase>");
    beginChildren();

    branchStack.back() = true;   node.adj->accept(*this);                  // NOT last
    branchStack.back() = false;  printNOUN(node.nounLexeme, /*isLast=*/true); // LAST

    endChildren();
}

// <adjective phrase> -> (ARTICLE | POSSESSIVE) ADJECTIVE
void Printer::visit(AdjectivePhrase& node) {
    printNonterminal("<adjective phrase>");
    beginChildren();

    branchStack.back() = true;
    if (node.detType == AdjectivePhrase::DetType::Article)
        printARTICLE(node.detLexeme, /*isLast=*/false);
    else
        printPOSSESSIVE(node.detLexeme, /*isLast=*/false);

    branchStack.back() = false;  printADJECTIVE(node.adjLexeme, /*isLast=*/true);

    endChildren();
}

// <verb phrase> -> VERB | ADVERB <verb phrase>
// We normalize/print as ADVERB* then VERB.
void Printer::visit(VerbPhrase& node) {
    printNonterminal("<verb phrase>");
    beginChildren();

    // All adverbs come before the final verb; while printing adverbs,
    // the verb is still to come, so each adverb is "not last".
    for (size_t i = 0; i < node.adverbs.size(); ++i) {
        branchStack.back() = true;   // more children will follow (another adverb or the verb)
        printADVERB(node.adverbs[i], /*isLast=*/false);
    }

    // Verb is always the last child in this normalized form.
    branchStack.back() = false;
    printVERB(node.verbLexeme, /*isLast=*/true);

    endChildren();
}
