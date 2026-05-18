# CVM++ — Stack-Based Virtual Machine & Custom Compiler

> A lightweight scripting language built from scratch in C++.  
> Source code → Lexer → Parser → AST → Bytecode Compiler → Stack-based VM

Built as part of the **IIT Guwahati Coding Club Even Semester Projects 2026**.

---

## Table of Contents
- [Overview](#overview)
- [Architecture](#architecture)
- [Language Reference](#language-reference)
- [Building](#building)
- [Running](#running)
- [Debug Mode](#debug-mode)
- [Sample Scripts](#sample-scripts)
- [Instruction Set Architecture (ISA)](#instruction-set-architecture-isa)
- [Known Limitations](#known-limitations)
- [Project Structure](#project-structure)
- [Running Tests](#running-tests)
- [References](#references)

---

## Overview

CVM++ is a complete compiler + runtime pipeline for a custom scripting language (`.cvm` files). It demonstrates every stage of language implementation:

```
source.cvm  →  [Lexer]  →  Tokens
            →  [Parser] →  Abstract Syntax Tree (AST)
            →  [Compiler] → Bytecode ([]uint8)
            →  [VM]     →  Execution output
```

---

## Architecture

### Lexer (`lexer.h / lexer.cpp`)
Converts raw source text into a flat list of tokens. Handles keywords, identifiers, integer literals, boolean literals, operators, and single-line comments (`//`).

### Parser (`parser.h / parser.cpp`)
Recursive descent parser. Converts the token stream into an **Abstract Syntax Tree** using precedence climbing:
```
parseExpr → parseComparison → parseAddSub → parseMulDiv → parseUnary → parsePrimary
```

### AST (`ast.h`)
Defines the node types for both expressions and statements using `unique_ptr` ownership:
- **Expressions:** `NumberExpr`, `BoolExpr`, `VarExpr`, `BinaryExpr`, `UnaryExpr`
- **Statements:** `LetStmt`, `AssignStmt`, `PrintStmt`, `InputStmt`, `IfStmt`, `WhileStmt`

### AST Printer (`ast_printer.h / ast_printer.cpp`)
Walks the AST and renders a human-readable tree to stdout (used in `--debug` mode).

### Compiler (`compiler.h / compiler.cpp`)
Walks the AST and emits a flat `vector<uint8_t>` of bytecode instructions. Uses **little-endian 4-byte integers** for operands. Handles:
- Forward jump patching for `if/else`
- Back-jump emission for `while` loops
- Variable slot allocation via a hash map

### VM (`vm.h / vm.cpp`)
A register-free, **stack-based** execution engine. Maintains:
- `stack` — operand stack (`vector<Value>`)
- `vars` — variable slots (`vector<Value>`)
- `ip` — instruction pointer

---

## Language Reference

### Data Types
| Type    | Examples          |
|---------|-------------------|
| Integer | `0`, `42`, `-7`   |
| Boolean | `true`, `false`   |

### Variable Declaration & Assignment
```
let x = 10;
let flag = true;
x = x + 1;          // re-assignment (no 'let')
```

### Operators
| Category    | Operators                          |
|-------------|-----------------------------------|
| Arithmetic  | `+`  `-`  `*`  `/`                |
| Comparison  | `==`  `!=`  `<`  `<=`  `>`  `>=` |
| Unary       | `-` (negation)                    |

> Note: Comparison operators always return a `bool`. Arithmetic operators require `int` operands.

### Control Flow
```
if (condition) {
    // ...
} else {
    // optional
}

while (condition) {
    // ...
}
```

### I/O
```
print expr;     // prints an integer or boolean, followed by newline
input x;        // reads one integer from stdin into variable x
```

### Comments
```
// This is a single-line comment
```

---

## Building

### Prerequisites
- C++17 compiler (`g++` >= 8, or `clang++` >= 7)
- CMake >= 3.16 (optional)

### With CMake (recommended)
```bash
# Windows (PowerShell) — run from project root
mkdir build
cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja
# Binary: build/cvm.exe

# macOS (Terminal) — run from project root
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
# Binary: build/cvm
```

### Direct compile
```bash
# Windows (PowerShell)
g++ -std=c++17 -O2 -o cvm.exe src/main.cpp src/lexer.cpp src/parser.cpp src/compiler.cpp src/vm.cpp src/ast_printer.cpp -I src

# macOS (Terminal)
g++ -std=c++17 -O2 -o cvm src/main.cpp src/lexer.cpp src/parser.cpp src/compiler.cpp src/vm.cpp src/ast_printer.cpp -I src
```

---

## Running

### Run a `.cvm` file
```bash
# Windows
.\build\cvm.exe tests\test_arithmetic.cvm

# macOS
./build/cvm tests/test_arithmetic.cvm
```

### Interactive REPL
> End multi-line input with a **blank line** to execute.
```bash
# Windows
.\build\cvm.exe

# macOS
./build/cvm
```
```
cvm>> let x = 5;
cvm>> print x * 2;

10
cvm>> exit
```

### Pipe input
```bash
# Windows
cmd /c "echo 7 | .\build\cvm.exe tests\test_input.cvm"

# macOS
echo "7" | ./build/cvm tests/test_input.cvm
```

---

## Debug Mode

Pass `--debug` (or `-d`) to see all four internal stages:

```bash
# Windows
.\build\cvm.exe --debug tests\test_if_else.cvm

# macOS
./build/cvm --debug tests/test_if_else.cvm
```

Prints in order:
1. **Token Stream** — every token with its type and lexeme
2. **Abstract Syntax Tree** — indented tree of all AST nodes
3. **Bytecode Disassembly** — each opcode with its operand and address
4. **Execution output** — the actual program result

---

## Sample Scripts

| Script | Command | Expected Output |
|--------|---------|----------------|
| `test_arithmetic.cvm` | `./build/cvm tests/test_arithmetic.cvm` | `25` `200` `20` `-10` `40` |
| `test_booleans.cvm` | `./build/cvm tests/test_booleans.cvm` | `true` `true` `false` `false` `false` `true` `true` `false` `true` |
| `test_if_else.cvm` | `./build/cvm tests/test_if_else.cvm` | `0` `42` `3` `15` |
| `test_while.cvm` | `./build/cvm tests/test_while.cvm` | `1 2 3 4 5` then `55` then `120` |
| `test_fizzbuzz.cvm` | `./build/cvm tests/test_fizzbuzz.cvm` | FizzBuzz 1–20 (3=Fizz, 5=Buzz, 15=FizzBuzz) |
| `test_input.cvm` | `echo "7" \| ./build/cvm tests/test_input.cvm` | `7` then `14` |

---

## Instruction Set Architecture (ISA)

All integers are encoded **little-endian 32-bit** immediately after the opcode byte.

| Opcode | Operand | Description |
|--------|---------|-------------|
| `OP_PUSH_INT` | `int32` | Push integer literal onto stack |
| `OP_PUSH_BOOL` | `uint8 (0/1)` | Push boolean literal onto stack |
| `OP_LOAD` | `int32 slot` | Push value of variable[slot] |
| `OP_STORE` | `int32 slot` | Pop top of stack → variable[slot] |
| `OP_ADD` | — | Pop a, b; push a+b |
| `OP_SUB` | — | Pop a, b; push a-b |
| `OP_MUL` | — | Pop a, b; push a*b |
| `OP_DIV` | — | Pop a, b; push a/b (errors on div-by-zero) |
| `OP_EQ` | — | Pop a, b; push (a==b) as bool |
| `OP_NEQ` | — | Pop a, b; push (a!=b) as bool |
| `OP_LT` | — | Pop a, b; push (a<b) as bool |
| `OP_LTE` | — | Pop a, b; push (a<=b) as bool |
| `OP_GT` | — | Pop a, b; push (a>b) as bool |
| `OP_GTE` | — | Pop a, b; push (a>=b) as bool |
| `OP_NEGATE` | — | Pop a; push -a |
| `OP_JUMP` | `int32 target` | Unconditional jump to absolute address |
| `OP_JUMP_IF_FALSE` | `int32 target` | Pop bool; jump if false |
| `OP_PRINT` | — | Pop and print value to stdout |
| `OP_INPUT` | `int32 slot` | Read int from stdin → variable[slot] |
| `OP_HALT` | — | Stop execution |

---

## Known Limitations

- No function definitions or call support (no user-defined functions or procedures).
- No string type or string operations — only integers and booleans are supported.
- Variables must be declared with `let` before use (the compiler enforces this).
- Single-line comments only (`//`). No multi-line comment support.

---

## Project Structure

```
CVM_PLUS_PLUS/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp              # Entry point: REPL, file runner, debug pipeline
│   ├── lexer.h / .cpp        # Tokenizer
│   ├── parser.h / .cpp       # Recursive descent parser -> AST
│   ├── ast.h                 # AST node definitions
│   ├── ast_printer.h / .cpp  # Debug AST pretty-printer
│   ├── compiler.h / .cpp     # AST -> Bytecode compiler
│   └── vm.h / .cpp           # Stack-based bytecode VM
└── tests/
    ├── run_tests.sh           # Automated test runner (macOS/Linux)
    ├── run_tests.ps1          # Automated test runner (Windows)
    ├── test_arithmetic.cvm
    ├── test_booleans.cvm
    ├── test_if_else.cvm
    ├── test_while.cvm
    ├── test_fizzbuzz.cvm
    └── test_input.cvm
```

---

## Running Tests

Two automated runners are included in `tests/` to exercise all sample scripts:

- `tests/run_tests.sh` — POSIX shell runner (macOS, Linux, WSL)
- `tests/run_tests.ps1` — PowerShell runner (Windows)

**Windows (PowerShell):**
```powershell
powershell -ExecutionPolicy Bypass -File .\tests\run_tests.ps1
```

**macOS / Linux:**
```bash
bash tests/run_tests.sh
```

> **Note:** Build the project first before running tests. For `test_input.cvm` on Windows, use cmd-style piping if PowerShell gives unexpected results:
> ```cmd
> cmd /c "echo 7 | .\build\cvm.exe tests\test_input.cvm"
> ```

---

## References

- [Crafting Interpreters](https://craftinginterpreters.com/) by Robert Nystrom
- [Writing a Lexer in C++](https://llvm.org/docs/tutorial/)
- [Understanding Stack-Based Virtual Machines](https://en.wikipedia.org/wiki/Stack_machine)

---
