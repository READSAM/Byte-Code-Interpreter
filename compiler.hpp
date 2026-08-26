#pragma once

#include <string_view>
#include "chunk.hpp"
#include "object.hpp"
#include "scanner.hpp"

class Compiler {
public:
    enum class Precedence : uint8_t {
        NONE,
        ASSIGNMENT,
        OR,
        AND,
        EQUALITY,
        COMPARISON,
        TERM,
        FACTOR,
        UNARY,
        CALL,
        PRIMARY
    };

    using ParseFn = void (Compiler::*)(bool canAssign);

    struct ParseRule {
        ParseFn prefix;
        ParseFn infix;
        Precedence precedence;
    };

    Compiler() = default;
    bool compile(std::string_view source, Chunk& chunk);

private:
    void advance();
    void consume(TokenType type, const char* message);
    bool check(TokenType type);
    bool match(TokenType type);
    void errorAtCurrent(const char* message);
    void error(const char* message);
    void errorAt(const Token& token, const char* message);

    void emitByte(uint8_t byte);
    void emitByte(OpCode op);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitBytes(OpCode op1, OpCode op2);
    void emitBytes(OpCode op, uint8_t byte);
    void emitReturn();
    uint8_t makeConstant(Value value);
    void emitConstant(Value value);
    void endCompiler();

    void expression();
    void statement();
    void declaration();
    void varDeclaration();
    void expressionStatement();
    void printStatement();
    void synchronize();
    
    void parsePrecedence(Precedence precedence);
    uint8_t identifierConstant(Token* name);
    uint8_t parseVariable(const char* errorMessage);
    void defineVariable(uint8_t global);

    void grouping(bool canAssign);
    void unary(bool canAssign);
    void binary(bool canAssign);
    void number(bool canAssign);
    void string(bool canAssign);
    void literal(bool canAssign);
    void variable(bool canAssign);
    void namedVariable(Token name, bool canAssign);

    const ParseRule& getRule(TokenType type) const;

    Scanner scanner;
    Chunk* compilingChunk{nullptr};
    Token current{};
    Token previous{};
    bool hadError{false};
    bool panicMode{false};
};

bool compile(std::string_view source, Chunk& chunk);