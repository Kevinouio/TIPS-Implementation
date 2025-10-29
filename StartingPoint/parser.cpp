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
static inline void consume_if(Token t) { if (peek() == t) nextTok(); }

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

static unique_ptr<Value> parseValue() {
  if (peek() == INTLIT) {
    string lex = peekLex; nextTok();
    return make_unique<Value>(Value::Kind::IntLit, lex);
  } else if (peek() == FLOATLIT) {
    string lex = peekLex; nextTok();
    return make_unique<Value>(Value::Kind::FloatLit, lex);
  } else if (peek() == IDENT) {
    string lex = peekLex; nextTok();
    return make_unique<Value>(Value::Kind::Ident, lex);
  }
  throw runtime_error("Parse error: expected a value (INTLIT, FLOATLIT, or IDENT)");
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
  consume_if(SEMICOLON);
  return stmt;
}

static unique_ptr<Statement> parseReadStmt() {
  expect(READ, "in read statement");
  if (peek() != IDENT) throw runtime_error("Parse error: expected IDENT after READ");
  string id = peekLex;
  if (!symbolTable.count(id)){
    throw runtime_error("Parse error: READ of undeclared identifier " + id);
  }
  expect(IDENT, "identifier to READ into");
  consume_if(SEMICOLON);
  return make_unique<ReadStmt>(id);
}

static unique_ptr<Statement> parseAssignStmtWithLeadingIdent(const string& firstIdent) {
  expect(ASSIGN, "expected ':=' after identifier");
  auto rhs = parseValue();
  consume_if(SEMICOLON);
  return make_unique<AssignStmt>(firstIdent, std::move(rhs));
}

static unique_ptr<Statement> parseAssignOrError() {
  if (peek() != IDENT) throw runtime_error("Parse error: expected IDENT to start assignment");
  string id = peekLex;
  nextTok(); // consume IDENT
  return parseAssignStmtWithLeadingIdent(id);
}

static unique_ptr<Statement> parseCompound(); // fwd

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
  // Zero or more statements until END
  while (true) {
    Token t = peek();
    if (t == END) break;
    // a tiny safety: permit END right away (empty block)
    comp->stmts.push_back(parseStatement());
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

