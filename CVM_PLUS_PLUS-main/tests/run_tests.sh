#!/usr/bin/env bash
# POSIX test runner for CVM++
# Runs all .cvm files in the tests/ directory using build/cvm

EXE="./build/cvm"
if [ ! -x "$EXE" ]; then
  echo "Executable not found: $EXE. Build the project first (see README)." >&2
  exit 1
fi

for t in tests/*.cvm; do
  echo "======================================="
  echo "Running: $(basename "$t")"
  echo "======================================="
  if [ "$(basename "$t")" = "test_input.cvm" ]; then
    echo 7 | "$EXE" "$t"
  else
    "$EXE" "$t"
  fi
  echo
done

echo "All tests executed."
exit 0
