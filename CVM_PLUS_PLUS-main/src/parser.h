#pragma once
#include <vector>
#include <memory>
#include "lexer.h"
#include "ast.h"

class Parser {
private:
    std::vector<Token> tokens;
    size_t curr = 0;

    Token look();
    Token advance();
    bool isAtEnd();
    bool match(Tokentype type);
    Token expect(Tokentype type, const std::string& msg);

    // Expression parsing (precedence climbing)
    std::unique_ptr<Expr> parseExpr();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseAddSub();
    std::unique_ptr<Expr> parseMulDiv();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePrimary();

    // Statement parsing
    std::unique_ptr<Stmt> parseStmt();
    std::unique_ptr<Stmt> parseLetStmt();
    std::unique_ptr<Stmt> parsePrintStmt();
    std::unique_ptr<Stmt> parseInputStmt();
    std::unique_ptr<Stmt> parseIfStmt();
    std::unique_ptr<Stmt> parseWhileStmt();
    std::unique_ptr<Stmt> parseAssignOrExpr();
    std::vector<std::unique_ptr<Stmt>> parseBlock();

public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens) {}
    std::vector<std::unique_ptr<Stmt>> parse();
};
