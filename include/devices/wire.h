#pragma once

#include "layers/layer1.h"

typedef struct {
    Layer1Provider provider;
} StableWire;

Layer1Provider *stableWire_create();
void noError(Layer1Provider *provider, Buffer *data);

typedef struct {
    Layer1Provider provider;
    f64 errorRate;
} SingleBitErrorWire;

Layer1Provider *singleBitErrorWire_create(f64 errorRate);
void singleBitError(Layer1Provider *provider, Buffer *data);
