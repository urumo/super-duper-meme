//
// Created by aramh on 3/17/2021.
//

#include "common.h"
#include "value.h"

#ifndef CSCRIPTY_OBJECT_H
#define CSCRIPTY_OBJECT_H

#define OBJ_TYPE(value)   (AS_OBJ(value)->type)

#define IS_STRING(value)  isObjType(value, O_STRING)

#define AS_STRING(value)  ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    O_STRING,
} ObjType;

struct Obj {
    ObjType type;
};

struct ObjString {
    Obj obj;
    int length;
    char *chars;
};

ObjString *copyString(const char *chars, int length);

static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif //CSCRIPTY_OBJECT_H
