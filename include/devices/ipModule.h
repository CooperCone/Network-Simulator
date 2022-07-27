#pragma once

#include "layers/layer3.h"

#include "event.h"
#include "bufferQueue.h"
#include "util/types.h"

#include <stdbool.h>

#pragma pack(push)
typedef struct {
    u8 version : 4;
    u8 headerLength : 4; // Number of u32s
    u8 typeOfService; // TODO: DSCP and ECN
    u16 datagramLength;
    u16 identifier;
    struct {
        u8 flag1 : 1;
        u8 flag2 : 1;
        u8 flag3 : 1;
        u16 fragmentationOffset : 13;
    };
    u8 timeToLive;
    u8 upperLayerProtocol;
    u16 checksum;
    IPAddress srcIPAddr;
    IPAddress dstIPAddr;
} IPHeader;
#pragma pack(pop)

typedef struct IPModule {
    Layer3Provider provider;

    u64 deviceID;

    IPAddress address;

    BufferQueue outgoingQueue;
    BufferQueue incomingQueue;

    bool isBusy;
} IPModule;


void handleIPModuleQueueOutEvent(EventData data);
void handleIPModuleQueueInEvent(EventData data);

typedef struct {
    IPModule *module;
} IPProcessEventData;

void handleIPProcessOutEvent(EventData data);
void handleIPProcessInEvent(EventData data);
