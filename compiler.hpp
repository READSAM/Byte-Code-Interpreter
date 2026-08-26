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

    using ParseFn = void (Compiler::*)();

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
    void parsePrecedence(Precedence precedence);
    void grouping();
    void unary();
    void binary();
    void number();
    void string();
    void literal();

    const ParseRule& getRule(TokenType type) const;

    Chunk* compilingChunk{nullptr};
    Token current{};
    Token previous{};
    bool hadError{false};
    bool panicMode{false};
};