#pragma once
#include <string>
#include "ast.h"

// ─── AST Pretty Printer ───────────────────────────────────────────────────────
// Walks the AST and prints a human-readable tree to stdout.
// Used by the --debug flag in the pipeline.

class ASTPrinter {
private:
    void printNode(const std::string& prefix, bool isLast, const std::string& label) const;
    std::string opName(Tokentype op) const;
    std::string exprSummary(const Expr* expr) const;

    void printExpr(const Expr* expr, const std::string& prefix, bool isLast);
    void printLabeledExpr(const std::string& prefix, bool isLast, const std::string& label, const Expr* expr);
    void printStmt(const Stmt* stmt, const std::string& prefix, bool isLast);
    void printStmtRoot(const Stmt* stmt);

public:
    void print(const std::vector<std::unique_ptr<Stmt>>& stmts);
};
