#include "devices/echoClient.h"

#include "timer.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void echoClient_send(EchoClient *client, char *msg) {
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
        .buffer=buff
    };

    PostEvent(client->id, GetFuncs(handleEchoClientSend), &data, sizeof(data), time);
}

void handleEchoClientSend(EventData data) {
    EchoClientEventData *d = data;

    Layer4InEventData eventData = {
        .data=d->buffer,
        .layer4=d->client->layer4Provider
    };

    PostEvent(d->client->id, d->client->layer4Provider->onSendBuffer, &eventData, sizeof(eventData), 0);

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
            .buffer=buff
        };

        PostEvent(eventData->client->id, GetFuncs(handleEchoClientSend), &data, sizeof(data), time);

        log(eventData->client->id, "Echo: -> Echoing: %s", buff.data + 1);
    }
}
