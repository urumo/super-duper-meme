//
// Created by aramh on 3/13/2021.
//

#ifndef CSCRIPTY_VALUE_H
#define CSCRIPTY_VALUE_H

#include "common.h"

typedef double Value;

typedef struct {
    int capacity;
    int count;
    Value *values;
} ValueArray;

void initValueArray(ValueArray *array);

void writeValueArray(ValueArray *array, Value value);

void freeValueArray(ValueArray *array);

void printValue(Value value);

#endif //CSCRIPTY_VALUE_H
