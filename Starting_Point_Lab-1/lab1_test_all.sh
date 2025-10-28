#!/usr/bin/env bash
# Lab 1 test harness — auto-classifies CSVs using companion .out files and filename patterns.
# BAD cases are now also diffed to their .out files when present.

set -euo pipefail

BIN="./lex"
MAKE="make -s"

# Pretty printing
green() { printf "\033[32m%s\033[0m" "$*"; }
red()   { printf "\033[31m%s\033[0m" "$*"; }
cyan()  { printf "\033[36m%s\033[0m" "$*"; }

pass() { green "✔ "; printf "%s\n" "$1"; }
fail() { red   "✘ "; printf "%s\n" "$1"; }
info() { cyan  "• "; printf "%s\n" "$1"; }

# Print run identity
info "Test run started: $(date)"
info "User: $(whoami)"

# Build step
info "Building with make..."
$MAKE
if [[ ! -x "$BIN" ]]; then
  fail "Build did not produce $BIN"
  exit 1
fi
pass "Build complete"

# Helpers
run_good() {
  local in="$1"
  local expected="${2:-}"
  info "GOOD: $in"
  set +e
  out=$($BIN "$in" 2>&1)
  status=$?
  set -e
  if [[ $status -ne 0 ]]; then
    echo "$out"
    fail "$in exited with status $status"
    return 1
  fi
  if echo "$out" | grep -q "ERROR"; then
    echo "$out"
    fail "$in produced ERROR output"
    return 1
  fi
  pass "$in ran without ERROR"
  if [[ -n "$expected" && -f "$expected" ]]; then
    if diff -u <(echo "$out") "$expected"; then
      pass "$in matches expected output ($expected)"
    else
      fail "$in does not match expected output ($expected)"
      return 1
    fi
  fi
  return 0
}

run_bad() {
  local in="$1"
  local expected="${2:-}"
  info "BAD (should error): $in"
  set +e
  out=$($BIN "$in" 2>&1)
  status=$?
  set -e
  if ! echo "$out" | grep -q "ERROR"; then
    echo "$out"
    fail "$in did not report an ERROR as expected"
    return 1
  fi
  pass "$in produced ERROR output as expected"
  # If an expected .out is provided, diff the entire output
  if [[ -n "$expected" && -f "$expected" ]]; then
    if diff -u <(echo "$out") "$expected"; then
      pass "$in error output matches expected ($expected)"
    else
      fail "$in error output does not match expected ($expected)"
      return 1
    fi
  fi
  return 0
}

# BAD patterns (files that should error if no .out guides them)
BAD_PATTERNS=("*_bad.csv" "another.csv" "unknown2.csv")

is_bad_by_pattern() {
  local name="$1"
  for pat in "${BAD_PATTERNS[@]}"; do
    if [[ "$name" == $pat ]]; then
      return 0
    fi
  done
  return 1
}

# Main: iterate over all .csv files present
any_fail=0
shopt -s nullglob
for csv in *.csv; do
  base="${csv%.csv}"
  out_file="${base}.out"
  if [[ -f "$out_file" ]] && grep -q "ERROR" "$out_file"; then
    run_bad "$csv" "$out_file" || any_fail=1
  elif is_bad_by_pattern "$csv"; then
    if [[ -f "$out_file" ]]; then
      run_bad "$csv" "$out_file" || any_fail=1
    else
      run_bad "$csv" || any_fail=1
    fi
  else
    if [[ -f "$out_file" ]]; then
      run_good "$csv" "$out_file" || any_fail=1
    else
      run_good "$csv" || any_fail=1
    fi
  fi
done

if [[ $any_fail -eq 0 ]]; then
  pass "All tests passed expectations."
  exit 0
else
  fail "Some tests failed expectations."
  exit 1
fi
