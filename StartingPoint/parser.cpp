// ============================================================================
//  parser.cpp — Recursive-descent parser 
// ----------------------------------------------------------------------------
// MSU CSE 4714/6714 Capstone Project (Fall 2025)
// Author: Derek Willis
// ============================================================================

#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>
#include <set>
#include <map>
#include <variant>
#include <cmath>
#include "lexer.h"
#include "ast.h"
#include "debug.h"
using namespace std;

// -----------------------------------------------------------------------------
// Global Variable
// -----------------------------------------------------------------------------
map<string, variant<int,double>> symbolTable; 

// -----------------------------------------------------------------------------
// One-token lookahead
// -----------------------------------------------------------------------------
bool   havePeek = false;
Token  peekTok  = 0;
string peekLex;

inline const char* tname(Token t) { return tokName(t); }

Token peek() 
{
  if (!havePeek) {
    peekTok = yylex();
    if (peekTok == 0) { peekTok = TOK_EOF; peekLex.clear(); }
    else              { peekLex = yytext ? string(yytext) : string(); }
    dbg::line(string("peek: ") + tname(peekTok) + (peekLex.empty() ? "" : " ["+peekLex+"]")
              + " @ line " + to_string(yylineno));
    havePeek = true;
  }
  return peekTok;
}
Token nextTok() 
{
  Token t = peek();
  dbg::line(string("consume: ") + tname(t));
  havePeek = false;
  return t;
}
Token expect(Token want, const char* msg) 
{
  Token got = nextTok();
  if (got != want) {
    dbg::line(string("expect FAIL: wanted ") + tname(want) + ", got " + tname(got));
    ostringstream oss;
    oss << "Parse error (line " << yylineno << "): expected "
        << tname(want) << " — " << msg << ", got " << tname(got)
        << " [" << (yytext ? yytext : "") << "]";
    throw runtime_error(oss.str());
  }
  return got;
}

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
static inline bool accept(Token t) { if (peek() == t) { nextTok(); return true; } return false; }
static unique_ptr<Statement> parseStatement();
static unique_ptr<Statement> parseCompound();

// TODO: implement parsing functions for each grammar in your language

static Decl::Type parseType() {
  if (peek() == INTEGER) { nextTok(); return Decl::Type::Int; }
  if (peek() == REAL)    { nextTok(); return Decl::Type::Real; }
  throw runtime_error("Parse error: expected type (INTEGER or REAL)");
}

static void parseDeclarations(vector<Decl>& outDecls) {
  if (peek() != VAR) return;
  nextTok(); // consume VAR

  while (peek() == IDENT) {
    Decl d;
    d.name = peekLex;
    expect(IDENT, "declaration name");
    expect(COLON, "':' after identifier in declaration");
    d.type = parseType();
    if (symbolTable.count(d.name)) {
      throw runtime_error("Parse error: duplicate declaration of " + d.name);
    }

    if (d.type == Decl::Type::Int)  { 
      symbolTable[d.name] = 0;
    }
    else {
      symbolTable[d.name] = 0.0;
    }

    expect(SEMICOLON, "';' after declaration");
    outDecls.push_back(std::move(d));
  }
}

// ---------- Expressions (Part 3) ----------
static unique_ptr<Expr> parsePrimary() {
  if (peek() == OPENPAREN) {
    nextTok();
    auto e = parseExpression();
    expect(CLOSEPAREN, "expected ')' to close expression");
    return e;
  }
  if (peek() == INTLIT) {

    int v = stoi(peekLex); nextTok();
    return unique_ptr<Expr>(static_cast<Expr*>(new IntLiteral(v)));
  }
  if (peek() == FLOATLIT) {
    double v = stod(peekLex); nextTok();
    return unique_ptr<Expr>(static_cast<Expr*>(new RealLiteral(v)));
  }
  if (peek() == IDENT) {
    string name = peekLex;
    if (!symbolTable.count(name))
      throw runtime_error("Parse error: use of undeclared identifier " + name);
    nextTok();
    return unique_ptr<Expr>(static_cast<Expr*>(new IdentExpr(name)));
  }
  throw runtime_error(string("Parse error: expected primary, got ") + tname(peek()));
}

static unique_ptr<Expr> parseUnary() {
  if (peek() == PLUS)  { nextTok(); return make_unique<UnaryExpr>(UnaryExpr::Op::Plus,  parseUnary()); }
  if (peek() == MINUS) { nextTok(); return make_unique<UnaryExpr>(UnaryExpr::Op::Minus, parseUnary()); }
  if (peek() == INCREMENT) {
    nextTok();
    if (peek() != IDENT) throw runtime_error("Parse error: ++ must be followed by IDENT");
    string name = peekLex;
    if (!symbolTable.count(name)) throw runtime_error("Parse error: ++ of undeclared identifier " + name);
    nextTok();
    return unique_ptr<Expr>(static_cast<Expr*>(new PreIncDecExpr(true, name)));
  }
  if (peek() == DECREMENT) {
    nextTok();
    if (peek() != IDENT) throw runtime_error("Parse error: -- must be followed by IDENT");
    string name = peekLex;
    if (!symbolTable.count(name)) throw runtime_error("Parse error: -- of undeclared identifier " + name);
    nextTok();
    return unique_ptr<Expr>(static_cast<Expr*>(new PreIncDecExpr(false, name)));
  }
  return parsePrimary();
}

// Right-associative power: unary (^^ power)?
static unique_ptr<Expr> parsePower() {
  auto lhs = parseUnary();
  if (peek() == CUSTOM_OPER) {
    nextTok();
    auto rhs = parsePower(); // recurse for right-assoc
    return make_unique<BinaryExpr>(BinaryExpr::Op::Pow, std::move(lhs), std::move(rhs));
  }
  return lhs;
}

static unique_ptr<Expr> parseTerm() {
  auto e = parsePower();
  while (true) {
    if (peek() == MULTIPLY) {
      nextTok(); auto r = parsePower();
      e = make_unique<BinaryExpr>(BinaryExpr::Op::Mul, std::move(e), std::move(r));
    } else if (peek() == DIVIDE) {
      nextTok(); auto r = parsePower();
      e = make_unique<BinaryExpr>(BinaryExpr::Op::Div, std::move(e), std::move(r));
    } else if (peek() == MOD) {
      nextTok(); auto r = parsePower();
      e = make_unique<BinaryExpr>(BinaryExpr::Op::Mod, std::move(e), std::move(r));
    } else break;
  }
  return e;
}

static unique_ptr<Expr> parseSimple() {
  auto e = parseTerm();
  while (true) {
    if (peek() == PLUS)  { nextTok(); auto r = parseTerm(); e = make_unique<BinaryExpr>(BinaryExpr::Op::Add, std::move(e), std::move(r)); }
    else if (peek() == MINUS) { nextTok(); auto r = parseTerm(); e = make_unique<BinaryExpr>(BinaryExpr::Op::Sub, std::move(e), std::move(r)); }
    else break;
  }
  return e;
}

static unique_ptr<Expr> parseExpression() {
  // For Part 3, arithmetic only
  return parseSimple();
}


static unique_ptr<Statement> parseWriteStmt() {
  expect(WRITE, "in write statement");
  expect(OPENPAREN, "expected '(' after WRITE");

  unique_ptr<Statement> stmt;
  if (peek() == STRINGLIT) {
    string val = peekLex;
    // Strip quotes
    if (val.size() >= 2 && val.front()=='\'' && val.back()=='\'') {
      val = val.substr(1, val.length() - 2);
    }
    expect(STRINGLIT, "string literal");
    expect(CLOSEPAREN, "expected ')' after string literal");
    auto w = make_unique<WriteStmt>(WriteStmt::ArgKind::Str, val);
    stmt = std::move(w);
  } else if (peek() == IDENT) {
    string id = peekLex;
    if (!symbolTable.count(id)){
      throw runtime_error("Parse error: WRITE of undeclared identifier " + id);
    }
    expect(IDENT, "identifier in WRITE(...)");
    expect(CLOSEPAREN, "expected ')' after identifier");
    auto w = make_unique<WriteStmt>(WriteStmt::ArgKind::Id, id);
    stmt = std::move(w);
  } else {
    throw runtime_error("Parse error: expected STRINGLIT or IDENT inside WRITE(...)");
  }
  return stmt;
}

static unique_ptr<Statement> parseReadStmt() {
  expect(READ, "in read statement");
  expect(OPENPAREN, "expected '(' after READ");
  if (peek() != IDENT) throw runtime_error("Parse error: expected IDENT inside READ(...)");
  string id = peekLex;
  if (!symbolTable.count(id))
    throw runtime_error("Parse error: READ of undeclared identifier " + id);
  expect(IDENT, "identifier to READ into");
  expect(CLOSEPAREN, "expected ')' after identifier");
  return make_unique<ReadStmt>(id);
}


static unique_ptr<Statement> parseAssignStmtWithLeadingIdent(const string& firstIdent) {
  expect(ASSIGN, "expected ':=' after identifier");
  auto rhs = parseExpression();
  return make_unique<AssignStmt>(firstIdent, std::move(rhs));
}

static unique_ptr<Statement> parseAssignOrError() {
  if (peek() != IDENT) throw runtime_error("Parse error: expected IDENT to start assignment");
  string id = peekLex;
  if (!symbolTable.count(id))
    throw runtime_error("Parse error: ASSIGN to undeclared identifier " + id);
  nextTok();
  return parseAssignStmtWithLeadingIdent(id);
}





static unique_ptr<Statement> parseStatement() {
  switch (peek()) {
    case READ:       return parseReadStmt();
    case WRITE:      return parseWriteStmt();
    case TOK_BEGIN:  return parseCompound();
    case IDENT:      return parseAssignOrError();
    default:
      throw runtime_error(string("Parse error: unexpected token in statement: ") + tname(peek()));
  }
}

static unique_ptr<Statement> parseCompound() {
  expect(TOK_BEGIN, "expected BEGIN to start a compound statement");
  auto comp = make_unique<CompoundStmt>();

  if (peek() != END) {
    comp->stmts.push_back(parseStatement());
    while (accept(SEMICOLON)) {
      if (peek() == END) break;
      comp->stmts.push_back(parseStatement());
    }
  }

  expect(END, "expected END to close compound statement");
  return comp;
}


// block → BEGIN write END
unique_ptr<Block> parseBlock() {
  auto b = make_unique<Block>();
  parseDeclarations(b->decls);  // consume_if VAR ... ; ...
  b->body = unique_ptr<CompoundStmt>(
      static_cast<CompoundStmt*>(parseCompound().release())
  );
  return b;
}
// -----------------------------------------------------------------------------
// Program → PROGRAM IDENT ';' Block EOF
// -----------------------------------------------------------------------------
unique_ptr<Program> parseProgram() {
  expect(PROGRAM, "start of program");
  if (peek() != IDENT)
    throw runtime_error("Parse error: expected IDENT after PROGRAM");
  string nameLex = peekLex;  
  expect(IDENT, "program name");
  expect(SEMICOLON, "after program name");
  
  auto p = make_unique<Program>();
  p->name  = nameLex;
  p->block = parseBlock();

  expect(TOK_EOF, "at end of file (no trailing tokens after program)");
  return p;
}
