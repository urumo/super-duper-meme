//
// Created by aramh on 3/13/2021.
//

#ifndef CSCRIPTY_VALUE_H
#define CSCRIPTY_VALUE_H

#include "common.h"

typedef enum {
    V_BOOL,
    V_NULL,
    V_NUM,
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
    } as;
} Value;

#define IS_BOOL(val) ((val).type == V_BOOL)
#define IS_NULL(val) ((val).type == V_NULL)
#define IS_NUM(val)  ((val).type == V_NUM)

#define AS_BOOL(val)   ((val).as.boolean)
#define AS_NUM(val)    ((val).as.number)

#define BOOL_VAL(val)  ((Value){V_BOOL, {.boolean = val}})
#define NULL_VAL       ((Value){V_NULL, {.number = 0}})
#define NUM_VAL(val)   ((Value){V_NUM, {.number = val}})

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
