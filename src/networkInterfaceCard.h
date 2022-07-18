#pragma once

#include <stdbool.h>

#include "event.h"
#include "bufferQueue.h"
#include "physicalLayer.h"

typedef struct NetworkInterfaceCard {
    bool isBusy;

    WireTerminal terminal;

    // TODO: Should this be based on number of bytes?
    BufferQueue outgoingQueue;
    BufferQueue incomingQueue;

    // Wire Terminal
    // Arp Module
    // Layer 2 implementation
} NetworkInterfaceCard;

typedef struct {
    NetworkInterfaceCard *card;
    Buffer data;
} NICQueueEventData;

void handleNICQueueOutEvent(EventData data);
void handleNICQueueInEvent(EventData data);

typedef struct {
    NetworkInterfaceCard *card;
} NICProcessEventData;

void handleNICProcessOutEvent(EventData data);
void handleNICProcessInEvent(EventData data);


