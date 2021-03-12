//
// Created by aramh on 3/13/2021.
//

#ifndef CSCRIPTY_CHUNK_H
#define CSCRIPTY_CHUNK_H

#include "common.h"

typedef enum {
    OP_RETURN
} OpCode;

typedef struct {
    int capacity;
    int count;
    uint8_t *code;
} Chunk;

void initChunk(Chunk *chunk);

void freeChunk(Chunk *chunk);

void writeChunk(Chunk *chunk, uint8_t byte);

#endif //CSCRIPTY_CHUNK_H
