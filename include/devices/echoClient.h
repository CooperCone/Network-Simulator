#pragma once

#include "layers/layer4.h"
#include "layers/layer3.h"
#include "buffer.h"

#include "event.h"

typedef struct EchoClient {
    u32 id;
    Layer4Provider *layer4Provider;
} EchoClient;

void echoClient_send(EchoClient *client, char *msg, IPAddress addr);

typedef struct {
    EchoClient *client;
    Buffer buffer;
    IPAddress addr;
} EchoClientEventData;

DeclareEvent(handleEchoClientSend);
DeclareEvent(handleEchoClientReceive);
