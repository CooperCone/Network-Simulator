#include "devices/echoClient.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void echoClient_send(EchoClient *client, char *msg) {

    Buffer buff = {0};
    buff.dataSize = strlen(msg) + 2;
    buff.data = calloc(buff.dataSize, 1);
    buff.data[0] = 0;
    memcpy(buff.data + 1, msg, strlen(msg));
    buff.data[buff.dataSize - 1] = '\0';

    EchoClientEventData data = {
        .client=client,
        .buffer=buff
    };

    PostEvent(handleEchoClientSend, &data, sizeof(data), 0);
}

void handleEchoClientSend(EventData data) {
    EchoClientEventData *d = data;

    UDPQueueEventData eventData = {
        .data=d->buffer,
        .module=d->client->udp
    };

    PostEvent(handleUDPModuleQueueOutEvent, &eventData, sizeof(eventData), 0);
}

void handleEchoClientReceive(EventData data) {
    EchoClientEventData *eventData = data;
    Buffer buff = eventData->buffer;

    u8 echoNum = buff.data[0];

    printf("Echo Client %u received: %s\n", eventData->client->id, buff.data + 1);

    if (echoNum == 0) {
        // Resend data
        buff.data[0] = 1;

        EchoClientEventData data = {
            .client=eventData->client,
            .buffer=buff
        };

        PostEvent(handleEchoClientSend, &data, sizeof(data), 0);
    }
}
