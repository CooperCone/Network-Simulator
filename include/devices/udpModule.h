#pragma once

#include "layers/layer4.h"

#include "layers/layer3.h"

#include "bufferQueue.h"
#include "util/types.h"

typedef struct UDPModule {
    Layer4Provider provider;

    u64 deviceID;

    BufferQueue outgoingQueue;
    BufferQueue incomingQueue;

    bool isBusy;
} UDPModule;

DeclareEvent(handleUDPModuleQueueOutEvent);
DeclareEvent(handleUDPModuleQueueInEvent);

typedef struct {
    UDPModule *module;
} UDPProcessEventData;

DeclareEvent(handleUDPProcessOutEvent);
DeclareEvent(handleUDPProcessInEvent);
