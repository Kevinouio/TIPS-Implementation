# ============================================================================
# Makefile - Build the Parsing + OO Lab starter
# ----------------------------------------------------------------------------
# Author: Derek Willis (Fall 2025)
# Students: you usually do NOT need to edit this file.
#   Only change if your platform or Flex install requires it.
# ============================================================================
#
# WHAT THIS DOES
#   - runs flex on rules.l (creates lex.yy.c)
#   - compiles all .cpp files and links them into the "parse" executable
#
# HOW TO USE
#   make                  # build (incremental)
#   ./parse input1.in     # run on a file (or just ./parse to read from stdin)
#   make run F=file.in    # convenience run target
#   make run-debug F=file # same, but with --debug (stderr traces)
#   make clean            # remove build artifacts
# ============================================================================

# ---- Variables (so we don't repeat ourselves) -------------------------------
CXX      := g++          # the C++ compiler
LEX      := flex         # the lexer generator (reads rules.l -> makes lex.yy.c)
CXXFLAGS := -std=gnu++17 -Wall -Wextra -O2  # compiler options (standard + warnings + optimize)
LDLIBS   := -ll         # link with the Flex library (needed on many systems)

# Project files (ingredients)
EXE      := parse
HEADERS  := ast.h printer.h parser.h lexer.h
SRCS     := ast.cpp printer.cpp parser.cpp driver.cpp
LEXER    := rules.l
LEXOUT_C := lex.yy.c
OBJS     := $(SRCS:.cpp=.o) lex.yy.o  # turn every .cpp into a .o, plus lex.yy.o

.PHONY: all clean run run-debug  # these names are "actions", not files

# ---- Default target: build the final program -------------------------------
all: $(EXE)

# Link step: how to make the dish "parse" from its object files (ingredients)
$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)
# $@ = parse (the target)
# $^ = all object files (the prerequisites)
# LDLIBS adds -lfl for Flex

# ---- Generate the scanner from the Flex rules ------------------------------
$(LEXOUT_C): $(LEXER)
	$(LEX) $(LEXER)
# When rules.l changes, re-run flex to produce lex.yy.c

# ---- Compile the generated scanner into an object file ---------------------
lex.yy.o: $(LEXOUT_C) $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
# $< = the first prerequisite = lex.yy.c

# ---- Generic rule: compile any .cpp to a matching .o -----------------------
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@
# This rule works for ast.cpp -> ast.o, printer.cpp -> printer.o, etc.

# ---- Convenience run targets (not required, just handy) --------------------
run: $(EXE)
	./$(EXE) $(F)
# usage: make run F=input1.in

run-debug: $(EXE)
	./$(EXE) --debug $(F)
# usage: make run-debug F=input1.in
# --debug prints parser/lexer traces to stderr; stdout stays clean

# ---- Clean up build artifacts ----------------------------------------------
clean:
	rm -f $(EXE) *.o lex.yy.c
# "make clean" removes the compiled program, object files, and generated scanner.
