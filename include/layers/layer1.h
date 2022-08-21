#pragma once

#include "layers/forward.h"

#include "event.h"
#include "buffer.h"
#include "util/types.h"

typedef void (*InjectError)(struct Layer1Provider *provider, Buffer *data);

typedef struct Layer1Provider {
    InjectError injectError;

    u64 deviceID;

    u64 length;
    u64 bandwidth; // bits per second

    struct Layer1Provider *other;
    struct Layer2Provider *layer2Provider;
} Layer1Provider;

void layer1Provider_connect(Layer1Provider *provider1, Layer1Provider *provider2);

typedef struct {
    Layer1Provider *receiver;
    Buffer data;
} Layer1ReceiveData;

DeclareEvent(handleLayer1Receive, Layer1ReceiveData);
