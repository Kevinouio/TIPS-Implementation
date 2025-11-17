#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

TARGET="./parse"
TEST_DIR="TestCasesPart3"

if [[ ! -x "$TARGET" ]]; then
  echo "[build] parser binary not found, running make..."
  make
fi

CASES=()
while IFS= read -r line; do
  [[ -n "$line" ]] && CASES+=("$line")
done < <(find "$TEST_DIR" -maxdepth 1 -type f -name '*.tips' | sort)

if (( ${#CASES[@]} == 0 )); then
  echo "No .tips files found under $TEST_DIR" >&2
  exit 1
fi

for case_path in "${CASES[@]}"; do
  base="$(basename "$case_path")"
  stem="${base%.tips}"
  input_file="$TEST_DIR/$stem.in"

  echo
  echo "===== $stem ====="
  if [[ -f "$input_file" ]]; then
    echo "[input] using $input_file"
    "$TARGET" "$case_path" < "$input_file"
  else
    cat <<EOF
[input] no $input_file present.
        Provide input for $stem now, then press Ctrl-D (EOF) when finished.
EOF
    "$TARGET" "$case_path"
  fi
done
