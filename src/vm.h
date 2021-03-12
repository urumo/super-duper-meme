//
// Created by aramh on 3/13/2021.
//

#ifndef CSCRIPTY_VM_H
#define CSCRIPTY_VM_H

#include "chunk.h"

typedef struct {
    Chunk* chunk;
} VM;

void initVM();
void freeVM();

#endif //CSCRIPTY_VM_H
