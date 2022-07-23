#include "devices/networkInterfaceCard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    EthernetHeader ethHeader = {0};
    { // Fill Header
        memset(ethHeader.preamble, EthPreambleStart, 7);
        memset(ethHeader.preamble + 7, EthPreambleEnd, 1);

        memcpy(ethHeader.dstAddr, card->layer1Provider->other->card->address, sizeof(MACAddress));
        memcpy(ethHeader.srcAddr, card->address, sizeof(MACAddress));

        memcpy(ethHeader.type, (u16*)&(buff.dataSize), 2);
    }

    // TODO: CRC

    // Copy header, data, and crc
    Buffer ethBuff = {
        .dataSize=buff.dataSize + sizeof(EthernetHeader),
    };
    ethBuff.data = malloc(ethBuff.dataSize);

    memcpy(ethBuff.data, &ethHeader, sizeof(EthernetHeader));
    memcpy(ethBuff.data + sizeof(EthernetHeader), buff.data, buff.dataSize);

    // Send buffer over wire
    Layer1ReceiveData receiveEventData = {0};
    receiveEventData.data = ethBuff;
    receiveEventData.receiver = card->layer1Provider->other;
    PostEvent(handleLayer1Receive, &receiveEventData, sizeof(receiveEventData), 0);

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

    printf("Got %d bytes\n", buff.dataSize);

    // Unwrap ethernet header
    EthernetHeader header = {0};
    memcpy(&header, buff.data, sizeof(EthernetHeader));

    Buffer newBuff = {
        .dataSize=buff.dataSize - sizeof(EthernetHeader),
    };
    newBuff.data = malloc(newBuff.dataSize);

    memcpy(newBuff.data, buff.data + sizeof(EthernetHeader), newBuff.dataSize);

    printf("Data: %s\n", newBuff.data);

    // Figure out where to send data

    // Set is busy
    card->isBusy = true;

    // Calculate the propagation and transmission delay
    // and create new nic process out event
    PostEvent(handleNICProcessInEvent, e, sizeof(NICProcessEventData), 0);

    printf("NIC Received data\n");
}
