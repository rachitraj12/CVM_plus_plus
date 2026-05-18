#pragma once
#include <memory>
#include <string>
#include <vector>
#include "lexer.h"

// ─── Base ────────────────────────────────────────────────────────────────────

struct Expr {
    virtual ~Expr() = default;
};

struct Stmt {
    virtual ~Stmt() = default;
};

// ─── Expressions ─────────────────────────────────────────────────────────────

// A literal integer: 42
struct NumberExpr : public Expr {
    int val;
    NumberExpr(int val) : val(val) {}
};

// A literal boolean: true / false
struct BoolExpr : public Expr {
    bool val;
    BoolExpr(bool val) : val(val) {}
};

// A variable reference: x
struct VarExpr : public Expr {
    std::string name;
    VarExpr(const std::string& name) : name(name) {}
};

// A binary operation: left OP right
struct BinaryExpr : public Expr {
    Tokentype op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> left, Tokentype op, std::unique_ptr<Expr> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}
};

// A unary operation: OP right (e.g., -5 or !true)
struct UnaryExpr : public Expr {
    Tokentype op;
    std::unique_ptr<Expr> right;

    UnaryExpr(Tokentype op, std::unique_ptr<Expr> right)
        : op(op), right(std::move(right)) {}
};
// ─── Statements ──────────────────────────────────────────────────────────────

// let x = expr;
struct LetStmt : public Stmt {
    std::string name;
    std::unique_ptr<Expr> init;
    LetStmt(const std::string& name, std::unique_ptr<Expr> init)
        : name(name), init(std::move(init)) {}
};

// x = expr;  (assignment)
struct AssignStmt : public Stmt {
    std::string name;
    std::unique_ptr<Expr> value;
    AssignStmt(const std::string& name, std::unique_ptr<Expr> value)
        : name(name), value(std::move(value)) {}
};

// print expr;
struct PrintStmt : public Stmt {
    std::unique_ptr<Expr> value;
    PrintStmt(std::unique_ptr<Expr> value) : value(std::move(value)) {}
};

// input x;
struct InputStmt : public Stmt {
    std::string name;
    InputStmt(const std::string& name) : name(name) {}
};

// if (cond) { then } else { els }
struct IfStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> thenBranch;
    std::vector<std::unique_ptr<Stmt>> elseBranch;

    IfStmt(std::unique_ptr<Expr> condition,
           std::vector<std::unique_ptr<Stmt>> thenBranch,
           std::vector<std::unique_ptr<Stmt>> elseBranch)
        : condition(std::move(condition)),
          thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)) {}
};

// while (cond) { body }
struct WhileStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;

    WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> body)
        : condition(std::move(condition)), body(std::move(body)) {}
};
