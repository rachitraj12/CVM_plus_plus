#pragma once
#include <vector>
#include <string>

enum class Tokentype {
    // Literals
    NUMBER,
    BOOL_TRUE,
    BOOL_FALSE,
    IDENTIFIER,

    // Arithmetic
    PLUS,
    MINUS,
    STAR,
    SLASH,

    // Comparison
    EQUAL_EQUAL,   // ==
    BANG_EQUAL,    // !=
    LESS,          // <
    LESS_EQUAL,    // <=
    GREATER,       // >
    GREATER_EQUAL, // >=

    // Assignment
    EQUAL,         // =

    // Delimiters
    LPAREN,   // (
    RPAREN,   // )
    LBRACE,   // {
    RBRACE,   // }
    SEMICOLON,

    // Keywords
    LET,
    IF,
    ELSE,
    WHILE,
    PRINT,
    INPUT,

    EOF_TOK
};

struct Token {
    Tokentype type;
    std::string lexeme;
    int line;
};

class Lexer {
private:
    std::string source;
    size_t curr = 0;
    int line = 1;

    bool isAtEnd();
    char look();
    char lookNext();
    char advance();
    bool match(char expected);

public:
    Lexer(const std::string& source) : source(source) {}
    std::vector<Token> scanTokens();
};
