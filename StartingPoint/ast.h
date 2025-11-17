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
#include <iomanip>
#include <cmath>
#include <cstdint>
using namespace std;

using IntType = int32_t;
using RealType = double;
using ValueVariant = variant<IntType, RealType>;

// -----------------------------------------------------------------------------
// Global Variable
// -----------------------------------------------------------------------------
extern map<string, ValueVariant> symbolTable;

inline void printValue(ostream& out, const ValueVariant& val) {
  if (holds_alternative<IntType>(val)) {
    out << get<IntType>(val);
  } else {
    auto flags = out.flags();
    auto precision = out.precision();
    out << fixed << setprecision(4) << get<RealType>(val);
    out.flags(flags);
    out.precision(precision);
  }
}

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
    if (holds_alternative<IntType>(it->second)) {
      IntType v; if (!(cin >> v)) throw runtime_error("Input error: expected INTEGER for " + id);
      it->second = v;
    } 
    else {
      RealType v; if (!(cin >> v)) throw runtime_error("Input error: expected REAL for " + id);
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
    printValue(out, it->second);
    out << '\n';
  }
};

// -----------------------------------------------------------------------------
// Part 3: Expressions
// -----------------------------------------------------------------------------
struct Expr {
  virtual ~Expr() = default;
  virtual void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const = 0;
  virtual ValueVariant eval() const = 0;
};

struct IntLiteral : Expr {
  IntType value;
  explicit IntLiteral(IntType v) : value(v) {}
  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    ast_line(os, prefix, isLast, "INT " + to_string(value));
  }
  ValueVariant eval() const override { return value; }
};

struct RealLiteral : Expr {
  RealType value;
  explicit RealLiteral(RealType v) : value(v) {}
  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    ast_line(os, prefix, isLast, "REAL " + to_string(value));
  }
  ValueVariant eval() const override { return value; }
};

struct IdentExpr : Expr {
  string name;
  explicit IdentExpr(string n) : name(std::move(n)) {}
  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    ast_line(os, prefix, isLast, "IDENT " + name);
  }
  ValueVariant eval() const override {
    auto it = symbolTable.find(name);
    if (it == symbolTable.end()) throw runtime_error("Runtime error: undeclared identifier " + name);
    return it->second;
  }
};

struct UnaryExpr : Expr {
  enum class Op { Plus, Minus };
  Op op; unique_ptr<Expr> child;
  UnaryExpr(Op o, unique_ptr<Expr> e) : op(o), child(std::move(e)) {}
  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    string o = (op == Op::Plus) ? "+" : "-";
    ast_line(os, prefix, isLast, string("Unary(") + o + ")");
    child->print_tree(os, kid_prefix(prefix, isLast), true);
  }
  ValueVariant eval() const override {
    auto v = child->eval();
    if (holds_alternative<IntType>(v)) {
      IntType i = get<IntType>(v);
      return (op == Op::Plus) ? i : -i;
    } 
    else {
      RealType d = get<RealType>(v);
      return (op == Op::Plus) ? d : -d;
    }
  }
};

struct PreIncDecExpr : Expr {
  bool isInc; string name;
  PreIncDecExpr(bool inc, string n) : isInc(inc), name(std::move(n)) {}
  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    ast_line(os, prefix, isLast, string(isInc?"PreInc":"PreDec") + "(" + name + ")");
  }
  ValueVariant eval() const override {
    auto it = symbolTable.find(name);
    if (it == symbolTable.end()) throw runtime_error("Runtime error: undeclared identifier " + name);
    if (holds_alternative<IntType>(it->second)) {
      IntType v = get<IntType>(it->second);
      v += isInc ? IntType{1} : IntType{-1};
      it->second = v;
      return v;
    } 
    else {
      RealType v = get<RealType>(it->second);
      v += isInc ? 1.0 : -1.0;
      it->second = v;
      return v;
    }
  }
};

struct BinaryExpr : Expr {
  enum class Op { Add, Sub, Mul, Div, Mod, Pow };
  Op op; unique_ptr<Expr> lhs, rhs;
  BinaryExpr(Op o, unique_ptr<Expr> L, unique_ptr<Expr> R)
    : op(o), lhs(std::move(L)), rhs(std::move(R)) {}
  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    string name;
    switch (op) {
      case Op::Add: name = "+"; break;
      case Op::Sub: name = "-"; break;
      case Op::Mul: name = "*"; break;
      case Op::Div: name = "/"; break;
      case Op::Mod: name = "MOD"; break;
      case Op::Pow: name = "^^"; break;
    }
    ast_line(os, prefix, isLast, string("Bin(") + name + ")");
    auto kp = kid_prefix(prefix, isLast);
    lhs->print_tree(os, kp, false);
    rhs->print_tree(os, kp, true);
  }
  static inline bool anyReal(const ValueVariant& a, const ValueVariant& b){
    return holds_alternative<RealType>(a) || holds_alternative<RealType>(b);
  }
  static inline IntType mulWrap(IntType a, IntType b) {
    int64_t prod = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<IntType>(prod);
  }
  static IntType powInt(IntType base, IntType exp) {
    if (exp == 0) return IntType{1};
    IntType result = 1;
    IntType factor = base;
    uint32_t e = static_cast<uint32_t>(exp);
    while (e > 0) {
      if (e & 1u) {
        result = mulWrap(result, factor);
      }
      e >>= 1u;
      if (e) {
        factor = mulWrap(factor, factor);
      }
    }
    return result;
  }
  ValueVariant eval() const override {
    auto A = lhs->eval();
    auto B = rhs->eval();
    if (op == Op::Mod) {
      if (!holds_alternative<IntType>(A) || !holds_alternative<IntType>(B))
        throw runtime_error("Runtime error: MOD requires INTEGER operands");
      IntType ai = get<IntType>(A);
      IntType bi = get<IntType>(B);
      if (bi == 0) throw runtime_error("Runtime error: division by zero in MOD");
      IntType r = ai % bi;
      if (r < 0) {
        IntType divisor = (bi > 0) ? bi : -bi;
        r += divisor;
      }
      return r;
    }
    if (op == Op::Pow) {
      bool lhsReal = holds_alternative<RealType>(A);
      bool rhsReal = holds_alternative<RealType>(B);
      if (!lhsReal && !rhsReal) {
        IntType base = get<IntType>(A);
        IntType exponent = get<IntType>(B);
        if (exponent < 0) {
          RealType da = static_cast<RealType>(base);
          RealType db = static_cast<RealType>(exponent);
          return pow(da, db);
        }
        return powInt(base, exponent);
      }
      RealType da = lhsReal ? get<RealType>(A) : static_cast<RealType>(get<IntType>(A));
      RealType db = rhsReal ? get<RealType>(B) : static_cast<RealType>(get<IntType>(B));
      RealType pd = pow(da, db);
      return pd;
    }
    if (op == Op::Div) {
      RealType a = holds_alternative<IntType>(A) ? static_cast<RealType>(get<IntType>(A)) : get<RealType>(A);
      RealType b = holds_alternative<IntType>(B) ? static_cast<RealType>(get<IntType>(B)) : get<RealType>(B);
      if (b == 0.0) throw runtime_error("Runtime error: division by zero");
      return a / b;  // division always yields REAL
    }
    // + - * /
    if (anyReal(A,B)) {
      RealType a = holds_alternative<IntType>(A) ? static_cast<RealType>(get<IntType>(A)) : get<RealType>(A);
      RealType b = holds_alternative<IntType>(B) ? static_cast<RealType>(get<IntType>(B)) : get<RealType>(B);
      switch (op) {
        case Op::Add: return a + b;
        case Op::Sub: return a - b;
        case Op::Mul: return a * b;
        default: break;
      }
    } else {
      IntType a = get<IntType>(A);
      IntType b = get<IntType>(B);
      switch (op) {
        case Op::Add: return a + b;
        case Op::Sub: return a - b;
        case Op::Mul: return a * b;
        default: break;
      }
    }
    throw runtime_error("Runtime error: unknown binary op");
  }
};

struct AssignStmt : Statement {
  string id;
  unique_ptr<Expr> rhs;

  AssignStmt(string id_, unique_ptr<Expr> rhs_)
    : id(std::move(id_)), rhs(std::move(rhs_)) {}

  void print_tree(ostream& os, const string& prefix = "", bool isLast = true) const override {
    ast_line(os, prefix, isLast, "Assign " + id + " :=");
    if (rhs) rhs->print_tree(os, kid_prefix(prefix, isLast), true);
  }

  void interpret(ostream& out) const override {
    auto itL = symbolTable.find(id);
    if (itL == symbolTable.end())
      throw runtime_error("Runtime error: ASSIGN to undeclared identifier " + id);

    ValueVariant rv = rhs->eval();
    if (holds_alternative<IntType>(itL->second)) {
      // store as integer (truncate if real)
      IntType v = holds_alternative<IntType>(rv) ? get<IntType>(rv)
                   : static_cast<IntType>(get<RealType>(rv));
      itL->second = v;
    } 
    else {
      // store as real (widen int)
      RealType v = holds_alternative<IntType>(rv)
                     ? static_cast<RealType>(get<IntType>(rv))
                     : get<RealType>(rv);
      itL->second = v;
    }
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
