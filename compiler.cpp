#include "compiler.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "common.hpp"

#ifdef DEBUG_PRINT_CODE
#include "debug.hpp"
#endif

void Compiler::errorAt(const Token& token, const char* message) {
    if (panicMode) return;
    panicMode = true;
    std::fprintf(stderr, "[line %d] Error", token.line);

    if (token.type == TokenType::TOKEN_EOF) {
        std::fprintf(stderr, " at end");
    } else if (token.type != TokenType::TOKEN_ERROR) {
        std::fprintf(stderr, " at '%.*s'", static_cast<int>(token.length), token.start);
    }

    std::fprintf(stderr, ": %s\n", message);
    hadError = true;
}

void Compiler::error(const char* message) {
    errorAt(previous, message);
}

void Compiler::errorAtCurrent(const char* message) {
    errorAt(current, message);
}

void Compiler::advance() {
    previous = current;

    for (;;) {
        current = scanToken();
        if (current.type != TokenType::TOKEN_ERROR) break;

        errorAtCurrent(current.start);
    }
}

void Compiler::consume(TokenType type, const char* message) {
    if (current.type == type) {
        advance();
        return;
    }
    errorAtCurrent(message);
}

void Compiler::emitByte(uint8_t byte) {
    compilingChunk->write(byte, previous.line);
}

void Compiler::emitByte(OpCode op) {
    emitByte(static_cast<uint8_t>(op));
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::emitBytes(OpCode op1, OpCode op2) {
    emitByte(op1);
    emitByte(op2);
}

void Compiler::emitBytes(OpCode op, uint8_t byte) {
    emitByte(op);
    emitByte(byte);
}

void Compiler::emitReturn() {
    emitByte(OpCode::OP_RETURN);
}

uint8_t Compiler::makeConstant(Value value) {
    int constant = compilingChunk->addConstant(value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }
    return static_cast<uint8_t>(constant);
}

void Compiler::emitConstant(Value value) {
    emitBytes(OpCode::OP_CONSTANT, makeConstant(value));
}

void Compiler::endCompiler() {
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!hadError) {
        disassembleChunk(*compilingChunk, "code");
    }
#endif
}

void Compiler::binary() {
    TokenType operatorType = previous.type;
    const ParseRule& rule = getRule(operatorType);
    parsePrecedence(static_cast<Precedence>(static_cast<uint8_t>(rule.precedence) + 1));

    switch (operatorType) {
        case TokenType::TOKEN_BANG_EQUAL:    emitBytes(OpCode::OP_EQUAL, OpCode::OP_NOT); break;
        case TokenType::TOKEN_EQUAL_EQUAL:   emitByte(OpCode::OP_EQUAL); break;
        case TokenType::TOKEN_GREATER:       emitByte(OpCode::OP_GREATER); break;
        case TokenType::TOKEN_GREATER_EQUAL: emitBytes(OpCode::OP_LESS, OpCode::OP_NOT); break;
        case TokenType::TOKEN_LESS:          emitByte(OpCode::OP_LESS); break;
        case TokenType::TOKEN_LESS_EQUAL:    emitBytes(OpCode::OP_GREATER, OpCode::OP_NOT); break;
        case TokenType::TOKEN_PLUS:          emitByte(OpCode::OP_ADD); break;
        case TokenType::TOKEN_MINUS:         emitByte(OpCode::OP_SUBTRACT); break;
        case TokenType::TOKEN_STAR:          emitByte(OpCode::OP_MULTIPLY); break;
        case TokenType::TOKEN_SLASH:         emitByte(OpCode::OP_DIVIDE); break;
        default: return;
    }
}

void Compiler::literal() {
    switch (previous.type) {
        case TokenType::TOKEN_FALSE: emitByte(OpCode::OP_FALSE); break;
        case TokenType::TOKEN_NIL:   emitByte(OpCode::OP_NIL); break;
        case TokenType::TOKEN_TRUE:  emitByte(OpCode::OP_TRUE); break;
        default: return;
    }
}

void Compiler::grouping() {
    expression();
    consume(TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::number() {
    double value = std::strtod(previous.start, nullptr);
    emitConstant(NUMBER_VAL(value));
}

void Compiler::string() {
    emitConstant(OBJ_VAL(copyString(previous.start + 1, static_cast<int>(previous.length - 2))));
}

void Compiler::unary() {
    TokenType operatorType = previous.type;

    parsePrecedence(Precedence::UNARY);

    switch (operatorType) {
        case TokenType::TOKEN_BANG:  emitByte(OpCode::OP_NOT); break;
        case TokenType::TOKEN_MINUS: emitByte(OpCode::OP_NEGATE); break;
        default: return;
    }
}

const Compiler::ParseRule& Compiler::getRule(TokenType type) const {
    static const auto rules = []() {
        std::array<ParseRule, static_cast<size_t>(TokenType::TOKEN_COUNT)> table{};
        
        auto set = [&](TokenType t, ParseFn prefix, ParseFn infix, Precedence prec) {
            table[static_cast<size_t>(t)] = {prefix, infix, prec};
        };

        set(TokenType::TOKEN_LEFT_PAREN,    &Compiler::grouping, nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_RIGHT_PAREN,   nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_LEFT_BRACE,    nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_RIGHT_BRACE,   nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_COMMA,         nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_DOT,           nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_MINUS,         &Compiler::unary,    &Compiler::binary,  Precedence::TERM);
        set(TokenType::TOKEN_PLUS,          nullptr,             &Compiler::binary,  Precedence::TERM);
        set(TokenType::TOKEN_SEMICOLON,     nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_SLASH,         nullptr,             &Compiler::binary,  Precedence::FACTOR);
        set(TokenType::TOKEN_STAR,          nullptr,             &Compiler::binary,  Precedence::FACTOR);
        set(TokenType::TOKEN_BANG,          &Compiler::unary,    nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_BANG_EQUAL,    nullptr,             &Compiler::binary,  Precedence::EQUALITY);
        set(TokenType::TOKEN_EQUAL,         nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_EQUAL_EQUAL,   nullptr,             &Compiler::binary,  Precedence::EQUALITY);
        set(TokenType::TOKEN_GREATER,       nullptr,             &Compiler::binary,  Precedence::COMPARISON);
        set(TokenType::TOKEN_GREATER_EQUAL, nullptr,             &Compiler::binary,  Precedence::COMPARISON);
        set(TokenType::TOKEN_LESS,          nullptr,             &Compiler::binary,  Precedence::COMPARISON);
        set(TokenType::TOKEN_LESS_EQUAL,    nullptr,             &Compiler::binary,  Precedence::COMPARISON);
        set(TokenType::TOKEN_IDENTIFIER,    nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_STRING,        &Compiler::string,   nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_NUMBER,        &Compiler::number,   nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_AND,           nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_CLASS,         nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_ELSE,          nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_FALSE,         &Compiler::literal,  nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_FOR,           nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_FUN,           nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_IF,            nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_NIL,           &Compiler::literal,  nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_OR,            nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_PRINT,         nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_RETURN,        nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_SUPER,         nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_THIS,          nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_TRUE,          &Compiler::literal,  nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_VAR,           nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_WHILE,         nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_ERROR,         nullptr,             nullptr,            Precedence::NONE);
        set(TokenType::TOKEN_EOF,           nullptr,             nullptr,            Precedence::NONE);

        return table;
    }();

    return rules[static_cast<size_t>(type)];
}

void Compiler::parsePrecedence(Precedence precedence) {
    advance();
    ParseFn prefixRule = getRule(previous.type).prefix;
    if (prefixRule == nullptr) {
        error("Expect expression.");
        return;
    }

    (this->*prefixRule)();

    while (precedence <= getRule(current.type).precedence) {
        advance();
        ParseFn infixRule = getRule(previous.type).infix;
        (this->*infixRule)();
    }
}

void Compiler::expression() {
    parsePrecedence(Precedence::ASSIGNMENT);
}

bool Compiler::compile(std::string_view source, Chunk& chunk) {
    initScanner(source.data());
    compilingChunk = &chunk;

    hadError = false;
    panicMode = false;

    advance();
    expression();
    consume(TokenType::TOKEN_EOF, "Expect end of expression.");
    endCompiler();

    return !hadError;
}

bool compile(std::string_view source, Chunk& chunk) {
    Compiler compiler;
    return compiler.compile(source, chunk);
}