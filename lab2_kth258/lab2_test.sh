#!/bin/bash
# lab2_test.sh — test script for Lab 2
# Builds parser and runs against all provided inputs

set -e

echo "=============================="
echo "Lab 2 Test Script"
echo "User: $(whoami)"
echo "Date: $(date)"
echo "=============================="

# == Build ==
echo -e "\033[1;34m== Building ==\033[0m"
rm -f parse *.o lex.yy.c
g++ -std=gnu++17 -Wall -Wextra -O2 -c ast.cpp -o ast.o
g++ -std=gnu++17 -Wall -Wextra -O2 -c printer.cpp -o printer.o
g++ -std=gnu++17 -Wall -Wextra -O2 -c parser.cpp -o parser.o
g++ -std=gnu++17 -Wall -Wextra -O2 -c driver.cpp -o driver.o
flex rules.l
g++ -std=gnu++17 -Wall -Wextra -O2 ast.o printer.o parser.o driver.o lex.yy.c -o parse

# == Run Tests ==
for f in input*.in; do
    echo -e "\033[1;33m-- Running $f --\033[0m"
    if ./parse "$f" > "$f.out" 2> "$f.err"; then
        echo -e "\033[1;32mPASS: $f\033[0m"
    else
        echo -e "\033[1;31mFAIL: $f\033[0m"
        cat "$f.err"
    fi
done
