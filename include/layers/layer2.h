#pragma once

#include "layers/forward.h"

#include "event.h"
#include "buffer.h"
#include "util/types.h"

typedef u16 EtherType;
#define EtherType_IPv4 0x0800
#define EtherType_ARP 0x0806

#pragma pack(push)
typedef struct {
    u8 preamble[8];
    MACAddress dstAddr;
    MACAddress srcAddr;
    EtherType type;
} EthernetHeader;
#pragma pack(pop)

typedef struct Layer2Provider {
    struct Layer1Provider *layer1Provider;

    EventFuncs onReceiveBuffer;
} Layer2Provider;

typedef struct {
    Layer2Provider *provider;
    Buffer data;
} Layer2InEventData;
