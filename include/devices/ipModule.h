#pragma once

#include "devices/networkInterfaceCard.h"
#include "bufferQueue.h"
#include "util/types.h"

#include <stdbool.h>

typedef u8 IPAddress[4];

struct UDPModule;

typedef struct IPModule {
    IPAddress address;
    NetworkInterfaceCard *layer2Provider;
    struct UDPModule *layer4Provider;

    BufferQueue outgoingQueue;
    BufferQueue incomingQueue;

    bool isBusy;
} IPModule;

typedef struct {
    IPModule *module;
    Buffer data;
} IPQueueEventData;

void handleIPModuleQueueOutEvent(EventData data);
void handleIPModuleQueueInEvent(EventData data);

typedef struct {
    IPModule *module;
} IPProcessEventData;

void handleIPProcessOutEvent(EventData data);
void handleIPProcessInEvent(EventData data);
