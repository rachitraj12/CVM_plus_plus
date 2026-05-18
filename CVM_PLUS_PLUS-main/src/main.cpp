#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#endif
#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
#include "ast_printer.h"

// ─── Debug helpers ────────────────────────────────────────────────────────────

static std::string opcodeName(uint8_t op) {
    switch (op) {
        case OP_PUSH_INT:      return "OP_PUSH_INT";
        case OP_PUSH_BOOL:     return "OP_PUSH_BOOL";
        case OP_LOAD:          return "OP_LOAD";
        case OP_STORE:         return "OP_STORE";
        case OP_ADD:           return "OP_ADD";
        case OP_SUB:           return "OP_SUB";
        case OP_MUL:           return "OP_MUL";
        case OP_DIV:           return "OP_DIV";
        case OP_EQ:            return "OP_EQ";
        case OP_NEQ:           return "OP_NEQ";
        case OP_LT:            return "OP_LT";
        case OP_LTE:           return "OP_LTE";
        case OP_GT:            return "OP_GT";
        case OP_GTE:           return "OP_GTE";
        case OP_NEGATE:        return "OP_NEGATE";
        case OP_JUMP:          return "OP_JUMP";
        case OP_JUMP_IF_FALSE: return "OP_JUMP_IF_FALSE";
        case OP_PRINT:         return "OP_PRINT";
        case OP_INPUT:         return "OP_INPUT";
        case OP_HALT:          return "OP_HALT";
        default:               return "UNKNOWN";
    }
}

static int32_t readInt32At(const std::vector<uint8_t>& code, size_t i) {
    int32_t v = 0;
    v |= (int32_t)code[i]   <<  0;
    v |= (int32_t)code[i+1] <<  8;
    v |= (int32_t)code[i+2] << 16;
    v |= (int32_t)code[i+3] << 24;
    return v;
}

static void disassemble(const std::vector<uint8_t>& code) {
    std::cout << "\n=======================================\n";
    std::cout << "          BYTECODE DISASSEMBLY         \n";
    std::cout << "=======================================\n";
    size_t i = 0;
    while (i < code.size()) {
        uint8_t op = code[i];
        std::cout << "  [" << std::setw(4) << i << "]  " << opcodeName(op);
        i++;
        switch (op) {
            case OP_PUSH_INT:
            case OP_LOAD:
            case OP_STORE:
            case OP_JUMP:
            case OP_JUMP_IF_FALSE:
            case OP_INPUT: {
                int32_t val = readInt32At(code, i);
                std::cout << "  " << val;
                i += 4;
                break;
            }
            case OP_PUSH_BOOL: {
                std::cout << "  " << (code[i] ? "true" : "false");
                i++;
                break;
            }
            default: break;
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

// ─── Run pipeline ─────────────────────────────────────────────────────────────

static void runSource(const std::string& source, bool debug) {
    // 1. Lex
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.scanTokens();

    if (debug) {
        std::cout << "\n=======================================\n";
        std::cout << "              TOKEN STREAM             \n";
        std::cout << "=======================================\n";
        for (auto& t : tokens) {
            std::string typeName;
            switch (t.type) {
                case Tokentype::NUMBER:       typeName = "NUMBER";      break;
                case Tokentype::BOOL_TRUE:    typeName = "TRUE";        break;
                case Tokentype::BOOL_FALSE:   typeName = "FALSE";       break;
                case Tokentype::IDENTIFIER:   typeName = "IDENTIFIER";  break;
                case Tokentype::PLUS:         typeName = "PLUS";        break;
                case Tokentype::MINUS:        typeName = "MINUS";       break;
                case Tokentype::STAR:         typeName = "STAR";        break;
                case Tokentype::SLASH:        typeName = "SLASH";       break;
                case Tokentype::EQUAL:        typeName = "EQUAL";       break;
                case Tokentype::EQUAL_EQUAL:  typeName = "EQUAL_EQUAL"; break;
                case Tokentype::BANG_EQUAL:   typeName = "BANG_EQUAL";  break;
                case Tokentype::LESS:         typeName = "LESS";        break;
                case Tokentype::LESS_EQUAL:   typeName = "LESS_EQUAL";  break;
                case Tokentype::GREATER:      typeName = "GREATER";     break;
                case Tokentype::GREATER_EQUAL:typeName = "GTE";         break;
                case Tokentype::LET:          typeName = "LET";         break;
                case Tokentype::IF:           typeName = "IF";          break;
                case Tokentype::ELSE:         typeName = "ELSE";        break;
                case Tokentype::WHILE:        typeName = "WHILE";       break;
                case Tokentype::PRINT:        typeName = "PRINT";       break;
                case Tokentype::INPUT:        typeName = "INPUT";       break;
                case Tokentype::LPAREN:       typeName = "LPAREN";      break;
                case Tokentype::RPAREN:       typeName = "RPAREN";      break;
                case Tokentype::LBRACE:       typeName = "LBRACE";      break;
                case Tokentype::RBRACE:       typeName = "RBRACE";      break;
                case Tokentype::SEMICOLON:    typeName = "SEMICOLON";   break;
                case Tokentype::EOF_TOK:      typeName = "EOF";         break;
                default:                      typeName = "UNKNOWN";     break;
            }
            std::cout << "  [" << typeName << "]  '" << t.lexeme << "'\n";
        }
    }

    // 2. Parse
    Parser parser(tokens);
    auto ast = parser.parse();

    if (debug) {
        ASTPrinter printer;
        // The ASTPrinter class should have its own banner update to match.
        printer.print(ast); 
    }

    // 3. Compile
    Compiler compiler;
    Bytecode bc = compiler.compile(ast);

    if (debug) {
        disassemble(bc.code);
    }

    // 4. Execute
    if (debug) {
        std::cout << "=======================================\n";
        std::cout << "              EXECUTION                \n";
        std::cout << "=======================================\n";
    }
    VM vm;
    vm.load(bc.code, bc.numVars);
    vm.run();
}

// ─── REPL ─────────────────────────────────────────────────────────────────────

static void repl(bool debug) {
    std::cout << "====================================================\n";
    std::cout << "          CVM++ Interactive REPL  v1.0              \n";
    std::cout << "  Type 'exit' or 'quit' to leave.                   \n";
    std::cout << "  End multi-line input with a blank line.           \n";
    std::cout << "====================================================\n\n";

    while (true) {
        std::cout << "cvm>> ";
        std::string line, source;
        while (std::getline(std::cin, line)) {
            if (line == "exit" || line == "quit") {
                std::cout << "Goodbye!\n";
                return;
            }
            if (line.empty()) break;
            source += line + "\n";
            // If line ends with ';' or '}', we might be done — keep reading
            // Simple heuristic: stop on blank line
        }
        if (source.empty()) continue;

        try {
            runSource(source, debug);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}

// ─── Entry point ──────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    bool debug = false;
    std::string filepath;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--debug" || arg == "-d") debug = true;
        else filepath = arg;
    }

    if (filepath.empty()) {
        repl(debug);
    } else {
        std::ifstream file(filepath);
        if (!file) {
            std::cerr << "Error: Cannot open file '" << filepath << "'\n";
            return 1;
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        try {
            runSource(ss.str(), debug);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    return 0;
}