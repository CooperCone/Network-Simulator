#pragma once

#include "layers/forward.h"

#include "event.h"
#include "buffer.h"
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

typedef struct Layer4Provider {
    struct Layer3Provider *layer3Provider;
    struct EchoClient *layer7Provider;

    EventFuncs onReceiveBuffer;
    EventFuncs onSendBuffer;
} Layer4Provider;

typedef struct {
    Layer4Provider *layer4;
    Buffer data;
    IPAddress addr;
} Layer4InEventData;
