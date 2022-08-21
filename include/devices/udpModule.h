#pragma once

#include "layers/forward.h"
#include "devices/forward.h"

#include "layers/layer3.h"

#include "event.h"
#include "bufferQueue.h"
#include "util/types.h"

typedef u16 PortNumber;

#pragma pack(push)
typedef struct {
    PortNumber srcPort;
    PortNumber dstPort;
    u16 length;
    u16 checksum;
} UDPHeader;
#pragma pack(pop)

typedef struct UDPModule {
    struct EchoClient *echoClient;
    struct IPModule *ipModule;

    u64 deviceID;

    EventFuncs onReceiveBuffer;
    EventFuncs onSendBuffer;

    BufferQueue outgoingQueue;
    BufferQueue incomingQueue;

    bool isBusy;
} UDPModule;

typedef struct {
    UDPModule *module;
    Buffer data;
    IPAddress addr;
} UDPInEventData;

DeclareEvent(handleUDPModuleQueueOutEvent, UDPInEventData);
DeclareEvent(handleUDPModuleQueueInEvent, UDPInEventData);

typedef struct {
    UDPModule *module;
    IPAddress addr;
} UDPProcessEventData;

DeclareEvent(handleUDPProcessOutEvent, UDPProcessEventData);
DeclareEvent(handleUDPProcessInEvent, UDPProcessEventData);
