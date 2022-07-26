#pragma once

#include "layers/forward.h"

#include "event.h"
#include "bufferQueue.h"
#include "util/types.h"

#include <stdbool.h>

typedef u8 IPAddress[4];

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
    u64 deviceID;

    IPAddress address;
    struct Layer2Provider *layer2Provider;
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
