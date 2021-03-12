//
// Created by aramh on 3/13/2021.
//

#ifndef CSCRIPTY_CHUNK_H
#define CSCRIPTY_CHUNK_H

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_RETURN
} OpCode;

typedef struct {
    int capacity;
    int count;
    uint8_t *code;
    ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);

void freeChunk(Chunk *chunk);

void writeChunk(Chunk *chunk, uint8_t byte);

int addConstant(Chunk *chunk, Value value);

#endif //CSCRIPTY_CHUNK_H
