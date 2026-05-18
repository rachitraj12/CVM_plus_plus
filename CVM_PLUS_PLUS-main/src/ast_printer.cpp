#include "ast_printer.h"
#include <iostream>

namespace {
    const std::string BRANCH = "├── ";
    const std::string LAST   = "└── ";
    const std::string PIPE   = "│   ";
    const std::string BLANK  = "    ";
}

void ASTPrinter::printNode(const std::string& prefix, bool isLast, const std::string& label) const {
    std::cout << prefix << (isLast ? LAST : BRANCH) << label << "\n";
}

std::string ASTPrinter::opName(Tokentype op) const {
    switch (op) {
        case Tokentype::PLUS:          return "+";
        case Tokentype::MINUS:         return "-";
        case Tokentype::STAR:          return "*";
        case Tokentype::SLASH:         return "/";
        case Tokentype::EQUAL_EQUAL:   return "==";
        case Tokentype::BANG_EQUAL:    return "!=";
        case Tokentype::LESS:          return "<";
        case Tokentype::LESS_EQUAL:    return "<=";
        case Tokentype::GREATER:       return ">";
        case Tokentype::GREATER_EQUAL: return ">=";
        default:                       return "?";
    }
}

std::string ASTPrinter::exprSummary(const Expr* expr) const {
    if (auto* n = dynamic_cast<const NumberExpr*>(expr)) {
        return "NumberExpr(" + std::to_string(n->val) + ")";

    } else if (auto* b = dynamic_cast<const BoolExpr*>(expr)) {
        return std::string("BoolExpr(") + (b->val ? "true" : "false") + ")";

    } else if (auto* v = dynamic_cast<const VarExpr*>(expr)) {
        return "VarExpr(\"" + v->name + "\")";

    } else if (auto* bin = dynamic_cast<const BinaryExpr*>(expr)) {
        return "BinaryExpr(" + opName(bin->op) + ")";

    } else if (auto* un = dynamic_cast<const UnaryExpr*>(expr)) {
        return "UnaryExpr(" + opName(un->op) + ")";
    }

    return "Expr";
}

void ASTPrinter::printExpr(const Expr* expr, const std::string& prefix, bool isLast) {
    printNode(prefix, isLast, exprSummary(expr));

    if (auto* bin = dynamic_cast<const BinaryExpr*>(expr)) {
        std::string childPrefix = prefix + (isLast ? BLANK : PIPE);
        printExpr(bin->left.get(), childPrefix, false);
        printExpr(bin->right.get(), childPrefix, true);

    } else if (auto* un = dynamic_cast<const UnaryExpr*>(expr)) {
        std::string childPrefix = prefix + (isLast ? BLANK : PIPE);
        printExpr(un->right.get(), childPrefix, true);
    }
}

void ASTPrinter::printLabeledExpr(const std::string& prefix, bool isLast, const std::string& label, const Expr* expr) {
    printNode(prefix, isLast, label + exprSummary(expr));

    if (auto* bin = dynamic_cast<const BinaryExpr*>(expr)) {
        std::string childPrefix = prefix + (isLast ? BLANK : PIPE);
        printExpr(bin->left.get(), childPrefix, false);
        printExpr(bin->right.get(), childPrefix, true);

    } else if (auto* un = dynamic_cast<const UnaryExpr*>(expr)) {
        std::string childPrefix = prefix + (isLast ? BLANK : PIPE);
        printExpr(un->right.get(), childPrefix, true);
    }
}

void ASTPrinter::printStmt(const Stmt* stmt, const std::string& prefix, bool isLast) {
    if (auto* s = dynamic_cast<const LetStmt*>(stmt)) {
        printNode(prefix, isLast, "LetStmt(\"" + s->name + "\")");
        printLabeledExpr(prefix + (isLast ? BLANK : PIPE), true, "init: ", s->init.get());

    } else if (auto* s = dynamic_cast<const AssignStmt*>(stmt)) {
        printNode(prefix, isLast, "AssignStmt(\"" + s->name + "\")");
        printLabeledExpr(prefix + (isLast ? BLANK : PIPE), true, "value: ", s->value.get());

    } else if (auto* s = dynamic_cast<const PrintStmt*>(stmt)) {
        printNode(prefix, isLast, "PrintStmt");
        printExpr(s->value.get(), prefix + (isLast ? BLANK : PIPE), true);

    } else if (auto* s = dynamic_cast<const InputStmt*>(stmt)) {
        printNode(prefix, isLast, "InputStmt(\"" + s->name + "\")");

    } else if (auto* s = dynamic_cast<const IfStmt*>(stmt)) {
        printNode(prefix, isLast, "IfStmt");
        std::string childPrefix = prefix + (isLast ? BLANK : PIPE);
        bool hasElse = !s->elseBranch.empty();

        printLabeledExpr(childPrefix, false, "condition: ", s->condition.get());
        printNode(childPrefix, hasElse ? false : true, "then:");
        std::string thenPrefix = childPrefix + (hasElse ? PIPE : BLANK);
        for (std::size_t i = 0; i < s->thenBranch.size(); ++i) {
            printStmt(s->thenBranch[i].get(), thenPrefix, i + 1 == s->thenBranch.size());
        }
        if (!s->elseBranch.empty()) {
            printNode(childPrefix, true, "else:");
            std::string elsePrefix = childPrefix + BLANK;
            for (std::size_t i = 0; i < s->elseBranch.size(); ++i) {
                printStmt(s->elseBranch[i].get(), elsePrefix, i + 1 == s->elseBranch.size());
            }
        }

    } else if (auto* s = dynamic_cast<const WhileStmt*>(stmt)) {
        printNode(prefix, isLast, "WhileStmt");
        std::string childPrefix = prefix + (isLast ? BLANK : PIPE);
        printLabeledExpr(childPrefix, false, "condition: ", s->condition.get());
        printNode(childPrefix, true, "body:");
        std::string bodyPrefix = childPrefix + BLANK;
        for (std::size_t i = 0; i < s->body.size(); ++i) {
            printStmt(s->body[i].get(), bodyPrefix, i + 1 == s->body.size());
        }
    }
}

void ASTPrinter::printStmtRoot(const Stmt* stmt) {
    if (auto* s = dynamic_cast<const LetStmt*>(stmt)) {
        std::cout << "LetStmt(\"" << s->name << "\")\n";
        printLabeledExpr("", true, "init: ", s->init.get());

    } else if (auto* s = dynamic_cast<const AssignStmt*>(stmt)) {
        std::cout << "AssignStmt(\"" << s->name << "\")\n";
        printLabeledExpr("", true, "value: ", s->value.get());

    } else if (auto* s = dynamic_cast<const PrintStmt*>(stmt)) {
        std::cout << "PrintStmt\n";
        printExpr(s->value.get(), "", true);

    } else if (auto* s = dynamic_cast<const InputStmt*>(stmt)) {
        std::cout << "InputStmt(\"" << s->name << "\")\n";

    } else if (auto* s = dynamic_cast<const IfStmt*>(stmt)) {
        std::cout << "IfStmt\n";
        bool hasElse = !s->elseBranch.empty();

        printLabeledExpr("", false, "condition: ", s->condition.get());
        printNode("", hasElse ? false : true, "then:");
        std::string thenPrefix = hasElse ? PIPE : BLANK;
        for (std::size_t i = 0; i < s->thenBranch.size(); ++i) {
            printStmt(s->thenBranch[i].get(), thenPrefix, i + 1 == s->thenBranch.size());
        }
        if (hasElse) {
            printNode("", true, "else:");
            std::string elsePrefix = BLANK;
            for (std::size_t i = 0; i < s->elseBranch.size(); ++i) {
                printStmt(s->elseBranch[i].get(), elsePrefix, i + 1 == s->elseBranch.size());
            }
        }

    } else if (auto* s = dynamic_cast<const WhileStmt*>(stmt)) {
        std::cout << "WhileStmt\n";
        printLabeledExpr("", false, "condition: ", s->condition.get());
        printNode("", true, "body:");
        std::string bodyPrefix = BLANK;
        for (std::size_t i = 0; i < s->body.size(); ++i) {
            printStmt(s->body[i].get(), bodyPrefix, i + 1 == s->body.size());
        }
    }
}

void ASTPrinter::print(const std::vector<std::unique_ptr<Stmt>>& stmts) {
    std::cout << "\n=======================================\n";
    std::cout << "          ABSTRACT SYNTAX TREE         \n";
    std::cout << "=======================================\n";
    for (auto& stmt : stmts) {
        printStmtRoot(stmt.get());
    }
    std::cout << "\n";
}