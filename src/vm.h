//
// Created by aramh on 3/13/2021.
//

#ifndef CSCRIPTY_VM_H
#define CSCRIPTY_VM_H

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
    Chunk *chunk;
    uint8_t *ip;
    Value stack[STACK_MAX];
    Value *stackTop;
} VM;

typedef enum {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
} InterpretResult;

void initVM();

void freeVM();

InterpretResult interpret(Chunk *chunk);

void push(Value value);

Value pop();

#endif //CSCRIPTY_VM_H
