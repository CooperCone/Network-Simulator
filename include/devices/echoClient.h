#pragma once

#include "devices/forward.h"
#include "layers/layer3.h"
#include "buffer.h"

#include "event.h"

typedef struct EchoClient {
    u32 id;
    struct UDPModule *udpModule;
} EchoClient;

void echoClient_send(EchoClient *client, char *msg, IPAddress addr);

typedef struct {
    EchoClient *client;
    Buffer buffer;
    IPAddress addr;
} EchoClientEventData;

DeclareEvent(handleEchoClientSend, EchoClientEventData);
DeclareEvent(handleEchoClientReceive, EchoClientEventData);
