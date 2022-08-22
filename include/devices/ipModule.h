#pragma once

#include "devices/forward.h"

#include "layers/layer3.h"

#include "event.h"
#include "bufferQueue.h"
#include "util/types.h"
#include "collections/linkedList.h"

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

typedef struct {
    SLNode node;

    IPAddress networkAddr;
    SubnetMask mask;
    u64 interfaceNumber; // TODO: Should this be a pointer?
} RoutingTable;

RoutingTable *routingTable_addEntry(IPAddress addr, SubnetMask mask, u64 interfaceNumber);

typedef struct IPModule {
    struct ARPModule *arpModule;
    struct UDPModule *udpModule;

    EventFuncs onReceiveBuffer;
    EventFuncs onSendBuffer;

    u64 deviceID;

    IPAddress address;

    BufferQueue outgoingQueue;
    BufferQueue incomingQueue;

    RoutingTable *routingTable;

    bool isBusy;
} IPModule;

typedef struct {
    IPModule *module;
    Buffer data;
    IPAddress addr;
} IPInEventData;

DeclareEvent(handleIPModuleQueueOutEvent, IPInEventData);
DeclareEvent(handleIPModuleQueueInEvent, IPInEventData);

typedef struct {
    IPModule *module;
    IPAddress addr;
} IPProcessEventData;

DeclareEvent(handleIPProcessOutEvent, IPProcessEventData);
DeclareEvent(handleIPProcessInEvent, IPProcessEventData);
