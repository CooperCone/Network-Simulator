#pragma once

#include "util/types.h"
#include "layers/layer1.h"

typedef struct {
    Layer1Provider provider;
} StableWire;

Layer1Provider *stableWire_create(u64 length, u64 bandwidth);
void noError(Layer1Provider *provider, Buffer *data);

typedef struct {
    Layer1Provider provider;
    f64 errorRate;
} SingleBitErrorWire;

Layer1Provider *singleBitErrorWire_create(f64 errorRate, u64 length, u64 bandwidth);
void singleBitError(Layer1Provider *provider, Buffer *data);
