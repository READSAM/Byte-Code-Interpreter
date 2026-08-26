#include "scanner.hpp"

#include <cstring>

static Scanner globalScanner;

// void initScanner(const char* source) {
//     globalScanner.init(source);
// }

// Token scanToken() {
//     return globalScanner.scanToken();
// }

Scanner::Scanner(const char* source) {
    init(source);
}

void Scanner::init(const char* source) {
    start = source;
    current = source;
    line = 1;
}

bool Scanner::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

bool Scanner::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Scanner::isAtEnd() const {
    return *current == '\0';
}

char Scanner::advance() {
    current++;
    return current[-1];
}

char Scanner::peek() const {
    return *current;
}

char Scanner::peekNext() const {
    if (isAtEnd()) return '\0';
    return current[1];
}

bool Scanner::match(char expected) {
    if (isAtEnd() || *current != expected) return false;
    current++;
    return true;
}

Token Scanner::makeToken(TokenType type) const {
    return Token{
        .type = type,
        .start = start,
        .length = static_cast<std::size_t>(current - start),
        .line = line
    };
}

Token Scanner::errorToken(const char* message) const {
    return Token{
        .type = TokenType::TOKEN_ERROR,
        .start = message,
        .length = std::strlen(message),
        .line = line
    };
}

void Scanner::skipWhitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                line++;
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    while (peek() != '\n' && !isAtEnd()) {
                        advance();
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

TokenType Scanner::checkKeyword(int startOffset, int length, const char* rest, TokenType type) const {
    if (current - start == startOffset + length &&
        std::memcmp(start + startOffset, rest, length) == 0) {
        return type;
    }
    return TokenType::TOKEN_IDENTIFIER;
}

TokenType Scanner::identifierType() const {
    switch (start[0]) {
        case 'a': return checkKeyword(1, 2, "nd", TokenType::TOKEN_AND);
        case 'c': return checkKeyword(1, 4, "lass", TokenType::TOKEN_CLASS);
        case 'e': return checkKeyword(1, 3, "lse", TokenType::TOKEN_ELSE);
        case 'f':
            if (current - start > 1) {
                switch (start[1]) {
                    case 'a': return checkKeyword(2, 3, "lse", TokenType::TOKEN_FALSE);
                    case 'o': return checkKeyword(2, 1, "r", TokenType::TOKEN_FOR);
                    case 'u': return checkKeyword(2, 1, "n", TokenType::TOKEN_FUN);
                }
            }
            break;
        case 'i': return checkKeyword(1, 1, "f", TokenType::TOKEN_IF);
        case 'n': return checkKeyword(1, 2, "il", TokenType::TOKEN_NIL);
        case 'o': return checkKeyword(1, 1, "r", TokenType::TOKEN_OR);
        case 'p': return checkKeyword(1, 4, "rint", TokenType::TOKEN_PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TokenType::TOKEN_RETURN);
        case 's': return checkKeyword(1, 4, "uper", TokenType::TOKEN_SUPER);
        case 't':
            if (current - start > 1) {
                switch (start[1]) {
                    case 'h': return checkKeyword(2, 2, "is", TokenType::TOKEN_THIS);
                    case 'r': return checkKeyword(2, 2, "ue", TokenType::TOKEN_TRUE);
                }
            }
            break;
        case 'v': return checkKeyword(1, 2, "ar", TokenType::TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TokenType::TOKEN_WHILE);
    }
    return TokenType::TOKEN_IDENTIFIER;
}

Token Scanner::identifier() {
    while (isAlpha(peek()) || isDigit(peek())) advance();
    return makeToken(identifierType());
}

Token Scanner::number() {
    while (isDigit(peek())) advance();

    //fractional component check
    if (peek() == '.' && isDigit(peekNext())) {
        advance(); // consume '.'
        while (isDigit(peek())) advance();
    }

    return makeToken(TokenType::TOKEN_NUMBER);
}

Token Scanner::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string.");

    advance(); // consume closing quote
    return makeToken(TokenType::TOKEN_STRING);
}

Token Scanner::scanToken() {
    skipWhitespace();
    start = current;

    if (isAtEnd()) return makeToken(TokenType::TOKEN_EOF);

    char c = advance();

    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

    switch (c) {
        case '(': return makeToken(TokenType::TOKEN_LEFT_PAREN);
        case ')': return makeToken(TokenType::TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TokenType::TOKEN_LEFT_BRACE);
        case '}': return makeToken(TokenType::TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TokenType::TOKEN_SEMICOLON);
        case ',': return makeToken(TokenType::TOKEN_COMMA);
        case '.': return makeToken(TokenType::TOKEN_DOT);
        case '-': return makeToken(TokenType::TOKEN_MINUS);
        case '+': return makeToken(TokenType::TOKEN_PLUS);
        case '/': return makeToken(TokenType::TOKEN_SLASH);
        case '*': return makeToken(TokenType::TOKEN_STAR);
        case '!':
            return makeToken(match('=') ? TokenType::TOKEN_BANG_EQUAL : TokenType::TOKEN_BANG);
        case '=':
            return makeToken(match('=') ? TokenType::TOKEN_EQUAL_EQUAL : TokenType::TOKEN_EQUAL);
        case '<':
            return makeToken(match('=') ? TokenType::TOKEN_LESS_EQUAL : TokenType::TOKEN_LESS);
        case '>':
            return makeToken(match('=') ? TokenType::TOKEN_GREATER_EQUAL : TokenType::TOKEN_GREATER);
        case '"':
            return string();
    }

    return errorToken("Unexpected character.");
}