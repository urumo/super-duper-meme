//
// Created by aramh on 3/14/2021.
//

#ifndef CSCRIPTY_SCANNER_H
#define CSCRIPTY_SCANNER_H

typedef enum {
    T_LPAREN, T_RPAREN, // ( )
    T_LBRACE, T_RBRACE, // { }
    T_COMMA, T_DOT, T_MINUS, T_PLUS, // , . - +
    T_SEMICOLON, T_SLASH, T_ASTERISK, // ; / *
    T_BANG, T_NE, T_ASSIGN, T_EQ, // ! != = ==
    T_GT, T_GTE, // < <=
    T_LT, T_LTE, // > >=
    T_IDENT, T_STRING, T_NUMBER, // name, ""/'', -infinity..infinity
    T_AND, T_CLASS, T_ELSE, T_FALSE,
    T_FOR, T_FUN, T_IF, T_NULL, T_OR,
    T_PUTS, T_RETURN, T_SUPER, T_THIS,
    T_TRUE, T_LET, T_WHILE,
    T_ERR, T_EOF
} TokenType;

typedef struct {
    int length;
    int line;
    const char *start;
    TokenType type;
} Token;

void initScanner(const char *source);

Token scanToken();

#endif //CSCRIPTY_SCANNER_H
