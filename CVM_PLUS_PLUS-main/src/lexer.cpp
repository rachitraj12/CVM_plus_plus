#include "lexer.h"
#include <cctype>
#include <stdexcept>
#include <unordered_map>

static const std::unordered_map<std::string, Tokentype> keywords = {
    {"let",   Tokentype::LET},
    {"if",    Tokentype::IF},
    {"else",  Tokentype::ELSE},
    {"while", Tokentype::WHILE},
    {"print", Tokentype::PRINT},
    {"input", Tokentype::INPUT},
    {"true",  Tokentype::BOOL_TRUE},
    {"false", Tokentype::BOOL_FALSE},
};

bool Lexer::isAtEnd() {
    return curr >= source.length();
}

char Lexer::look() {
    if (isAtEnd()) return '\0';
    return source[curr];
}

char Lexer::lookNext() {
    if (curr + 1 >= source.length()) return '\0';
    return source[curr + 1];
}

char Lexer::advance() {
    return source[curr++];
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source[curr] != expected) return false;
    curr++;
    return true;
}

std::vector<Token> Lexer::scanTokens() {
    std::vector<Token> tokens;
    while (!isAtEnd()) {
        char c = advance();

        // Skip whitespace
        if (c == ' ' || c == '\r' || c == '\t') continue;
        if (c == '\n') { line++; continue; }

        // Skip single-line comments
        if (c == '/' && look() == '/') {
            while (!isAtEnd() && look() != '\n') advance();
            continue;
        }

        switch (c) {
            case '+': tokens.push_back({Tokentype::PLUS,      "+", line}); break;
            case '-': tokens.push_back({Tokentype::MINUS,     "-", line}); break;
            case '*': tokens.push_back({Tokentype::STAR,      "*", line}); break;
            case '/': tokens.push_back({Tokentype::SLASH,     "/", line}); break;
            case '(': tokens.push_back({Tokentype::LPAREN,    "(", line}); break;
            case ')': tokens.push_back({Tokentype::RPAREN,    ")", line}); break;
            case '{': tokens.push_back({Tokentype::LBRACE,    "{", line}); break;
            case '}': tokens.push_back({Tokentype::RBRACE,    "}", line}); break;
            case ';': tokens.push_back({Tokentype::SEMICOLON, ";", line}); break;

            case '=':
                tokens.push_back(match('=')
                    ? Token{Tokentype::EQUAL_EQUAL,   "==", line}
                    : Token{Tokentype::EQUAL,          "=", line});
                break;
            case '!':
                tokens.push_back(match('=')
                    ? Token{Tokentype::BANG_EQUAL,    "!=", line}
                    : throw std::runtime_error("Unexpected '!' at line " + std::to_string(line)));
                break;
            case '<':
                tokens.push_back(match('=')
                    ? Token{Tokentype::LESS_EQUAL,    "<=", line}
                    : Token{Tokentype::LESS,           "<", line});
                break;
            case '>':
                tokens.push_back(match('=')
                    ? Token{Tokentype::GREATER_EQUAL, ">=", line}
                    : Token{Tokentype::GREATER,        ">", line});
                break;

            default:
                if (std::isdigit(c)) {
                    std::string num;
                    num += c;
                    while (std::isdigit(look())) num += advance();
                    tokens.push_back({Tokentype::NUMBER, num, line});

                } else if (std::isalpha(c) || c == '_') {
                    std::string ident;
                    ident += c;
                    while (std::isalnum(look()) || look() == '_') ident += advance();

                    auto it = keywords.find(ident);
                    if (it != keywords.end()) {
                        tokens.push_back({it->second, ident, line});
                    } else {
                        tokens.push_back({Tokentype::IDENTIFIER, ident, line});
                    }

                } else {
                    throw std::runtime_error(
                        std::string("Unexpected character '") + c + "' at line " + std::to_string(line));
                }
                break;
        }
    }

    tokens.push_back({Tokentype::EOF_TOK, "", line});
    return tokens;
}
