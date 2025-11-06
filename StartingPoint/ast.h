// =============================================================================
//   ast.h â€” AST for TIPS Subset (matches PDF diagrams)
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
#include <variant>
#include <stdexcept>
#include <iostream>
using namespace std;


// -----------------------------------------------------------------------------
// Global Variable
// -----------------------------------------------------------------------------
extern map<string, variant<int,double>> symbolTable;

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
struct ReadStmt : Statement {
  string id;
  explicit ReadStmt(string id_) : id(std::move(id_)) {}

  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    ast_line(os, prefix, isLast, "Read(" + id + ")");
  }

  void interpret(ostream& out) const override {
    auto it = symbolTable.find(id);
    if (it == symbolTable.end())
      throw runtime_error("Runtime error: READ of undeclared identifier " + id);
    if (holds_alternative<int>(it->second)) {
      int v; if (!(cin >> v)) throw runtime_error("Input error: expected INTEGER for " + id);
      it->second = v;
    } else {
      double v; if (!(cin >> v)) throw runtime_error("Input error: expected REAL for " + id);
      it->second = v;
    }
  }
};

struct WriteStmt : Statement {
  enum class ArgKind { Str, Id };
  ArgKind kind;
  string text_or_id;

  WriteStmt(ArgKind k, string v) : kind(k), text_or_id(std::move(v)) {}

  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    string payload = (kind == ArgKind::Str) ? ("'" + text_or_id + "'") : text_or_id;
    ast_line(os, prefix, isLast, "Write(" + payload + ")");
  }

  void interpret(ostream& out) const override {
    if (kind == ArgKind::Str) {
      out << "'" << text_or_id << "'" << '\n';
      return;
    }
    auto it = symbolTable.find(text_or_id);
    if (it == symbolTable.end())
      throw runtime_error("Runtime error: WRITE of undeclared identifier " + text_or_id);
    visit([&](auto&& v){ out << v << '\n'; }, it->second);
  }
};

struct AssignStmt : Statement {
  string id;
  unique_ptr<Value> rhs;

  AssignStmt(string id_, unique_ptr<Value> rhs_)
    : id(std::move(id_)), rhs(std::move(rhs_)) {}

  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    ast_line(os, prefix, isLast, "Assign " + id + " :=");
    if (rhs) rhs->print_tree(os, kid_prefix(prefix, isLast), true);
  }

  void interpret(ostream& out) const override {
    auto itL = symbolTable.find(id);
    if (itL == symbolTable.end())
      throw runtime_error("Runtime error: ASSIGN to undeclared identifier " + id);

    auto fetchIdent = [](const string& name) -> double {
      auto it = symbolTable.find(name);
      if (it == symbolTable.end())
        throw runtime_error("Runtime error: undeclared identifier on RHS: " + name);
      return holds_alternative<int>(it->second) ? static_cast<double>(get<int>(it->second))
                                                : get<double>(it->second);
    };

    double r = 0.0;
    switch (rhs->kind) {
      case Value::Kind::IntLit:   r = static_cast<double>(stoi(rhs->lexeme)); break;
      case Value::Kind::FloatLit: r = stod(rhs->lexeme); break;
      case Value::Kind::Ident:    r = fetchIdent(rhs->lexeme); break;
    }

    if (holds_alternative<int>(itL->second)) itL->second = static_cast<int>(r); // truncate
    else                                     itL->second = r;                   // widen or keep real
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
  void interpret(ostream& out) const override {
    for (auto& s : stmts) {
      s->interpret(out);
    }
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


  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const {
    ast_line(os, prefix, isLast, "Block");
    string kid = kid_prefix(prefix, isLast);

    if (!decls.empty()) {
      // "VAR" line
      ast_line(os, kid, !body, "VAR");  
      string varkid = kid_prefix(kid, !body);  
      for (size_t i = 0; i < decls.size(); ++i) {
        const auto& d = decls[i];
        string typ = (d.type == Decl::Type::Int) ? "INTEGER" : "REAL";
        bool lastDecl = (i + 1 == decls.size()) && !body;  
        ast_line(os, varkid, lastDecl, d.name + " : " + typ + ";");
      }
    }

    if (body) {
      body->print_tree(os, kid, true);  
    } else if (decls.empty()) {
      ast_line(os, kid, true, "(empty)");
    }
  }

  void interpret(ostream& out) const {
    if (body) {
      for (auto& s : body->stmts) s->interpret(out);
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


