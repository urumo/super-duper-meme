//
// Created by aramh on 3/18/2021.
//

#ifndef CSCRIPTY_TABLE_H
#define CSCRIPTY_TABLE_H

#include "common.h"
#include "value.h"

typedef struct {
    ObjString *key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry *entries;
} Table;

void initTable(Table *table);

void freeTable(Table *table);

#endif //CSCRIPTY_TABLE_H
