#pragma once

#include "devices/ipModule.h"
#include "bufferQueue.h"
#include "util/types.h"

typedef u16 PortNumber;

typedef struct {
    IPModule *layer3Provider;

    BufferQueue outgoingQueue;
    BufferQueue incomingQueue;

    bool isBusy;
} UDPModule;

typedef struct {
    UDPModule *module;
    Buffer data;
} UDPQueueEventData;

void handleUDPModuleQueueOutEvent(EventData data);
void handleUDPModuleQueueInEvent(EventData data);

typedef struct {
    UDPModule *module;
} UDPProcessEventData;

void handleUDPProcessOutEvent(EventData data);
void handleUDPProcessInEvent(EventData data);
