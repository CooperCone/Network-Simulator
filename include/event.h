#pragma once

#include "util/types.h"

// Types of errors
// - processing delay
// - queueing delay
// - transmission delay
// - propagation delay

typedef void *EventData;
typedef void (*HandleEvent)(EventData data);
typedef char* (*GetHandlerName)();

typedef struct {
    HandleEvent handleEvent;
    GetHandlerName getName;
} EventFuncs;

typedef struct {
    u64 deviceID;

    EventFuncs eventFuncs;
    EventData data;
} Event;

void PostEvent(u64 deviceID, EventFuncs funcs, EventData data, u64 dataSize, u64 delay);
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

#define GetFuncs(handlerName) handlerName ## _getFuncs()

#define EventHandlerPrototype(handlerName) void handlerName(EventData data);

#define EventHandlerGetName(handlerName) static char * handlerName ## _getName() {\
    return #handlerName;\
}

#define EventHandlerGetFuncs(handlerName) static EventFuncs handlerName ## _getFuncs() {\
    return (EventFuncs){\
        .handleEvent=handlerName,\
        .getName=handlerName ## _getName\
    };\
}

#define DeclareEvent(handlerName) EventHandlerPrototype(handlerName)\
    EventHandlerGetName(handlerName)\
    EventHandlerGetFuncs(handlerName)
