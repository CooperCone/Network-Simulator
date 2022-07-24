#include "devices/networkInterfaceCard.h"

#include "devices/ipModule.h"

#include "log.h"
#include "timer.h"
#include "util/math.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handleNICQueueOutEvent(EventData data) {
    NICQueueEventData *d = data;
    NetworkInterfaceCard *card = d->card;
    BufferQueue *queue = &(d->card->outgoingQueue);

    // If queue full, drop traffic
    if (queue->numBuffers == queue->maxBuffers) {
        log(card->deviceID, "NIC: -> Queue Full, Dropping");
        return;
    }

    // Otherwise, add to queue and post a process event if
    // NIC is not busy
    bufferQueue_push(queue, d->data);

    if (!card->isBusy) {
        NICProcessEventData processEvent = {
            .card=card
        };
        // No delay because the card currently isn't busy
        PostEvent(handleNICProcessOutEvent, &processEvent, sizeof(processEvent), 0);
    }

    log(card->deviceID, "NIC: -> Queueing data");
}

void handleNICQueueInEvent(EventData data) {
    NICQueueEventData *d = data;
    NetworkInterfaceCard *card = d->card;
    BufferQueue *queue = &(d->card->incomingQueue);

    Timer timer = timer_start();
    {
        // If queue is full, drop traffic
        if (queue->numBuffers == queue->maxBuffers) {
            log(card->deviceID, "NIC: <- Queue Full, Dropping");
            return;
        }

        // Otherwise, add to queue and post a process event if
        // NIC is not busy
        bufferQueue_push(queue, d->data);
    }
    u64 time = timer_stop(timer);

    if (!card->isBusy) {
        NICProcessEventData processEvent = {
            .card=card
        };
        PostEvent(handleNICProcessInEvent, &processEvent, sizeof(processEvent), time);
    }

    log(card->deviceID, "NIC: <- Queueing data");
}

void handleNICProcessOutEvent(EventData data) {
    // Check if we need to do an arp request
    NICProcessEventData *e = data;
    NetworkInterfaceCard *card = e->card;

    Timer timer = timer_start();

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

    u64 time = timer_stop(timer);

    u64 propagationDelay = unitToNano(card->layer1Provider->other->length) / (u64)(0.8 * speedOfLight);
    u64 transmissionDelay = unitToNano(ethBuff.dataSize * 8) / card->layer1Provider->other->bandwidth;

    time += propagationDelay + transmissionDelay;

    // Send buffer over wire
    Layer1ReceiveData receiveEventData = {0};
    receiveEventData.data = ethBuff;
    receiveEventData.receiver = card->layer1Provider->other;
    PostEvent(handleLayer1Receive, &receiveEventData, sizeof(receiveEventData), time);

    // Set is busy
    card->isBusy = true;

    PostEvent(handleNICProcessOutEvent, e, sizeof(NICProcessEventData), time);

    log(card->deviceID, "NIC: -> Sending Data");
}

void handleNICProcessInEvent(EventData data) {
    // Check if we need to do an arp request
    NICProcessEventData *e = data;
    NetworkInterfaceCard *card = e->card;

    Timer timer = timer_start();

    if (card->incomingQueue.numBuffers == 0) {
        card->isBusy = false;
        return;
    }

    Buffer buff = bufferQueue_pop(&(card->incomingQueue));

    // Unwrap ethernet header
    EthernetHeader header = {0};
    memcpy(&header, buff.data, sizeof(EthernetHeader));

    Buffer newBuff = {
        .dataSize=buff.dataSize - sizeof(EthernetHeader),
    };
    newBuff.data = malloc(newBuff.dataSize);

    memcpy(newBuff.data, buff.data + sizeof(EthernetHeader), newBuff.dataSize);

    u64 time = timer_stop(timer);

    // Figure out where to send data
    IPQueueEventData newEvent = {
        .data=newBuff,
        .module=card->layer3Provider
    };
    PostEvent(handleIPModuleQueueInEvent, &newEvent, sizeof(newEvent), time);

    // Set is busy
    card->isBusy = true;

    PostEvent(handleNICProcessInEvent, e, sizeof(NICProcessEventData), time);

    log(card->deviceID, "NIC: <- Received Data, Forwarding Up");
}
