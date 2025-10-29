// =============================================================================
//   ast.h — AST for TIPS Subset (matches PDF diagrams)
// =============================================================================
// MSU CSE 4714/6714 Capstone Project (Fall 2025)
// Author: Kevin Ho
//
//   Part 1 : PROGRAM, BLOCK, WRITE
//   Part 2 : VAR/READ/ASSIGN + symtab + Compound Statement + BLOCK (fr this time)
//   Part 3 : expression/simple/term/factor + relations/logic/arithmetic
//   Part 4 : IF/WHILE, custom op/keyword, skins
// =============================================================================
#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <iostream>
using namespace std;

// -----------------------------------------------------------------------------
// Pretty printer
// -----------------------------------------------------------------------------
inline void ast_line(ostream& os, string prefix, bool last, string label) {
  os << prefix << (last ? "└── " : "├── ") << label << "\n";
}
inline string kid_prefix(const string& prefix, bool isLast) {
  return prefix + (isLast ? "    " : "│   ");
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

struct Value {
  enum class Kind { IntLit, FloatLit, Ident };   // renamed to avoid macros
  Kind kind;
  string lexeme; // digits or identifier name

  Value(Kind k, string s) : kind(k), lexeme(std::move(s)) {}

  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const {
    string tag = (kind == Kind::IntLit)   ? "INT " :
                 (kind == Kind::FloatLit) ? "REAL " : "IDENT ";
    ast_line(os, prefix, isLast, "Value(" + tag + lexeme + ")");
  }
};

struct Statement {
  virtual ~Statement() = default;
  virtual void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const = 0;
  virtual void interpret(ostream& out) const { (void)out; }  // Step 4 will implement behavior
};

struct AssignStmt : Statement {
  string id;
  unique_ptr<Value> rhs;  // Part 2 only supports Value on RHS

  AssignStmt(string id_, unique_ptr<Value> rhs_)
    : id(std::move(id_)), rhs(std::move(rhs_)) {}

  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    ast_line(os, prefix, isLast, "Assign " + id + " :=");
    if (rhs) rhs->print_tree(os, kid_prefix(prefix, isLast), true);
  }
};

struct ReadStmt : Statement {
  string id;
  explicit ReadStmt(string id_) : id(std::move(id_)) {}

  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    ast_line(os, prefix, isLast, "Read(" + id + ")");
  }
};

struct WriteStmt : Statement {
  enum class ArgKind { Str, Id };   // renamed to avoid STRING/IDENT collisions
  ArgKind kind;
  string text_or_id; // string literal (no quotes) or identifier

  WriteStmt(ArgKind k, string v) : kind(k), text_or_id(std::move(v)) {}

  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    string payload = (kind == ArgKind::Str) ? ("'" + text_or_id + "'") : text_or_id;
    ast_line(os, prefix, isLast, "Write(" + payload + ")");
  }
};

struct CompoundStmt : Statement {
  vector<unique_ptr<Statement>> stmts;  // sequence inside BEGIN ... END

  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    ast_line(os, prefix, isLast, "BEGIN");
    string kid = kid_prefix(prefix, isLast);
    if (stmts.empty()) {
      ast_line(os, kid, true, "(empty)");
    } else {
      for (size_t i = 0; i < stmts.size(); ++i) {
        bool lastChild = (i + 1 == stmts.size());
        stmts[i]->print_tree(os, kid, lastChild);
      }
    }
    ast_line(os, prefix, true, "END");
  }
};


// Declarations (VAR section)
struct Decl {
  enum class Type { Int, Real };   // renamed to avoid INTEGER/REAL macros
  string name;
  Type type;
};


struct Block {
  vector<Decl> decls;                    // optional VAR declarations
  unique_ptr<CompoundStmt> body;         // BEGIN ... END

  unique_ptr<Write> write;

  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const {
    ast_line(os, prefix, isLast, "Block");
    string kid = kid_prefix(prefix, isLast);

    if (!decls.empty()) {
      // "VAR" line
      ast_line(os, kid, (!body && !write), "VAR");
      string varkid = kid_prefix(kid, (!body && !write));
      for (size_t i = 0; i < decls.size(); ++i) {
        const auto& d = decls[i];
        string typ = (d.type == Decl::Type::Int) ? "INTEGER" : "REAL";
        bool lastDecl = (i + 1 == decls.size()) && !body && !write;
        ast_line(os, varkid, lastDecl, d.name + " : " + typ + ";");
      }
    }

    if (body) {
      body->print_tree(os, kid, write == nullptr);
    } else if (write) {
      write->print_tree(os, kid, true);
    } else if (decls.empty()) {
      ast_line(os, kid, true, "(empty)");
    }
  }

  void interpret(ostream& out) const {
    if (body) {
      for (auto& s : body->stmts) s->interpret(out);
    } else if (write) {
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


