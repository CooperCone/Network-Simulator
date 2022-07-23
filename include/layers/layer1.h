#pragma once

#include "event.h"
#include "Buffer.h"
#include "util/types.h"

struct NetworkInterfaceCard;
struct Layer1Provider;

typedef void (*InjectError)(struct Layer1Provider *provider, Buffer *data);

typedef struct Layer1Provider {
    InjectError injectError;

    struct Layer1Provider *other;
    // Info like length, bytes per second, etc

    // TODO: This shouldn't always be a network interface card
    // It should also probably be some generic provider
    struct NetworkInterfaceCard *card;
} Layer1Provider;

void layer1Provider_connect(Layer1Provider *provider1, Layer1Provider *provider2);

typedef struct {
    Layer1Provider *receiver;
    Buffer data;
} Layer1ReceiveData;

void handleLayer1Receive(EventData data);
