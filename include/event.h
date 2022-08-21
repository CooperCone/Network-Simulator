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
typedef char* (*GetHandlerEventName)();

typedef struct {
    HandleEvent handleEvent;
    GetHandlerName getName;
    GetHandlerEventName getEventName;
} EventFuncs;

typedef struct {
    u64 deviceID;

    EventFuncs eventFuncs;
    EventData data;
} Event;

#define PostEvent(deviceID, funcs, data, eventName, delay) _postEvent(deviceID,\
    funcs, data, sizeof(eventName), #eventName, delay, __FILE__, __LINE__)
void _postEvent(u64 deviceID, EventFuncs funcs, EventData data, u64 dataSize, char *eventName, u64 delay, char *file, int line);
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

#define EventHandlerGetEventName(handlerName, eventName) static char * handlerName ## _getEventName() {\
    return #eventName;\
}

#define EventHandlerGetFuncs(handlerName) static EventFuncs handlerName ## _getFuncs() {\
    return (EventFuncs){\
        .handleEvent=handlerName,\
        .getName=handlerName ## _getName,\
        .getEventName=handlerName ## _getEventName\
    };\
}

#define DeclareEvent(handlerName, eventName) EventHandlerPrototype(handlerName)\
    EventHandlerGetName(handlerName)\
    EventHandlerGetEventName(handlerName, eventName)\
    EventHandlerGetFuncs(handlerName)
