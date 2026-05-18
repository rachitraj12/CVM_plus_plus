#include "vm.h"
#include <stdexcept>
#include <iostream>

void VM::load(const std::vector<uint8_t>& bytecode, int numVars) {
    code = bytecode;
    ip   = 0;
    stack.clear();
    // Initialize variable slots with integer 0
    vars.assign(numVars, Value::integer(0));
}

int32_t VM::readInt32() {
    // Little-endian 4-byte int
    int32_t val = 0;
    val |= (int32_t)code[ip]     << 0;
    val |= (int32_t)code[ip + 1] << 8;
    val |= (int32_t)code[ip + 2] << 16;
    val |= (int32_t)code[ip + 3] << 24;
    ip += 4;
    return val;
}

void VM::run() {
    while (ip < code.size()) {
        uint8_t instr = code[ip++];

        switch (instr) {

            case OP_PUSH_INT: {
                int32_t val = readInt32();
                stack.push_back(Value::integer(val));
                break;
            }

            case OP_PUSH_BOOL: {
                uint8_t val = code[ip++];
                stack.push_back(Value::boolean(val));
                break;
            }

            case OP_LOAD: {
                int32_t slot = readInt32();
                stack.push_back(vars[slot]);
                break;
            }

            case OP_STORE: {
                int32_t slot = readInt32();
                Value val = stack.back(); stack.pop_back();
                vars[slot] = val;
                break;
            }

            case OP_ADD: {
                Value b = stack.back(); stack.pop_back();
                Value a = stack.back(); stack.pop_back();
                if (a.type != ValueType::INT || b.type != ValueType::INT) {
                    throw std::runtime_error("Runtime Error: Operands for '+' must be integers");
                }
                stack.push_back(Value::integer(a.asInt + b.asInt));
                break;
            }

            case OP_SUB: {
                Value b = stack.back(); stack.pop_back();
                Value a = stack.back(); stack.pop_back();
                if (a.type != ValueType::INT || b.type != ValueType::INT) {
                    throw std::runtime_error("Runtime Error: Operands for '-' must be integers");
                }
                stack.push_back(Value::integer(a.asInt - b.asInt));
                break;
            }

            case OP_MUL: {
                Value b = stack.back(); stack.pop_back();
                Value a = stack.back(); stack.pop_back();
                if (a.type != ValueType::INT || b.type != ValueType::INT) {
                    throw std::runtime_error("Runtime Error: Operands for '*' must be integers");
                }
                stack.push_back(Value::integer(a.asInt * b.asInt));
                break;
            }

            case OP_DIV: {
                Value b = stack.back(); stack.pop_back();
                Value a = stack.back(); stack.pop_back();
                if (a.type != ValueType::INT || b.type != ValueType::INT) {
                    throw std::runtime_error("Runtime Error: Operands for '/' must be integers");
                }
                if (b.asInt == 0) throw std::runtime_error("Runtime Error: Division by zero");
                stack.push_back(Value::integer(a.asInt / b.asInt));
                break;
            }

            case OP_EQ: {
                Value b = stack.back(); stack.pop_back();
                Value a = stack.back(); stack.pop_back();
                // Strict equality: Types must match AND values must match
                bool isEqual = (a.type == b.type) && (a.asInt == b.asInt);
                stack.push_back(Value::boolean(isEqual ? 1 : 0));
                break;
            }

            case OP_NEQ: {
                Value b = stack.back(); stack.pop_back();
                Value a = stack.back(); stack.pop_back();
                bool isEqual = (a.type == b.type) && (a.asInt == b.asInt);
                stack.push_back(Value::boolean(isEqual ? 0 : 1));
                break;
            }

            case OP_LT: {
                Value b = stack.back(); stack.pop_back();
                Value a = stack.back(); stack.pop_back();
                if (a.type != ValueType::INT || b.type != ValueType::INT) {
                    throw std::runtime_error("Runtime Error: Operands for '<' must be integers");
                }
                stack.push_back(Value::boolean(a.asInt < b.asInt ? 1 : 0));
                break;
            }

            case OP_LTE: {
                Value b = stack.back(); stack.pop_back();
                Value a = stack.back(); stack.pop_back();
                if (a.type != ValueType::INT || b.type != ValueType::INT) {
                    throw std::runtime_error("Runtime Error: Operands for '<=' must be integers");
                }
                stack.push_back(Value::boolean(a.asInt <= b.asInt ? 1 : 0));
                break;
            }

            case OP_GT: {
                Value b = stack.back(); stack.pop_back();
                Value a = stack.back(); stack.pop_back();
                if (a.type != ValueType::INT || b.type != ValueType::INT) {
                    throw std::runtime_error("Runtime Error: Operands for '>' must be integers");
                }
                stack.push_back(Value::boolean(a.asInt > b.asInt ? 1 : 0));
                break;
            }

            case OP_GTE: {
                Value b = stack.back(); stack.pop_back();
                Value a = stack.back(); stack.pop_back();
                if (a.type != ValueType::INT || b.type != ValueType::INT) {
                    throw std::runtime_error("Runtime Error: Operands for '>=' must be integers");
                }
                stack.push_back(Value::boolean(a.asInt >= b.asInt ? 1 : 0));
                break;
            }

            case OP_NEGATE: {
                Value a = stack.back(); stack.pop_back();
                if (a.type != ValueType::INT) {
                    throw std::runtime_error("Runtime Error: Operand for '-' must be an integer");
                }
                stack.push_back(Value::integer(-a.asInt));
                break;
            }

            case OP_JUMP: {
                int32_t target = readInt32();
                ip = (size_t)target;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                int32_t target = readInt32();
                Value cond = stack.back(); stack.pop_back();
                
                // Enforce that the condition is actually a boolean
                if (cond.type != ValueType::BOOL) {
                    throw std::runtime_error("Runtime Error: Condition in if/while must be a boolean");
                }
                
                if (!cond.asInt) ip = (size_t)target;
                break;
            }

            case OP_PRINT: {
                Value val = stack.back(); stack.pop_back();
                if (val.type == ValueType::INT) {
                    std::cout << val.asInt << "\n";
                } else {
                    std::cout << (val.asInt ? "true" : "false") << "\n";
                }
                break;
            }

            case OP_INPUT: {
                int32_t slot = readInt32();
                int val;
                std::cin >> val;
                vars[slot] = Value::integer(val);
                break;
            }

            case OP_HALT:
                return;

            default:
                throw std::runtime_error("Runtime Error: Unknown opcode " + std::to_string(instr));
        }
    }
}