#pragma once

#include "layers/forward.h"

#include "util/types.h"
#include "buffer.h"
#include "event.h"

typedef u8 IPAddress[4];

typedef struct Layer3Provider {
    struct Layer2Provider *layer2Provider;
    struct UDPModule *layer4Provider;

    HandleEvent onReceiveBuffer;
    HandleEvent onSendBuffer;
} Layer3Provider;

typedef struct {
    Layer3Provider *module;
    Buffer data;
} Layer3InEventData;
