#pragma once

#include "layers/layer2.h"
#include "layers/layer1.h"

typedef struct {
    Layer2Provider provider;

    u64 numPorts;
    Layer1Provider **ports;
} Switch;
