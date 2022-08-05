#pragma once

#include "layers/forward.h"

#include "event.h"
#include "buffer.h"
#include "util/types.h"

void ipAddr_copy(IPAddress dst, IPAddress src);
void ipAddr_toStr(IPAddress addr, IPStr outStr);
void ipAddr_fromStr(IPStr str, IPAddress addr);

typedef struct Layer3Provider {
    struct Layer2Provider *layer2Provider;
    struct Layer4Provider *layer4Provider;

    EventFuncs onReceiveBuffer;
    EventFuncs onSendBuffer;
} Layer3Provider;

typedef struct {
    Layer3Provider *module;
    Buffer data;
    IPAddress addr;
} Layer3InEventData;
