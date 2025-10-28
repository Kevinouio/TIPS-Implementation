// =============================================================================
//   ast.h — AST for TIPS Subset (matches PDF diagrams)
// =============================================================================
// MSU CSE 4714/6714 Capstone Project (Fall 2025)
// Author: Derek Willis
//
//   Part 1 : PROGRAM, BLOCK, WRITE
//   Part 2 : VAR/READ/ASSIGN + symtab + Compound Statement + BLOCK (fr this time)
//   Part 3 : expression/simple/term/factor + relations/logic/arithmetic
//   Part 4 : IF/WHILE, custom op/keyword, skins
// =============================================================================
#pragma once
#include <memory>
#include <string>
#include <iostream>
using namespace std;

// -----------------------------------------------------------------------------
// Pretty printer
// -----------------------------------------------------------------------------
inline void ast_line(ostream& os, string prefix, bool last, string label) {
  os << prefix << (last ? "└── " : "├── ") << label << "\n";
}


// Write and Block Structs

struct Write {
    string text;  

    void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const {
        ast_line(os, prefix, isLast, "Write( '" + text + "' )");
    }

    void interpret(ostream& out) const {
        out << "'" << text << "'" << '\n';
    }
};

struct Block {
    unique_ptr<Write> write;  

    void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const {
        ast_line(os, prefix, isLast, "Block");
        string kid = prefix + (isLast ? "    " : "│   ");
        if (write) {
          write->print_tree(os, kid, true);
        }
        else{
          ast_line(os, kid, true, "(empty)");
        }
    }

    void interpret(ostream& out) const {
        if (write) {
          write->interpret(out);
        }
    }
};


struct Program 
{
  string name; 
  unique_ptr<Block> block;
  void print_tree(ostream& os)
  {
    cout << "Program\n";
    ast_line(os, "", false, "name: " + name);
    if (block) block->print_tree(os, "", true);
    else 
    { 
      ast_line(os, "", true, "Block"); 
      ast_line(os, "    ", true, "(empty)");
    }
  }
  void interpret(ostream& out) { if (block) block->interpret(out); }
};

// Overload << for Program
inline ostream& operator<<(ostream& os, Program& p) {
    p.print_tree(os);
    return os;
}

inline ostream& operator<<(ostream& os, Program* p) {
    if (p) p->print_tree(os);
    return os;
}

inline ostream& operator<<(ostream& os, unique_ptr<Program>& p) {
    if (p) p->print_tree(os);
    return os;
}


