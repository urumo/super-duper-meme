//
// Created by aramh on 3/13/2021.
//

#ifndef CSCRIPTY_DEBUG_H
#define CSCRIPTY_DEBUG_H

#include "chunk.h"

void disassembleChunk(Chunk *chunk, const char *name);

int disassembleInstruction(Chunk *chunk, int offset);

#endif //CSCRIPTY_DEBUG_H
