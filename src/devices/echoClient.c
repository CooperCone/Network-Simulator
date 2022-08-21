#include "devices/echoClient.h"

#include "devices/udpModule.h"

#include "timer.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void echoClient_send(EchoClient *client, char *msg, IPAddress addr) {
    Timer timer = timer_start();

    Buffer buff = {0};
    buff.dataSize = strlen(msg) + 2;
    buff.data = calloc(buff.dataSize, 1);
    buff.data[0] = 0;
    memcpy(buff.data + 1, msg, strlen(msg));
    buff.data[buff.dataSize - 1] = '\0';

    u64 time = timer_stop(timer);

    printf("Echo Client %u sending: %s\n", client->id, msg);

    EchoClientEventData data = {
        .client=client,
        .buffer=buff,
        .addr=addr
    };

    PostEvent(client->id, GetFuncs(handleEchoClientSend), &data, EchoClientEventData, time);
}

void handleEchoClientSend(EventData data) {
    EchoClientEventData *d = data;

    UDPInEventData eventData = {
        .data=d->buffer,
        .module=d->client->udpModule,
        .addr=d->addr
    };

    PostEvent(d->client->id, d->client->udpModule->onSendBuffer, &eventData, UDPInEventData, 0);

    log(d->client->id, "Echo: -> Sending: %s", d->buffer.data + 1);
}

void handleEchoClientReceive(EventData data) {
    EchoClientEventData *eventData = data;
    Buffer buff = eventData->buffer;

    u8 echoNum = buff.data[0];

    printf("Echo Client %u received: %s\n", eventData->client->id, buff.data + 1);

    log(eventData->client->id, "Echo: <- Received: %s", buff.data + 1);

    if (echoNum == 0) {
        Timer timer = timer_start();

        // Resend data
        buff.data[0] = 1;

        u64 time = timer_stop(timer);

        EchoClientEventData data = {
            .client=eventData->client,
            .buffer=buff,
            .addr=eventData->addr
        };

        PostEvent(eventData->client->id, GetFuncs(handleEchoClientSend), &data, EchoClientEventData, time);

        log(eventData->client->id, "Echo: -> Echoing: %s", buff.data + 1);
    }
}
