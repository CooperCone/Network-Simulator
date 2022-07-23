#pragma once

#include "devices/udpModule.h"
#include "buffer.h"

#include "event.h"

typedef struct EchoClient {
    u32 id;
    UDPModule *udp;
} EchoClient;

void echoClient_send(EchoClient *client, char *msg);

typedef struct {
    EchoClient *client;
    Buffer buffer;
} EchoClientEventData;

void handleEchoClientSend(EventData data);
void handleEchoClientReceive(EventData data);
