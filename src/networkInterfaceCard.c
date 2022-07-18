#include "networkInterfaceCard.h"

#include <stdio.h>
#include <stdlib.h>

void handleNICQueueOutEvent(EventData data) {
    NICQueueEventData *d = data;
    NetworkInterfaceCard *card = d->card;
    BufferQueue *queue = &(d->card->outgoingQueue);

    // If queue full, drop traffic
    if (queue->numBuffers == queue->maxBuffers) {
        return;
    }

    // Otherwise, add to queue and post a process event if
    // NIC is not busy
    bufferQueue_push(queue, d->data);

    if (!card->isBusy) {
        NICProcessEventData processEvent = {
            .card=card
        };
        PostEvent(handleNICProcessOutEvent, &processEvent, sizeof(processEvent), 0);
    }

    printf("Queueing data\n");
}

void handleNICQueueInEvent(EventData data) {
    NICQueueEventData *d = data;
    NetworkInterfaceCard *card = d->card;
    BufferQueue *queue = &(d->card->incomingQueue);

    // If queue is full, drop traffic
    if (queue->numBuffers == queue->maxBuffers) {
        return;
    }

    // Otherwise, add to queue and post a process event if
    // NIC is not busy
    bufferQueue_push(queue, d->data);

    if (!card->isBusy) {
        NICProcessEventData processEvent = {
            .card=card
        };
        PostEvent(handleNICProcessInEvent, &processEvent, sizeof(processEvent), 0);
    }

    printf("Queueing data for receive\n");
}

void handleNICProcessOutEvent(EventData data) {
    // Check if we need to do an arp request
    NICProcessEventData *e = data;
    NetworkInterfaceCard *card = e->card;

    if (card->outgoingQueue.numBuffers == 0) {
        card->isBusy = false;
        return;
    }

    Buffer buff = bufferQueue_pop(&(card->outgoingQueue));

    // Send buffer over wire
    WireTerminalReceiveData receiveEventData = {0};
    receiveEventData.data = buff;
    receiveEventData.receiver = card->terminal.other->card;
    PostEvent(handleWireTerminalReceive, &receiveEventData, sizeof(receiveEventData), 0);

    // Set is busy
    card->isBusy = true;

    // Calculate the propagation and transmission delay
    // and create new nic process out event

    PostEvent(handleNICProcessOutEvent, e, sizeof(NICProcessEventData), 0);

    printf("Sending data\n");
}

void handleNICProcessInEvent(EventData data) {
    // Check if we need to do an arp request
    NICProcessEventData *e = data;
    NetworkInterfaceCard *card = e->card;

    if (card->incomingQueue.numBuffers == 0) {
        card->isBusy = false;
        return;
    }

    Buffer buff = bufferQueue_pop(&(card->incomingQueue));

    // Figure out where to send data

    // Set is busy
    card->isBusy = true;

    // Calculate the propagation and transmission delay
    // and create new nic process out event
    PostEvent(handleNICProcessInEvent, e, sizeof(NICProcessEventData), 0);

    printf("NIC Received data\n");
}
