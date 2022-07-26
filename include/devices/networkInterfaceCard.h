#pragma once

#include "layers/layer2.h"

#include "event.h"
#include "bufferQueue.h"
#include "layers/layer1.h"

#define EthPreambleStart 0b10101010
#define EthPreambleEnd 0b10101011

// TODO: Layer provider interface
struct IPModule;

typedef struct NetworkInterfaceCard {
    Layer2Provider provider;

    u64 deviceID;

    bool isBusy;

    MACAddress address;

    // TODO: Should this be based on number of bytes?
    BufferQueue outgoingQueue;
    BufferQueue incomingQueue;

    // Arp Module
} NetworkInterfaceCard;

void handleNICQueueOutEvent(EventData data);
void handleNICQueueInEvent(EventData data);

typedef struct {
    NetworkInterfaceCard *card;
} NICProcessEventData;

void handleNICProcessOutEvent(EventData data);
void handleNICProcessInEvent(EventData data);


