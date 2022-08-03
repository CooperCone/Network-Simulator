#include "devices/wire.h"

#include <stdio.h>
#include <stdlib.h>

#include "devices/networkInterfaceCard.h"

Layer1Provider *stableWire_create(u64 deviceID, u64 length, u64 bandwidth) {
    StableWire *wire = calloc(1, sizeof(StableWire));

    wire->provider.length = length;
    wire->provider.bandwidth = bandwidth;
    wire->provider.injectError = noError;
    wire->provider.deviceID = deviceID;

    return (Layer1Provider*)wire;
}

void noError(Layer1Provider *provider, Buffer *data) {
    // Do nothing
}

Layer1Provider *singleBitErrorWire_create(u64 deviceID, f64 errorRate, u64 length, u64 bandwidth) {
    SingleBitErrorWire *wire = calloc(1, sizeof(SingleBitErrorWire));

    wire->errorRate = errorRate;
    wire->provider.injectError = singleBitError;
    wire->provider.length = length;
    wire->provider.bandwidth = bandwidth;
    wire->provider.deviceID = deviceID;

    return (Layer1Provider*)wire;
}

void singleBitError(Layer1Provider *provider, Buffer *data) {
    SingleBitErrorWire *wire = (SingleBitErrorWire*)provider;

    // TODO: Do this stuff
    for (u64 i = 0; i < data->dataSize * 8; i++) {
        if ((double)rand() / (double)RAND_MAX > wire->errorRate) {
            printf("Injecting error\n");

            u64 byteNumber = i / 8;
            u64 bitNumber = i % 8;

            data->data[byteNumber] = data->data[byteNumber] ^ (1 << bitNumber);
        }
    }
}