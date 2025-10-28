#!/usr/bin/env bash
set -u

# -----------------------------
# Config
# -----------------------------
PARSE_BIN="${PARSE_BIN:-./parse}"         # your Part 1 executable
INPUT_DIR="TestCasesPart1"                # test inputs
EXPECTED_DIR="ExpectedOutputs"            # expected outputs
ACTUAL_DIR="ActualOutputs"                # actual outputs
TESTS=("1-test" "2-test" "3-test" "4-test" "5-test")

# -----------------------------
# Colors & symbols
# -----------------------------
BOLD=$'\e[1m'; DIM=$'\e[2m'; RESET=$'\e[0m'
RED=$'\e[31m'; GREEN=$'\e[32m'; YELLOW=$'\e[33m'; BLUE=$'\e[34m'; MAGENTA=$'\e[35m'; CYAN=$'\e[36m'
OK="${GREEN}✔${RESET}"; FAIL="${RED}✘${RESET}"; WARN="${YELLOW}!${RESET}"; ARROW="${BLUE}▶${RESET}"

# -----------------------------
# Preflight
# -----------------------------
if [[ ! -x "$PARSE_BIN" ]]; then
  echo "${RED}Error:${RESET} ${BOLD}PARSE_BIN${RESET} not found or not executable: ${BOLD}$PARSE_BIN${RESET}"
  exit 2
fi

mkdir -p "$EXPECTED_DIR" "$ACTUAL_DIR"

norm() { sed -e 's/\r$//' "$1"; }

color_diff() {
  local f="$1"
  while IFS= read -r line; do
    case "$line" in
      '+++ '*|'--- '*|'@@ '*)
        printf "%s%s%s\n" "$CYAN" "$line" "$RESET"
        ;;
      +*)
        printf "%s%s%s\n" "$GREEN" "$line" "$RESET"
        ;;
      -*)
        printf "%s%s%s\n" "$RED" "$line" "$RESET"
        ;;
      *)
        printf "%s\n" "$line"
        ;;
    esac
  done < "$f"
}

run_one() {
  local name="$1"
  local in_file="$INPUT_DIR/${name}.in"
  local exp_file="$EXPECTED_DIR/${name}.out"
  local act_file="$ACTUAL_DIR/${name}.out"

  if [[ ! -f "$in_file" ]]; then
    echo "  ${WARN} ${YELLOW}Missing input${RESET}: ${BOLD}$in_file${RESET}"
    return 3
  fi
  if [[ ! -f "$exp_file" ]]; then
    echo "  ${WARN} ${YELLOW}Missing expected${RESET}: ${BOLD}$exp_file${RESET}"
    return 4
  fi

  # Temps
  local t_out p_out t_err p_err
  t_out="$(mktemp)"; p_out="$(mktemp)"
  t_err="$(mktemp)"; p_err="$(mktemp)"

  # TOKENS phase
  "$PARSE_BIN" -t < "$in_file" > "$t_out" 2> "$t_err"
  local rc_t=$?

  # PARSE phase
  "$PARSE_BIN" -p < "$in_file" > "$p_out" 2> "$p_err"
  local rc_p=$?

  {
    echo "### FILE: ${name}.in"
    echo "### TOKENS (stdout)"
    norm "$t_out"
    echo
    echo "### PARSE (stdout)"
    norm "$p_out"
    echo
    echo "### STDERR (tokens + parse)"
    echo "[tokens stderr]"
    norm "$t_err"
    echo
    echo "[parse stderr]"
    norm "$p_err"
    echo
    echo "### EXITCODES"
    echo "tokens_rc=$rc_t"
    echo "parse_rc=$rc_p"
  } > "$act_file"

  rm -f "$t_out" "$p_out" "$t_err" "$p_err"

  local difffile; difffile="$(mktemp)"
  if diff -u --label "expected/${name}.out" --label "actual/${name}.out" "$exp_file" "$act_file" > "$difffile"; then
    echo "  ${OK} ${GREEN}MATCH${RESET} ${DIM}(${name})${RESET}"
    rm -f "$difffile"
    return 0
  else
    echo "  ${FAIL} ${RED}MISMATCH${RESET} ${DIM}(${name})${RESET}"
    color_diff "$difffile"
    rm -f "$difffile"
    return 1
  fi
}

echo
echo "${BOLD}${MAGENTA}TIPS Part 1: Demo Runner${RESET}  ${DIM}(tokens, parse, stderr, exit codes)${RESET}"
echo "${DIM}Binary:${RESET} $PARSE_BIN"
echo "${DIM}Inputs:${RESET} $INPUT_DIR    ${DIM}Expected:${RESET} $EXPECTED_DIR    ${DIM}Actual:${RESET} $ACTUAL_DIR"
echo

fail=0; miss=0
for t in "${TESTS[@]}"; do
  printf "%s %s%s%s\n" "$ARROW" "$BOLD" "$t" "$RESET"
  if ! run_one "$t"; then
    rc=$?
    case $rc in
      3|4) ((miss++)) ;;
      *)   ((fail++)) ;;
    esac
  fi
  echo
done

echo "${BOLD}Summary:${RESET}"
echo "  ${OK} Matched:   $(( ${#TESTS[@]} - fail - miss ))"
echo "  ${FAIL} Mismatch: ${fail}"
echo "  ${WARN} Missing:  ${miss}"
echo

(( fail > 0 || miss > 0 )) && exit 1 || exit 0
