#pragma once

#include "layers/layer2.h"

#include "event.h"
#include "bufferQueue.h"
#include "collections/linkedList.h"
#include "layers/layer1.h"

#define EthPreambleStart 0b10101010
#define EthPreambleEnd 0b10101011

// TODO: Layer provider interface
struct IPModule;

typedef struct {
    SLNode node;

    Buffer buffer;
    EtherType protocol;
} NICOutQueueList;

typedef struct NetworkInterfaceCard {
    Layer2Provider provider;

    struct IPModule *ipModule;
    struct ARPModule *arpModule;

    u64 deviceID;

    bool isBusy;

    MACAddress address;

    // TODO: Should this be based on number of bytes?
    NICOutQueueList *outgoingQueue;
    BufferQueue incomingQueue;

    // Arp Module
} NetworkInterfaceCard;

DeclareEvent(handleNICQueueInEvent, Layer2InEventData);

typedef struct {
    NetworkInterfaceCard *card;
    Buffer data;
    EtherType higherProtocol;
    MACAddress dstAddr;
} NICQueueOutData;
DeclareEvent(handleNICQueueOutEvent, NICQueueOutData);

typedef struct {
    NetworkInterfaceCard *card;
} NICProcessEventData;

DeclareEvent(handleNICProcessOutEvent, NICProcessEventData);
DeclareEvent(handleNICProcessInEvent, NICProcessEventData);


