#include "event.h"

#include <stdlib.h>
#include <stdio.h>

void swap(EventQueue *queue, u64 i, u64 ii) {
    EventNode node = queue->nodes[i];
    queue->nodes[i] = queue->nodes[ii];
    queue->nodes[ii] = node;
}

void eventQueue_push(EventQueue *queue, EventNode event) {
    // Check if resize
    if (queue->numNodes == queue->maxNodes) {
        // Resize
        EventNode *prevList = queue->nodes;
        u64 prevPos = queue->startNode;
        u64 prevMaxNodes = queue->maxNodes;

        if (queue->maxNodes == 0)
            queue->maxNodes = 8;
        else
            queue->maxNodes *= 2;

        queue->nodes = calloc(queue->maxNodes, sizeof(EventNode));
        queue->startNode = 0;

        IterEventNodeList(i, c, prevPos, queue->numNodes, prevMaxNodes) {
            queue->nodes[c] = prevList[i];
        }

        free(prevList);
    }

    // Add node to end of queue
    u64 lastPos = EventNodeListLast(queue->startNode, queue->numNodes, queue->maxNodes);
    queue->nodes[lastPos] = event;
    queue->numNodes++;

    // Sort event queue
    IterEventNodeList(i, c1, queue->startNode, queue->numNodes - 1, queue->maxNodes) {
        u64 next = EventNodeListNext(i, queue->maxNodes);
        IterEventNodeList(ii, c2, next, queue->numNodes - c1 - 1, queue->maxNodes) {
            // Sort Queue
            if (queue->nodes[ii].time < queue->nodes[i].time) {
                swap(queue, i, ii);
            }
        }
    }
}

EventNode eventQueue_pop(EventQueue *queue) {
    EventNode node = queue->nodes[queue->startNode];
    queue->startNode = EventNodeListNext(queue->startNode, queue->maxNodes);
    queue->numNodes--;
    return node;
}
