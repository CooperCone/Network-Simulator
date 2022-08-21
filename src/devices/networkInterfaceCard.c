#include "devices/networkInterfaceCard.h"

#include "devices/ipModule.h"
#include "devices/udpModule.h"
#include "devices/arpModule.h"

#include "log.h"
#include "timer.h"
#include "util/math.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handleNICQueueOutEvent(EventData data) {
    NICQueueOutData *d = data;
    NetworkInterfaceCard *card = (NetworkInterfaceCard*)d->card;
    NICOutQueueList *queue = card->outgoingQueue;

    // If queue full, drop traffic
    // TODO: don't hardcode the max queue size
    if (sll_size(&(queue->node)) == 8) {
        log(card->deviceID, "NIC: -> Queue Full, Dropping");
        return;
    }

    // Otherwise, add to queue and post a process event if
    // NIC is not busy
    NICOutQueueList newNode = {
        .buffer=d->data,
        .protocol=d->higherProtocol
    };
    card->outgoingQueue = (NICOutQueueList*)sll_prepend(&(queue->node), &(newNode.node), sizeof(newNode));

    if (!card->isBusy) {
        NICProcessEventData processEvent = {
            .card=card
        };
        // No delay because the card currently isn't busy
        PostEvent(card->deviceID, GetFuncs(handleNICProcessOutEvent), &processEvent, NICProcessEventData, 0);
    }

    log(card->deviceID, "NIC: -> Queueing data");
}

void handleNICQueueInEvent(EventData data) {
    Layer2InEventData *d = data;
    NetworkInterfaceCard *card = (NetworkInterfaceCard*)d->provider;
    BufferQueue *queue = &(card->incomingQueue);

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
        PostEvent(card->deviceID, GetFuncs(handleNICProcessInEvent), &processEvent, NICProcessEventData, time);
    }

    log(card->deviceID, "NIC: <- Queueing data");
}

void handleNICProcessOutEvent(EventData data) {
    // Check if we need to do an arp request
    NICProcessEventData *e = data;
    NetworkInterfaceCard *card = e->card;

    Timer timer = timer_start();

    if (sll_size(&(card->outgoingQueue->node)) == 0) {
        card->isBusy = false;
        return;
    }

    NICOutQueueList lastNode = {0};
    card->outgoingQueue = (NICOutQueueList*)sll_popLast(&(card->outgoingQueue->node), &(lastNode.node), sizeof(lastNode));

    EthernetHeader ethHeader = {0};
    { // Fill Header
        memset(ethHeader.preamble, EthPreambleStart, 7);
        memset(ethHeader.preamble + 7, EthPreambleEnd, 1);

        // TODO: Be able to specify the destination mac address
        // memcpy(ethHeader.dstAddr, card->provider.layer1Provider->other->layer2Provider->address, sizeof(MACAddress));
        memcpy(ethHeader.srcAddr, card->address, sizeof(MACAddress));

        ethHeader.type = lastNode.protocol;
    }

    // Copy header, data, and crc
    Buffer ethBuff = {
        .dataSize=lastNode.buffer.dataSize + sizeof(EthernetHeader) + 4,
    };
    ethBuff.data = calloc(ethBuff.dataSize, 1);

    memcpy(ethBuff.data, &ethHeader, sizeof(EthernetHeader));
    memcpy(ethBuff.data + sizeof(EthernetHeader), lastNode.buffer.data, lastNode.buffer.dataSize);

    // Checksum
    u32 checksum = buffer_checksum32(ethBuff);
    memcpy(ethBuff.data + lastNode.buffer.dataSize + sizeof(EthernetHeader), &checksum, sizeof(checksum));

    u64 time = timer_stop(timer);

    u64 propagationDelay = unitToNano(card->provider.layer1Provider->other->length) / (u64)(0.8 * speedOfLight);
    u64 transmissionDelay = unitToNano(ethBuff.dataSize * 8) / card->provider.layer1Provider->other->bandwidth;

    time += propagationDelay + transmissionDelay;

    // Send buffer over wire
    Layer1ReceiveData receiveEventData = {0};
    receiveEventData.data = ethBuff;
    receiveEventData.receiver = card->provider.layer1Provider->other;

    PostEvent(card->deviceID, GetFuncs(handleLayer1Receive), &receiveEventData, Layer1ReceiveData, time);

    // Set is busy
    card->isBusy = true;

    PostEvent(card->deviceID, GetFuncs(handleNICProcessOutEvent), e, NICProcessEventData, time);

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

    // Verify checksum
    u32 checksum = 0;
    memcpy(&checksum, buff.data + buff.dataSize - 4, 4);

    memset(buff.data + buff.dataSize - 4, 0, 4);

    u32 calcChecksum = buffer_checksum32(buff);

    if (calcChecksum != checksum) {
        // Drop packet because checksums didnt match
        log(card->deviceID, "NIC: <- Invalid Checksum, Dropping Packet");
        return;
    }

    // Unwrap ethernet header
    EthernetHeader header = {0};
    memcpy(&header, buff.data, sizeof(EthernetHeader));

    Buffer newBuff = {
        .dataSize=buff.dataSize - sizeof(EthernetHeader) - 4,
    };
    newBuff.data = malloc(newBuff.dataSize);

    memcpy(newBuff.data, buff.data + sizeof(EthernetHeader), newBuff.dataSize);

    u64 time = timer_stop(timer);

    // Figure out where to send data
    if (header.type == EtherType_ARP) {
        ARPResponseData arpEvent = {
            .module=card->arpModule,
            .buffer=newBuff
        };
        PostEvent(card->deviceID, GetFuncs(handleARPResponseEvent), &arpEvent, ARPResponseData, time);
    }
    else if (header.type == EtherType_IPv4) {
        IPInEventData newEvent = {
            .data=newBuff,
            .module=card->ipModule
        };
        PostEvent(card->deviceID, card->ipModule->onReceiveBuffer, &newEvent, IPInEventData, time);
    }    

    // Set is busy
    card->isBusy = true;

    PostEvent(card->deviceID, GetFuncs(handleNICProcessInEvent), e, NICProcessEventData, time);

    log(card->deviceID, "NIC: <- Received Data, Forwarding Up");
}
