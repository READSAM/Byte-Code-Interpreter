#pragma once

#include <string_view>

#include "common.hpp"

enum class TokenType : uint8_t {
    // Single-character tokens
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,

    // One or two character tokens
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,

    // Literals
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // Keywords
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

    TOKEN_ERROR, TOKEN_EOF,

    TOKEN_COUNT // Helper for table indexing
};

struct Token {
    TokenType type;
    const char* start;
    std::size_t length;
    int line;
};

class Scanner {
public:
    Scanner() = default;
    explicit Scanner(const char* source);

    void init(const char* source);
    Token scanToken();

private:
    const char* start = nullptr;
    const char* current = nullptr;
    int line = 1;

    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);

    Token makeToken(TokenType type) const;
    Token errorToken(const char* message) const;
    void skipWhitespace();
    TokenType checkKeyword(int startOffset, int length, const char* rest, TokenType type) const;
    TokenType identifierType() const;
    Token identifier();
    Token number();
    Token string();
};