#include "collections/linkedList.h"

#include <stdlib.h>
#include <string.h>

SLNode *sll_prepend(SLNode *list, SLNode *data, u64 size) {
    SLNode *node = malloc(size);
    memcpy(node, data, size);
    node->next = list;
    return node;
}

SLNode *sll_pop(SLNode *node) {
    SLNode *newNode = node->next;
    free(node);
    return newNode;
}

// Returns true if it is the last node, false if not last node
bool sll_popLastHelper(SLNode *list, SLNode *outNode, u64 size) {
    if (list == NULL)
        return false;
    
    if (list->next == NULL) {
        // found last
        memcpy(outNode, list, size);
        free(list);
        return true;
    }
    else {
        if (sll_popLastHelper(list->next, outNode, size)) {
            // this means the next node is the last, so
            // we need to set out pointer to null
            list->next = NULL;
        }
        return false;
    }
}

SLNode *sll_popLast(SLNode *list, SLNode *outNode, u64 size) {
    if (sll_popLastHelper(list, outNode, size))
        return NULL;
    else
        return list;
}

u64 sll_size(SLNode *node) {
    if (node == NULL)
        return 0;
    
    return 1 + sll_size(node->next);
}

SLNode *sll_find(SLNode *list, SLLCmpFunc cmpFunc, void *data) {
    if (list == NULL)
        return NULL;
    
    if (cmpFunc(list, data))
        return list;

    return sll_find(list->next, cmpFunc, data);
}

void sll_foreach(SLNode *list, SLLForeachFunc func, void *data) {
    if (list == NULL)
        return;
    
    func(list, data);

    sll_foreach(list->next, func, data);
}
