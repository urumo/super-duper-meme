//
// Created by aramh on 3/13/2021.
//

#ifndef CSCRIPTY_VM_H
#define CSCRIPTY_VM_H

#include "chunk.h"
#include "value.h"
#include "table.h"

#define STACK_MAX 256

typedef struct {
    Chunk *chunk;
    uint8_t *ip;
    Value stack[STACK_MAX];
    Value *stackTop;
    Table strings;

    Obj *objects;
} VM;

typedef enum {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();

void freeVM();

InterpretResult interpret(const char *source);

void push(Value value);

Value pop();

#endif //CSCRIPTY_VM_H
