#pragma once

#include "event.h"
#include "Buffer.h"

struct NetworkInterfaceCard;

typedef struct WireTerminal {
    struct WireTerminal *other;
    // Info like length, bytes per second, etc

    // TODO: This shouldn't always be a network interface card
    // It should also probably be some generic provider
    struct NetworkInterfaceCard *card;
} WireTerminal;

void wireTerminal_connect(WireTerminal *term1, WireTerminal *term2);

typedef struct {
    struct NetworkInterfaceCard *receiver;
    Buffer data;
} WireTerminalReceiveData;

void handleWireTerminalReceive(EventData data);

// Different funcs so we can do things like
// - simulate different bit errors
// - instrument and log out info
