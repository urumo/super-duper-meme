//
// Created by aramh on 3/13/2021.
//

#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "value.h"
#include "object.h"

void initValueArray(ValueArray *array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray *array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCap = array->capacity;
        array->capacity = GROW_CAPACITY(oldCap);
        array->values = GROW_ARRAY(Value, array->values, oldCap, array->capacity);
    }
    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray *array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValue(Value value) {
    switch (value.type) {
        case V_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case V_NULL:
            printf("null");
            break;
        case V_NUM:
            printf("%g", AS_NUM(value));
            break;
        case V_OBJ:
            printObject(value);
            break;
    }
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case V_BOOL:
            return AS_BOOL(a) == AS_BOOL(b);
        case V_NULL:
            return true;
        case V_NUM:
            return AS_NUM(a) == AS_NUM(b);
        case V_OBJ: {
            ObjString *aString = AS_STRING(a);
            ObjString *bString = AS_STRING(b);
            return aString->length == bString->length && memcmp(aString->chars, bString->chars, aString->length) == 0;
        }
        default:
            return false;
    }
}