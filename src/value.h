//
// Created by aramh on 3/13/2021.
//

#ifndef CSCRIPTY_VALUE_H
#define CSCRIPTY_VALUE_H

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum {
    V_BOOL,
    V_NULL,
    V_NUM,
    V_OBJ,
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj *obj;
    } as;
} Value;

#define IS_BOOL(val) ((val).type == V_BOOL)
#define IS_NULL(val) ((val).type == V_NULL)
#define IS_NUM(val)  ((val).type == V_NUM)
#define IS_OBJ(val)  ((val).type == V_OBJ)

#define AS_OBJ(val)    ((val).as.obj)
#define AS_BOOL(val)   ((val).as.boolean)
#define AS_NUM(val)    ((val).as.number)

#define BOOL_VAL(val)    ((Value){V_BOOL, {.boolean = val}})
#define NULL_VAL         ((Value){V_NULL, {.number = 0}})
#define NUM_VAL(val)     ((Value){V_NUM, {.number = val}})
#define OBJ_VAL(object)  ((Value){V_OBJ, {.obj = (Obj*)object}})

typedef struct {
    int capacity;
    int count;
    Value *values;
} ValueArray;

bool valuesEqual(Value a, Value b);

void initValueArray(ValueArray *array);

void writeValueArray(ValueArray *array, Value value);

void freeValueArray(ValueArray *array);

void printValue(Value value);

#endif //CSCRIPTY_VALUE_H
