#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <iostream>

// ─── Instruction Set Architecture ────────────────────────────────────────────
//
//  OP_PUSH_INT  <4 bytes: int32 little-endian>  -- push literal integer
//  OP_PUSH_BOOL <1 byte: 0/1>                   -- push literal boolean
//  OP_LOAD      <4 bytes: var-slot index>        -- push value of variable[slot]
//  OP_STORE     <4 bytes: var-slot index>        -- pop and store into variable[slot]
//  OP_ADD / OP_SUB / OP_MUL / OP_DIV           -- arithmetic (pop 2, push 1)
//  OP_EQ / OP_NEQ / OP_LT / OP_LTE             -- comparison  (pop 2, push 1)
//  OP_GT / OP_GTE
//  OP_JUMP      <4 bytes: absolute target>       -- unconditional jump
//  OP_JUMP_IF_FALSE <4 bytes: target>            -- pop; jump if 0/false
//  OP_PRINT                                      -- pop and print
//  OP_INPUT     <4 bytes: var-slot index>        -- read int from stdin into slot
//  OP_HALT                                       -- stop execution
//

enum Opcode : uint8_t {
    OP_PUSH_INT,
    OP_PUSH_BOOL,
    OP_LOAD,
    OP_STORE,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_LTE,
    OP_GT,
    OP_GTE,
    OP_NEGATE,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_PRINT,
    OP_INPUT,
    OP_HALT,
};

enum class ValueType { INT, BOOL };

struct Value {
    ValueType type;
    int asInt; // We use an int to hold both, but we track the TYPE

    // Helper constructors
    static Value integer(int val) { return {ValueType::INT, val}; }
    static Value boolean(int val) { return {ValueType::BOOL, val}; }
};

class VM {
private:
    std::vector<uint8_t> code;
    std::vector<Value>     stack;
    std::vector<Value>     vars;   // variable slots
    size_t ip = 0;

    int32_t readInt32();

public:
    void load(const std::vector<uint8_t>& bytecode, int numVars);
    void run();
};
