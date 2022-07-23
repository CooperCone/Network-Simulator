#pragma once

#include "util/types.h"

// Types of errors
// - processing delay
// - queueing delay
// - transmission delay
// - propagation delay

typedef void *EventData;
typedef void (*HandleEvent)(EventData data);

typedef struct {
    HandleEvent handleEvent;
    EventData data;
} Event;

void PostEvent(HandleEvent handler, EventData data, u64 dataSize, u64 delay);
u64 CurTime();

typedef struct {
    Event *event;
    u64 time;
} EventNode;

#define EventNodeListLast(start, numNodes, maxNodes) ((start + numNodes) % maxNodes)
#define EventNodeListNext(i, maxNodes) ((i + 1) % maxNodes)
#define IterEventNodeList(i, c, pos, numNodes, maxNodes) for (u64 i = pos, c = 0;\
 c < numNodes; c++, i = EventNodeListNext(i, maxNodes))

// This is a normal circular queue, but it's sorted by event time
typedef struct {
    EventNode *nodes;
    u64 numNodes;
    u64 maxNodes;
    u64 startNode;
} EventQueue;

void eventQueue_push(EventQueue *queue, EventNode event);
EventNode eventQueue_pop(EventQueue *queue);
