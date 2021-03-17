//
// Created by aramh on 3/13/2021.
//

#ifndef CSCRIPTY_MEMORY_H
#define CSCRIPTY_MEMORY_H

#include "common.h"
#include "object.h"

#define FREE(t, ptr) reallocate(ptr, sizeof(t), 0)
#define GROW_CAPACITY(cap) ((cap) < 8 ? 8 : (cap) * 8)
#define GROW_ARRAY(t, ptr, oldCount, newCount) \
(t*)reallocate(ptr, sizeof(t) * (oldCount), sizeof(t) * (newCount))
#define FREE_ARRAY(t, ptr, oldCount) \
reallocate(ptr, sizeof(t) * (oldCount), 0)
#define ALLOCATE(t, count) (t*)reallocate(NULL, 0, sizeof(t) * (count))

void *reallocate(void *ptr, size_t oldSize, size_t newSize);

void freeObjects();

#endif //CSCRIPTY_MEMORY_H
