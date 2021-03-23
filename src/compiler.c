//
// Created by aramh on 3/14/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

typedef void (*ParseFn)(bool canAssign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct {
    Token name;
    int depth;
} Local;

typedef struct {
    Local locals[UINT8_COUNT];
    int localCount;
    int scopeDepth;
} Compiler;

Parser parser;

Compiler *current = NULL;

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

static bool check(TokenType type) {
    return parser.current.type == type;
}

static bool match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t b1, uint8_t b2) {
    emitByte(b1);
    emitByte(b2);
}

static void emitLoop(int loopStart) {
    emitByte(OP_LOOP);
    int offset = currentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body too large");
    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

static int emitJump(uint8_t intsruction) {
    emitByte(intsruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->count - 2;
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

static void patchJump(int offset) {
    int jump = currentChunk()->count - offset - 2;
    if (jump > UINT16_MAX) {
        error("Too much code to jump over");
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

static void initCompiler(Compiler *compiler) {
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    current = compiler;
}

static void endCompiler() {
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}

static void beginScope() {
    current->scopeDepth++;
}

static void endScope() {
    current->scopeDepth--;

    while (current->localCount > 0 &&
           current->locals[current->localCount - 1].depth >
           current->scopeDepth) {
        emitByte(OP_POP);
        current->localCount--;
    }
}

static void expression();

static void statement();

static void declaration();

static ParseRule *getRule(TokenType type);

static void parsePrecedence(Precedence precedence);

static uint8_t identifierConstant(Token *name) {
    return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static bool identifiersEqual(Token *a, Token *b) {
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler *compiler, Token *name) {
    for (int i = compiler->localCount - 1; i >= 0; i--) {
        Local *local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name)) {
            if (local->depth == -1) {
                error("Can't initialize a variable with itself");
            }
            return i;
        }
    }
    return -1;
}

static void addLocal(Token name) {
    if (current->localCount == UINT8_COUNT) {
        error("Too many variables in one scope");
        return;
    }
    Local *local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
}

static void declareVariable() {
    if (current->scopeDepth == 0) return;
    Token *name = &parser.previous;
    for (int i = current->localCount - 1; i >= 0; i--) {
        Local *local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scopeDepth) {
            break;
        }

        if (identifiersEqual(name, &local->name)) {
            error("Variable with this name is already declared in this scope");
        }
    }
    addLocal(*name);
}

static uint8_t parseVariable(const char *errorMessage) {
    consume(T_IDENT, errorMessage);
    declareVariable();
    if (current->scopeDepth > 0) return 0;
    return identifierConstant(&parser.previous);
}

static void markInitialized() {
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(uint8_t global) {
    if (current->scopeDepth > 0) {
        markInitialized();
        return;
    }
    emitBytes(OP_DEFINE_GLOBAL, global);
}

static void and_(bool canAssign) {
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parsePrecedence(AND);
    patchJump(endJump);
}

static void binary(bool canAssign) {
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

static void literal(bool canAssign) {
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

static void grouping(bool canAssign) {
    expression();
    consume(T_RPAREN, "`)` is expected after expression.");
}

static void number(bool canAssign) {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUM_VAL(value));
}

static void or_(bool canAssign) {
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(OR);
    patchJump(endJump);
}

static void string(bool canAssign) {
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign) {
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name);
    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }
    if (canAssign && match(T_ASSIGN)) {
        expression();
        emitBytes(setOp, (uint8_t) arg);
    } else {
        emitBytes(getOp, (uint8_t) arg);
    }
}

static void variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
}

static void unary(bool canAssign) {
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
        [T_IDENT]             = {variable, NULL, NONE},
        [T_STRING]            = {string, NULL, NONE},
        [T_NUMBER]            = {number, NULL, NONE},
        [T_AND]               = {NULL, and_, AND},
        [T_CLASS]             = {NULL, NULL, NONE},
        [T_ELSE]              = {NULL, NULL, NONE},
        [T_FALSE]             = {literal, NULL, NONE},
        [T_FOR]               = {NULL, NULL, NONE},
        [T_FUN]               = {NULL, NULL, NONE},
        [T_IF]                = {NULL, NULL, NONE},
        [T_NULL]              = {literal, NULL, NONE},
        [T_OR]                = {NULL, or_, OR},
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

    bool canAssign = precedence <= ASSIGNMENT;
    prefixRule(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && match(T_ASSIGN)) {
        error("Invalid assignment target");
    }
}

static ParseRule *getRule(TokenType type) {
    return &rules[type];
}

static void expression() {
    parsePrecedence(ASSIGNMENT);
}

static void block() {
    while (!check(T_RBRACE) && !check(T_EOF)) {
        declaration();
    }
    consume(T_RBRACE, "`}` expected at the end of a block");
}

static void varDeclaration() {
    uint8_t global = parseVariable("Expected variable name");
    if (match(T_ASSIGN)) {
        expression();
    } else {
        emitByte(OP_NULL);
    }

    consume(T_SEMICOLON, "`;` expected after variable declaration");

    defineVariable(global);
}

static void expressionStatement() {
    expression();
    consume(T_SEMICOLON, "`;` expected after expression.");
    emitByte(OP_POP);
}

static void forStatement() {
    beginScope();
    consume(T_LPAREN, "`(` expected after `for`");

    if (match(T_SEMICOLON)) {}
    else if (match(T_LET)) {
        varDeclaration();
    } else {
        expressionStatement();
    }

    int loopStart = currentChunk()->count;

    int exitJump = -1;

    if (!match(T_SEMICOLON)) {
        expression();
        consume(T_SEMICOLON, "`;` expected after the loop condition");
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP);
    }
    if (!match(T_RPAREN)) {
        int bodyJump = emitJump(OP_JUMP);
        int incrementStart = currentChunk()->count;
        expression();
        emitByte(OP_POP);
        consume(T_RPAREN, "`)` expected after the clauses");
        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }
    statement();
    emitLoop(loopStart);
    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(OP_POP);
    }
    endScope();
}

static void ifStatement() {
    consume(T_LPAREN, "`(` expected after `if`");
    expression();
    consume(T_RPAREN, "`)` expected after condition");
    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    int elseJump = emitJump(OP_JUMP);
    patchJump(thenJump);
    emitByte(OP_POP);
    if (match(T_ELSE)) statement();
    patchJump(elseJump);
}

static void printStatement() {
    expression();
    consume(T_SEMICOLON, "`;` expected after value.");
    emitByte(OP_PUTS);
}

static void whileStatement() {
    int loopStart = currentChunk()->count;
    consume(T_LPAREN, "`(` expected after `while`");
    expression();
    consume(T_RPAREN, "`)` expected after condition");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    emitLoop(loopStart);
    patchJump(exitJump);
    emitByte(OP_POP);
}

static void synchronize() {
    parser.panicMode = false;

    while (parser.current.type != T_EOF) {
        if (parser.previous.type == T_SEMICOLON) return;

        switch (parser.current.type) {
            case T_CLASS:
            case T_FUN:
            case T_LET:
            case T_FOR:
            case T_IF:
            case T_WHILE:
            case T_PUTS:
            case T_RETURN:
                return;
            default:;
        }
        advance();
    }
}

static void declaration() {
    if (match(T_LET)) {
        varDeclaration();
    } else {
        statement();
    }

    if (parser.panicMode) synchronize();
}

static void statement() {
    if (match(T_PUTS)) {
        printStatement();
    } else if (match(T_FOR)) {
        forStatement();
    } else if (match(T_IF)) {
        ifStatement();
    } else if (match(T_WHILE)) {
        whileStatement();
    } else if (match(T_LBRACE)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}

bool compile(const char *source, Chunk *chunk) {
    initScanner(source);
    Compiler compiler;
    initCompiler(&compiler);
    compilingChunk = chunk;
    parser.hadError = false;
    parser.panicMode = false;
    advance();
    while (!match(T_EOF)) {
        declaration();
    }
    endCompiler();
    return !parser.hadError;
}