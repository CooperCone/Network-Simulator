#pragma once

#include "util/types.h"

#include <stdbool.h>

typedef bool (*SLLCmpFunc)(void *list, void *data);
typedef void (*SLLForeachFunc)(void *list, void *data);

typedef struct SLNode {
    struct SLNode *next;
} SLNode;

SLNode *sll_prepend(SLNode *list, SLNode *data, u64 size);
SLNode *sll_pop(SLNode *node);
SLNode *sll_popLast(SLNode *list, SLNode *outNode, u64 size);
u64 sll_size(SLNode *node);
SLNode *sll_find(SLNode *list, SLLCmpFunc cmpFunc, void *data);
void sll_foreach(SLNode *list, SLLForeachFunc func, void *data);
