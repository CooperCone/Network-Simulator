#pragma once

#include <stdbool.h>

#include "event.h"
#include "bufferQueue.h"
#include "layers/layer1.h"

// TODO: Should this really be a u64?
typedef u8 MACAddress[6];

#define EthPreambleStart 0b10101010
#define EthPreambleEnd 0b10101011

#pragma pack(push)
typedef struct {
    u8 preamble[8];
    MACAddress dstAddr;
    MACAddress srcAddr;
    u8 type[2];
} EthernetHeader;
#pragma pack(pop)

// TODO: Layer provider interface
struct IPModule;

typedef struct NetworkInterfaceCard {
    u64 deviceID;

    bool isBusy;

    MACAddress address;

    Layer1Provider *layer1Provider;
    struct IPModule *layer3Provider;

    // TODO: Should this be based on number of bytes?
    BufferQueue outgoingQueue;
    BufferQueue incomingQueue;

    // Arp Module
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


