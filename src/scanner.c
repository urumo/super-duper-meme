//
// Created by aramh on 3/14/2021.
//
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "scanner.h"

typedef struct {
    const char *start;
    const char *current;
    int line;
} Scanner;

Scanner scanner;

void initScanner(const char *source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAtEnd() {
    return *scanner.current == '\0';
}

static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

static char peek() {
    return *scanner.current;
}

static char peekNext() {
    if (isAtEnd()) return '\0';
    return scanner.current[1];
}

static bool match(char expected) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;
    scanner.current++;
    return true;
}

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int) (scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

static Token errorToken(const char *message) {
    Token token;
    token.type = T_ERR;
    token.start = message;
    token.length = (int) strlen(message);
    token.line = scanner.line;
    return token;
}

static void skipWhitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                scanner.line++;
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static Token number() {
    while (isDigit(peek())) advance();
    if (peek() == '.' && isDigit(peekNext())) {
        advance();
        while (isDigit(peek())) advance();
    }

    return makeToken(T_NUMBER);
}

static Token string(bool doubleQuoted) {
    while (peek() != (doubleQuoted ? '"' : '\'') && !isAtEnd()) {
        if (peek() == '\n') scanner.line++;
        advance();
    }
    if (isAtEnd()) return errorToken("Unterminated string.");
    advance();
    return makeToken(T_STRING);
}

Token scanToken() {
    skipWhitespace();
    scanner.start = scanner.current;
    if (isAtEnd()) return makeToken(T_EOF);
    char c = advance();
    if (isDigit(c)) return number();
    switch (c) {
        case '(':
            return makeToken(T_LPAREN);
        case ')':
            return makeToken(T_RPAREN);
        case '{':
            return makeToken(T_LBRACE);
        case '}':
            return makeToken(T_RBRACE);
        case ';':
            return makeToken(T_SEMICOLON);
        case ',':
            return makeToken(T_COMMA);
        case '.':
            return makeToken(T_DOT);
        case '-':
            return makeToken(T_MINUS);
        case '+':
            return makeToken(T_PLUS);
        case '/':
            return makeToken(T_SLASH);
        case '*':
            return makeToken(T_ASTERISK);
        case '!':
            return makeToken(
                    match('=') ? T_NE : T_BANG);
        case '=':
            return makeToken(
                    match('=') ? T_EQ : T_ASSIGN);
        case '<':
            return makeToken(
                    match('=') ? T_LTE : T_LT);
        case '>':
            return makeToken(
                    match('=') ? T_GTE : T_GT);
        case '\'':
            return string(false);
        case '"':
            return string(true);
    }
    return errorToken("Unexpected Character.");
}
