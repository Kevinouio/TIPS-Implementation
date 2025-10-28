#ifndef AST_H
#define AST_H
// ============================================================================
// ast.h - Abstract Syntax Tree (AST) shapes for the combined Parsing + OO lab
// ----------------------------------------------------------------------------
// Author: Derek Willis (Fall 2025)
// Students: read this first; you usually do NOT need to edit this file.
// ----------------------------------------------------------------------------
//
// READ THIS FIRST
// - This header defines the "shapes" of the parse tree your parser will build.
// - There is one abstract base type (Node) and four concrete node types that
//   map directly to the grammar nonterminals.
//
// Grammar (from the assignment):
//   <sentence>         -> <noun phrase> <verb phrase> <noun phrase>
//   <noun phrase>      -> <adjective phrase> NOUN
//   <adjective phrase> -> (ARTICLE | POSSESSIVE) ADJECTIVE
//   <verb phrase>      -> VERB | ADVERB <verb phrase>
// We will store verb phrases as: zero or more ADVERB, then exactly one VERB.
//
// What you will practice:
// - Inheritance and virtual functions (abstract base + overrides)
// - Polymorphism and double dispatch (accept on nodes, visit on the printer)
// - Smart pointers for ownership (unique_ptr for children)
// - STL containers (vector<string> for ADVERB*)
//
// FAQ
// Q: What does "forward declare" mean and why do we do it?
// A: A forward declaration says "this type exists" without including its full
//    definition. We forward declare the Printer so this header does not need
//    to include printer.h (which itself includes this header). That prevents
//    circular includes.
//
// Q: Why use struct instead of class?
// A: In C++, struct and class are the same except for the default access level
//    (struct is public by default, class is private). These AST nodes are simple
//    data holders. Using struct keeps the code shorter and clearer for this lab.
//    If you wanted stronger encapsulation, class would also be fine.
//
// Q: Why a virtual destructor in the base Node?
// A: We will delete derived objects through a Node* at times. A virtual
//    destructor ensures the entire derived object is destroyed correctly.
//    The "= default" means "use the compiler's correct trivial destructor."
//
// Q: Why no constructors here?
// A: The parser will create nodes with make_unique<...>() and then fill the
//    public data fields. Default-generated constructors are fine for that.
//    You could add convenience constructors later, but they are not required.
//
// Q: Why "enum class" below instead of plain "enum"?
// A: enum class is a scoped enum (names do not leak into the global scope),
//    which avoids collisions. Refer to values as AdjectivePhrase::DetType::Article.
//
// Implementation notes:
// - Students will implement Node::accept(...) overrides in ast.cpp. Each one
//   is a tiny one-liner that calls p.visit(*this).
// - Ownership: children are held by unique_ptr. Copying nodes is implicitly
//   disabled; moving is allowed. Destroying the root Sentence automatically
//   frees the entire tree.
// - Course choice: we use "using namespace std;" here to keep code minimal for
//   the lab. In larger projects, avoid "using" in headers.
// ============================================================================

#include <memory>   // unique_ptr
#include <string>   // string
#include <vector>   // vector

using namespace std; // course choice for simplicity in this lab

// Forward declaration: we only need to name Printer here.
// The full Printer type is defined in printer.h / printer.cpp.
struct Printer;

// ============================================================================
// Base node (abstract)
// ----------------------------------------------------------------------------
// - Has a virtual destructor for safe polymorphic deletion.
// - Declares a pure virtual accept that each derived node must override.
// ============================================================================
struct Node {
    // Virtual destructor - needed when deleting via base pointer
    virtual ~Node() = default;

    // Double-dispatch entry point:
    // Implement in each derived node in ast.cpp as: p.visit(*this);
    virtual void accept(Printer& p) = 0;
};

// ============================================================================
// <adjective phrase>
// ----------------------------------------------------------------------------
// Grammar: <adjective phrase> -> (ARTICLE | POSSESSIVE) ADJECTIVE
// Data:
//   - detLexeme: the determiner text (for example "the" or "my")
//   - adjLexeme: the adjective text (for example "green")
// Extra:
//   - detType: whether the determiner token was an ARTICLE or a POSSESSIVE,
//     so the printer can label the leaf line correctly.
// ============================================================================
struct AdjectivePhrase : Node {
    // Scoped enum (names do not leak): refer to values as
    // AdjectivePhrase::DetType::Article or ::Possessive
    enum class DetType { Article, Possessive };
    DetType detType = DetType::Article;

    string detLexeme;  // the determiner word: ARTICLE or POSSESSIVE
    string adjLexeme;  // the adjective word

    // Students implement the body in ast.cpp: p.visit(*this);
    void accept(Printer& p) override;
};

// ============================================================================
// <noun phrase>
// ----------------------------------------------------------------------------
/*
Grammar: <noun phrase> -> <adjective phrase> NOUN

Data and ownership:
- adj:        child node (owned by the noun phrase)
- nounLexeme: the noun word
Note: unique_ptr makes this type move-only (copying disabled implicitly).
*/
struct NounPhrase : Node {
    unique_ptr<AdjectivePhrase> adj; // owned child
    string nounLexeme;               // the NOUN word

    void accept(Printer& p) override; // implement in ast.cpp
};

// ============================================================================
// <verb phrase>
// ----------------------------------------------------------------------------
/*
Grammar: <verb phrase> -> VERB | ADVERB <verb phrase>

Modeling choice:
- Normalize it to ADVERB* then VERB.
- Example: "really quickly eats"
  adverbs = ["really", "quickly"]
  verbLexeme = "eats"
*/
struct VerbPhrase : Node {
    vector<string> adverbs; // zero or more ADVERB tokens, in input order
    string verbLexeme;      // the final VERB token

    void accept(Printer& p) override; // implement in ast.cpp
};

// ============================================================================
// <sentence> (top-level)
// ----------------------------------------------------------------------------
/*
Grammar: <sentence> -> <noun phrase> <verb phrase> <noun phrase>

Data and ownership:
- subjectNP: left noun phrase
- verbP:     verb phrase
- objectNP:  right noun phrase
Destroying a Sentence frees its children automatically (unique_ptr).
*/
struct Sentence : Node {
    unique_ptr<NounPhrase> subjectNP;
    unique_ptr<VerbPhrase> verbP;
    unique_ptr<NounPhrase> objectNP;

    void accept(Printer& p) override; // implement in ast.cpp
};

#endif // AST_H
