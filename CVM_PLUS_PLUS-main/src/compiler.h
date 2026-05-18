#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <cstdint>
#include "ast.h"
#include "vm.h"

struct Bytecode {
    std::vector<uint8_t> code;
    int numVars = 0;
};

class Compiler {
private:
    std::vector<uint8_t> code;
    std::unordered_map<std::string, int> varSlots;
    std::unordered_set<std::string> declaredVars;
    int nextSlot = 0;
    int getOrCreateSlot(const std::string& name);
    int getSlot(const std::string& name) const;

    void emitByte(uint8_t byte);
    void emitInt32(int32_t val);
    void emitOpInt(Opcode op, int32_t val);

    // Returns the index of the placeholder to patch later
    size_t emitJumpPlaceholder(Opcode op);
    void patchJump(size_t placeholderIdx);
    void emitJumpTo(Opcode op, size_t target);

    void compileExpr(const Expr* expr);
    void compileStmt(const Stmt* stmt);

public:
    Bytecode compile(const std::vector<std::unique_ptr<Stmt>>& stmts);
};
