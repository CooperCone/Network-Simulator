#include "devices/wire.h"

#include <stdio.h>
#include <stdlib.h>

#include "devices/networkInterfaceCard.h"

void layer1Provider_connect(Layer1Provider *provider1, Layer1Provider *provider2) {
    provider1->other = provider2;
    provider2->other = provider1;
}

void handleLayer1Receive(EventData data) {
    // Send card to incoming queue
    Layer1ReceiveData *d = data;

    NICQueueEventData newData = {
        .card=d->receiver->card,
        .data=d->data
    };

    d->receiver->injectError(d->receiver, &(newData.data));

    PostEvent(handleNICQueueInEvent, &newData, sizeof(newData), 0);
}

Layer1Provider *stableWire_create() {
    StableWire *wire = calloc(1, sizeof(StableWire));

    wire->provider.injectError = noError;

    return (Layer1Provider*)wire;
}

void noError(Layer1Provider *provider, Buffer *data) {
    // Do nothing
}

Layer1Provider *singleBitErrorWire_create(f64 errorRate) {
    SingleBitErrorWire *wire = calloc(1, sizeof(StableWire));

    wire->errorRate = errorRate;
    wire->provider.injectError = singleBitError;

    return (Layer1Provider*)wire;
}

void singleBitError(Layer1Provider *provider, Buffer *data) {
    SingleBitErrorWire *wire = (SingleBitErrorWire*)provider;

    // TODO: Do this stuff
    for (u64 i = 0; i < data->dataSize * 8; i++) {
        if ((double)rand() / (double)RAND_MAX > wire->errorRate) {
            u64 byteNumber = i / 8;
            u64 bitNumber = i % 8;

            data->data[byteNumber] = data->data[byteNumber] ^ (1 << bitNumber);
        }
    }
}