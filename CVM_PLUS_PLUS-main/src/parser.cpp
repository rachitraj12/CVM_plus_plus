#include "parser.h"
#include <stdexcept>

// ─── Navigation helpers ───────────────────────────────────────────────────────

Token Parser::look() {
    return tokens[curr];
}

Token Parser::advance() {
    if (!isAtEnd()) curr++;
    return tokens[curr - 1];
}

bool Parser::isAtEnd() {
    return look().type == Tokentype::EOF_TOK;
}

bool Parser::match(Tokentype type) {
    if (look().type == type) { advance(); return true; }
    return false;
}

Token Parser::expect(Tokentype type, const std::string& msg) {
    if (look().type == type) return advance();
    throw std::runtime_error("Parse error at line " + std::to_string(look().line) + ": " + msg);
}

// ─── Top level ───────────────────────────────────────────────────────────────

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (!isAtEnd()) {
        stmts.push_back(parseStmt());
    }
    return stmts;
}

// ─── Statements ──────────────────────────────────────────────────────────────

std::unique_ptr<Stmt> Parser::parseStmt() {
    if (look().type == Tokentype::LET)   return parseLetStmt();
    if (look().type == Tokentype::PRINT) return parsePrintStmt();
    if (look().type == Tokentype::INPUT) return parseInputStmt();
    if (look().type == Tokentype::IF)    return parseIfStmt();
    if (look().type == Tokentype::WHILE) return parseWhileStmt();
    return parseAssignOrExpr();
}

std::unique_ptr<Stmt> Parser::parseLetStmt() {
    advance(); // consume 'let'
    Token name = expect(Tokentype::IDENTIFIER, "Expected variable name after 'let'");
    expect(Tokentype::EQUAL, "Expected '=' after variable name");
    auto init = parseExpr();
    expect(Tokentype::SEMICOLON, "Expected ';' after variable declaration");
    return std::make_unique<LetStmt>(name.lexeme, std::move(init));
}

std::unique_ptr<Stmt> Parser::parsePrintStmt() {
    advance(); // consume 'print'
    auto val = parseExpr();
    expect(Tokentype::SEMICOLON, "Expected ';' after print statement");
    return std::make_unique<PrintStmt>(std::move(val));
}

std::unique_ptr<Stmt> Parser::parseInputStmt() {
    advance(); // consume 'input'
    Token name = expect(Tokentype::IDENTIFIER, "Expected variable name after 'input'");
    expect(Tokentype::SEMICOLON, "Expected ';' after input statement");
    return std::make_unique<InputStmt>(name.lexeme);
}

std::unique_ptr<Stmt> Parser::parseIfStmt() {
    advance(); // consume 'if'
    expect(Tokentype::LPAREN, "Expected '(' after 'if'");
    auto cond = parseExpr();
    expect(Tokentype::RPAREN, "Expected ')' after condition");
    auto thenBranch = parseBlock();

    std::vector<std::unique_ptr<Stmt>> elseBranch;
    if (look().type == Tokentype::ELSE) {
        advance();
        elseBranch = parseBlock();
    }

    return std::make_unique<IfStmt>(std::move(cond), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::parseWhileStmt() {
    advance(); // consume 'while'
    expect(Tokentype::LPAREN, "Expected '(' after 'while'");
    auto cond = parseExpr();
    expect(Tokentype::RPAREN, "Expected ')' after condition");
    auto body = parseBlock();
    return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseAssignOrExpr() {
    // assignment: IDENTIFIER = expr ;
    if (look().type == Tokentype::IDENTIFIER && curr + 1 < tokens.size()
        && tokens[curr + 1].type == Tokentype::EQUAL) {
        Token name = advance();
        advance(); // consume '='
        auto val = parseExpr();
        expect(Tokentype::SEMICOLON, "Expected ';' after assignment");
        return std::make_unique<AssignStmt>(name.lexeme, std::move(val));
    }
    // otherwise it's an expression statement (rare, but handle for completeness)
    throw std::runtime_error("Parse error at line " + std::to_string(look().line)
        + ": Unexpected token '" + look().lexeme + "'");
}

std::vector<std::unique_ptr<Stmt>> Parser::parseBlock() {
    expect(Tokentype::LBRACE, "Expected '{'");
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (!isAtEnd() && look().type != Tokentype::RBRACE) {
        stmts.push_back(parseStmt());
    }
    expect(Tokentype::RBRACE, "Expected '}'");
    return stmts;
}

// ─── Expressions (precedence climbing) ───────────────────────────────────────

std::unique_ptr<Expr> Parser::parseExpr() {
    return parseComparison();
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto left = parseAddSub();

    while (look().type == Tokentype::EQUAL_EQUAL   ||
           look().type == Tokentype::BANG_EQUAL    ||
           look().type == Tokentype::LESS          ||
           look().type == Tokentype::LESS_EQUAL    ||
           look().type == Tokentype::GREATER       ||
           look().type == Tokentype::GREATER_EQUAL) {
        Token op = advance();
        auto right = parseAddSub();
        left = std::make_unique<BinaryExpr>(std::move(left), op.type, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseAddSub() {
    auto left = parseMulDiv();

    while (look().type == Tokentype::PLUS || look().type == Tokentype::MINUS) {
        Token op = advance();
        auto right = parseMulDiv();
        left = std::make_unique<BinaryExpr>(std::move(left), op.type, std::move(right));
    }
    return left;
}



std::unique_ptr<Expr> Parser::parseMulDiv() {
    auto left = parseUnary(); // <-- Change parsePrimary() to parseUnary()

    while (look().type == Tokentype::STAR || look().type == Tokentype::SLASH) {
        Token op = advance();
        auto right = parseUnary(); // <-- Change here too
        left = std::make_unique<BinaryExpr>(std::move(left), op.type, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (match(Tokentype::MINUS)) {
        Token op = tokens[curr - 1];
        auto right = parseUnary(); // Recursively call for things like --5
        return std::make_unique<UnaryExpr>(op.type, std::move(right));
    }
    return parsePrimary();
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (match(Tokentype::NUMBER)) {
        int val = std::stoi(tokens[curr - 1].lexeme);
        return std::make_unique<NumberExpr>(val);
    }
    if (match(Tokentype::BOOL_TRUE))  return std::make_unique<BoolExpr>(true);
    if (match(Tokentype::BOOL_FALSE)) return std::make_unique<BoolExpr>(false);

    if (match(Tokentype::IDENTIFIER)) {
        return std::make_unique<VarExpr>(tokens[curr - 1].lexeme);
    }

    if (match(Tokentype::LPAREN)) {
        auto expr = parseExpr();
        expect(Tokentype::RPAREN, "Expected ')' after expression");
        return expr;
    }

    throw std::runtime_error("Parse error at line " + std::to_string(look().line)
        + ": Unexpected token '" + look().lexeme + "'");
}
