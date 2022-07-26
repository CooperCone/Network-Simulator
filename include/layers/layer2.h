#pragma once

#include "util/types.h"

#include "event.h"
#include "buffer.h"
#include "layers/forward.h"

typedef u8 MACAddress[6];

#pragma pack(push)
typedef struct {
    u8 preamble[8];
    MACAddress dstAddr;
    MACAddress srcAddr;
    u8 type[2];
} EthernetHeader;
#pragma pack(pop)

typedef struct Layer2Provider {
    struct Layer1Provider *layer1Provider;
    struct IPModule *layer3Provider;

    HandleEvent onReceiveBuffer;
    HandleEvent onSendBuffer;
} Layer2Provider;

typedef struct {
    Layer2Provider *provider;
    Buffer data;
} Layer2InEventData;
