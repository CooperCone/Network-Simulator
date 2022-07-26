#pragma once

#include "event.h"
#include "Buffer.h"
#include "util/types.h"

#include "layers/forward.h"

typedef void (*InjectError)(struct Layer1Provider *provider, Buffer *data);

typedef struct Layer1Provider {
    InjectError injectError;

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

void handleLayer1Receive(EventData data);
