//
// Created by aramh on 3/14/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "object.h"

#ifdef DEBUG_PRINT_CODE

#include "debug.h"

#endif

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

typedef enum {
    NONE,
    ASSIGNMENT,  // =
    OR,          // or
    AND,         // and
    EQUALITY,    // == !=
    COMPARISON,  // < > <= >=
    SUM,         // + - Term
    PRODUCT,     // * / Factor
    PREFIX,      // ! -
    CALL,        // ()
    PRIMARY
} Precedence;

typedef void (*ParseFn)();

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Parser parser;

Chunk *compilingChunk;

static Chunk *currentChunk() {
    return compilingChunk;
}

static void errorAt(Token *token, const char *message) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == T_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == T_ERR) {
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void error(const char *message) {
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char *message) {
    errorAt(&parser.current, message);
}

static void advance() {
    parser.previous = parser.current;
    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != T_ERR) break;
        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char *message) {
    if (parser.current.type == type) {
        advance();
        return;
    }
    errorAtCurrent(message);
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t b1, uint8_t b2) {
    emitByte(b1);
    emitByte(b2);
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }
    return (uint8_t) constant;
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler() {
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}

static void expression();

static ParseRule *getRule(TokenType type);

static void parsePrecedence(Precedence precedence);

static void binary() {
    TokenType operatorType = parser.previous.type;

    ParseRule *rule = getRule(operatorType);
    parsePrecedence((Precedence) (rule->precedence + 1));

    switch (operatorType) {
        case T_NE:
            emitBytes(OP_EQUAL, OP_NOT);
            break;
        case T_EQ:
            emitByte(OP_EQUAL);
            break;
        case T_GT:
            emitByte(OP_GREATER);
            break;
        case T_GTE:
            emitBytes(OP_LESS, OP_NOT);
            break;
        case T_LT:
            emitByte(OP_LESS);
            break;
        case T_LTE:
            emitBytes(OP_GREATER, OP_NOT);
            break;
        case T_PLUS:
            emitByte(OP_ADD);
            break;
        case T_MINUS:
            emitByte(OP_SUB);
            break;
        case T_ASTERISK:
            emitByte(OP_MUL);
            break;
        case T_SLASH:
            emitByte(OP_DIV);
            break;
    }
}

static void literal() {
    switch (parser.previous.type) {
        case T_FALSE:
            emitByte(OP_FALSE);
            break;
        case T_TRUE:
            emitByte(OP_TRUE);
            break;
        case T_NULL:
            emitByte(OP_NULL);
            break;
        default:
            return;
    }
}

static void grouping() {
    expression();
    consume(T_RPAREN, "`)` is expected after expression.");
}

static void number() {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUM_VAL(value));
}

static void string() {
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void unary() {
    TokenType operatorType = parser.previous.type;

    parsePrecedence(PREFIX);

    switch (operatorType) {
        case T_BANG:
            emitByte(OP_NOT);
            break;
        case T_MINUS:
            emitByte(OP_NEGATE);
            break;
        default:
            return;
    }
}

ParseRule rules[] = {
        [T_LPAREN]            = {grouping, NULL, NONE},
        [T_RPAREN]            = {NULL, NULL, NONE},
        [T_LBRACE]            = {NULL, NULL, NONE},
        [T_RBRACE]            = {NULL, NULL, NONE},
        [T_COMMA]             = {NULL, NULL, NONE},
        [T_DOT]               = {NULL, NULL, NONE},
        [T_MINUS]             = {unary, binary, SUM},
        [T_PLUS]              = {NULL, binary, SUM},
        [T_SEMICOLON]         = {NULL, NULL, NONE},
        [T_SLASH]             = {NULL, binary, PRODUCT},
        [T_ASTERISK]          = {NULL, binary, PRODUCT},
        [T_BANG]              = {unary, NULL, NONE},
        [T_NE]                = {NULL, binary, EQUALITY},
        [T_ASSIGN]            = {NULL, NULL, NONE},
        [T_EQ]                = {NULL, binary, EQUALITY},
        [T_GT]                = {NULL, binary, COMPARISON},
        [T_GTE]               = {NULL, binary, COMPARISON},
        [T_LT]                = {NULL, binary, COMPARISON},
        [T_LTE]               = {NULL, binary, COMPARISON},
        [T_IDENT]             = {NULL, NULL, NONE},
        [T_STRING]            = {string, NULL, NONE},
        [T_NUMBER]            = {number, NULL, NONE},
        [T_AND]               = {NULL, NULL, NONE},
        [T_CLASS]             = {NULL, NULL, NONE},
        [T_ELSE]              = {NULL, NULL, NONE},
        [T_FALSE]             = {literal, NULL, NONE},
        [T_FOR]               = {NULL, NULL, NONE},
        [T_FUN]               = {NULL, NULL, NONE},
        [T_IF]                = {NULL, NULL, NONE},
        [T_NULL]              = {literal, NULL, NONE},
        [T_OR]                = {NULL, NULL, NONE},
        [T_PUTS]              = {NULL, NULL, NONE},
        [T_RETURN]            = {NULL, NULL, NONE},
        [T_SUPER]             = {NULL, NULL, NONE},
        [T_THIS]              = {NULL, NULL, NONE},
        [T_TRUE]              = {literal, NULL, NONE},
        [T_LET]               = {NULL, NULL, NONE},
        [T_WHILE]             = {NULL, NULL, NONE},
        [T_ERR]               = {NULL, NULL, NONE},
        [T_EOF]               = {NULL, NULL, NONE},
};

static void parsePrecedence(Precedence precedence) {
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expression is expected.");
        return;
    }
    prefixRule();

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }
}

static ParseRule *getRule(TokenType type) {
    return &rules[type];
}

static void expression() {
    parsePrecedence(ASSIGNMENT);
}

bool compile(const char *source, Chunk *chunk) {
    initScanner(source);
    compilingChunk = chunk;
    parser.hadError = false;
    parser.panicMode = false;
    advance();
    expression();
    consume(T_EOF, "Expected end of expression.");
    endCompiler();
    return !parser.hadError;
}
