#include "compiler.h"
#include <stdexcept>

// ─── Helpers ──────────────────────────────────────────────────────────────────

int Compiler::getOrCreateSlot(const std::string& name) {
    auto it = varSlots.find(name);
    if (it != varSlots.end()) return it->second;
    varSlots[name] = nextSlot;
    return nextSlot++;
}

int Compiler::getSlot(const std::string& name) const {
    auto it = varSlots.find(name);
    if (it == varSlots.end()) {
        throw std::runtime_error("Compiler Error: Use of undeclared variable '" + name + "'");
    }
    return it->second;
}

void Compiler::emitByte(uint8_t byte) {
    code.push_back(byte);
}

void Compiler::emitInt32(int32_t val) {
    // Little-endian
    code.push_back((val >>  0) & 0xFF);
    code.push_back((val >>  8) & 0xFF);
    code.push_back((val >> 16) & 0xFF);
    code.push_back((val >> 24) & 0xFF);
}

void Compiler::emitOpInt(Opcode op, int32_t val) {
    emitByte(op);
    emitInt32(val);
}

size_t Compiler::emitJumpPlaceholder(Opcode op) {
    emitByte(op);
    size_t idx = code.size();
    emitInt32(0); // placeholder
    return idx;
}

void Compiler::patchJump(size_t placeholderIdx) {
    int32_t target = (int32_t)code.size();
    code[placeholderIdx + 0] = (target >>  0) & 0xFF;
    code[placeholderIdx + 1] = (target >>  8) & 0xFF;
    code[placeholderIdx + 2] = (target >> 16) & 0xFF;
    code[placeholderIdx + 3] = (target >> 24) & 0xFF;
}

void Compiler::emitJumpTo(Opcode op, size_t target) {
    emitByte(op);
    emitInt32((int32_t)target);
}

// ─── Expression compilation ───────────────────────────────────────────────────

void Compiler::compileExpr(const Expr* expr) {
    if (auto* n = dynamic_cast<const NumberExpr*>(expr)) {
        emitOpInt(OP_PUSH_INT, n->val);

    } else if (auto* b = dynamic_cast<const BoolExpr*>(expr)) {
        emitByte(OP_PUSH_BOOL);
        emitByte(b->val ? 1 : 0);

    } else if (auto* v = dynamic_cast<const VarExpr*>(expr)) {
        // Loading a variable requires it to have been declared previously
        if (declaredVars.find(v->name) == declaredVars.end()) {
            throw std::runtime_error("Compiler Error: Use of undeclared variable '" + v->name + "'");
        }
        int slot = getSlot(v->name);
        emitOpInt(OP_LOAD, slot);

    } else if (auto* bin = dynamic_cast<const BinaryExpr*>(expr)) {
        compileExpr(bin->left.get());
        compileExpr(bin->right.get());

        switch (bin->op) {
            case Tokentype::PLUS:          emitByte(OP_ADD); break;
            case Tokentype::MINUS:         emitByte(OP_SUB); break;
            case Tokentype::STAR:          emitByte(OP_MUL); break;
            case Tokentype::SLASH:         emitByte(OP_DIV); break;
            case Tokentype::EQUAL_EQUAL:   emitByte(OP_EQ);  break;
            case Tokentype::BANG_EQUAL:    emitByte(OP_NEQ); break;
            case Tokentype::LESS:          emitByte(OP_LT);  break;
            case Tokentype::LESS_EQUAL:    emitByte(OP_LTE); break;
            case Tokentype::GREATER:       emitByte(OP_GT);  break;
            case Tokentype::GREATER_EQUAL: emitByte(OP_GTE); break;
            default:
                throw std::runtime_error("Compiler Error: Unknown binary operator");
        }

    } else if (auto* un = dynamic_cast<const UnaryExpr*>(expr)) {
        compileExpr(un->right.get());
        if (un->op == Tokentype::MINUS) {
            emitByte(OP_NEGATE);
        } else {
            throw std::runtime_error("Compiler Error: Unknown unary operator");
        }

    } else {
        throw std::runtime_error("Compiler Error: Unknown expression type");
    }
}

// ─── Statement compilation ────────────────────────────────────────────────────

void Compiler::compileStmt(const Stmt* stmt) {
    if (auto* s = dynamic_cast<const LetStmt*>(stmt)) {
        // Compile initializer first, then create the slot and mark declared
        compileExpr(s->init.get());
        int slot = getOrCreateSlot(s->name);
        declaredVars.insert(s->name);
        emitOpInt(OP_STORE, slot);

    } else if (auto* s = dynamic_cast<const AssignStmt*>(stmt)) {
        // Assignment requires prior declaration
        if (declaredVars.find(s->name) == declaredVars.end()) {
            throw std::runtime_error("Compiler Error: Assignment to undeclared variable '" + s->name + "'");
        }
        compileExpr(s->value.get());
        int slot = getSlot(s->name);
        emitOpInt(OP_STORE, slot);

    } else if (auto* s = dynamic_cast<const PrintStmt*>(stmt)) {
        compileExpr(s->value.get());
        emitByte(OP_PRINT);

    } else if (auto* s = dynamic_cast<const InputStmt*>(stmt)) {
        // Input requires prior declaration
        if (declaredVars.find(s->name) == declaredVars.end()) {
            throw std::runtime_error("Compiler Error: Input to undeclared variable '" + s->name + "'");
        }
        int slot = getSlot(s->name);
        emitOpInt(OP_INPUT, slot);

    } else if (auto* s = dynamic_cast<const IfStmt*>(stmt)) {
        // Compile condition
        compileExpr(s->condition.get());

        // Jump past then-branch if false
        size_t jumpIfFalse = emitJumpPlaceholder(OP_JUMP_IF_FALSE);

        // Compile then-branch
        for (auto& st : s->thenBranch) compileStmt(st.get());

        if (!s->elseBranch.empty()) {
            // Jump past else-branch at end of then-branch
            size_t jumpOver = emitJumpPlaceholder(OP_JUMP);
            patchJump(jumpIfFalse);
            for (auto& st : s->elseBranch) compileStmt(st.get());
            patchJump(jumpOver);
        } else {
            patchJump(jumpIfFalse);
        }

    } else if (auto* s = dynamic_cast<const WhileStmt*>(stmt)) {
        // Mark loop start
        size_t loopStart = code.size();

        // Compile condition
        compileExpr(s->condition.get());

        // Jump past body if false
        size_t jumpIfFalse = emitJumpPlaceholder(OP_JUMP_IF_FALSE);

        // Compile body
        for (auto& st : s->body) compileStmt(st.get());

        // Jump back to loop start
        emitJumpTo(OP_JUMP, loopStart);

        // Patch the exit jump
        patchJump(jumpIfFalse);

    } else {
        throw std::runtime_error("Compiler Error: Unknown statement type");
    }
}

// ─── Public entry point ───────────────────────────────────────────────────────

Bytecode Compiler::compile(const std::vector<std::unique_ptr<Stmt>>& stmts) {
    for (auto& stmt : stmts) {
        compileStmt(stmt.get());
    }
    emitByte(OP_HALT);

    return Bytecode{code, nextSlot};
}