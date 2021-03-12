//
// Created by aramh on 3/13/2021.
//

#ifndef CSCRIPTY_VM_H
#define CSCRIPTY_VM_H

#include "chunk.h"

typedef struct {
    Chunk* chunk;
    uint8_t *ip;
} VM;

typedef enum {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();
InterpretResult interpret(Chunk* chunk);

#endif //CSCRIPTY_VM_H
