//
// Created by aramh on 3/13/2021.
//

#ifndef CSCRIPTY_CHUNK_H
#define CSCRIPTY_CHUNK_H

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NOT,
    OP_NEGATE,
    OP_PUTS,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_RETURN
} OpCode;

typedef struct {
    int capacity;
    int count;
    uint8_t *code;
    int *lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);

void freeChunk(Chunk *chunk);

void writeChunk(Chunk *chunk, uint8_t byte, int line);

int addConstant(Chunk *chunk, Value value);

#endif //CSCRIPTY_CHUNK_H
