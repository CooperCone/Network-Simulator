#include "devices/udpModule.h"

#include "devices/echoClient.h"
#include "devices/ipModule.h"

#include "log.h"
#include "timer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handleUDPModuleQueueOutEvent(EventData data) {
    UDPInEventData *d = data;
    UDPModule *module = (UDPModule*)d->module;
    BufferQueue *queue = &(module->outgoingQueue);

    Timer timer = timer_start();

    // If queue full, drop traffic
    if (queue->numBuffers == queue->maxBuffers) {
        log(module->deviceID, "UDP: -> Queue Full, Dropping");
        return;
    }

    // Otherwise, add to queue and post a process event if
    // NIC is not busy
    bufferQueue_push(queue, d->data);

    u64 time = timer_stop(timer);

    if (!module->isBusy) {
        UDPProcessEventData processEvent = {
            .module=module,
            .addr=d->addr
        };
        PostEvent(module->deviceID, GetFuncs(handleUDPProcessOutEvent), &processEvent, UDPProcessEventData, time);
    }

    log(module->deviceID, "UDP: -> Queueing Data");
}

void handleUDPModuleQueueInEvent(EventData data) {
    UDPInEventData *d = data;
    UDPModule *module = (UDPModule*)d->module;
    BufferQueue *queue = &(module->incomingQueue);

    Timer timer = timer_start();

    // If queue is full, drop traffic
    if (queue->numBuffers == queue->maxBuffers) {
        log(module->deviceID, "UDP: <- Queue Full, Dropping");
        return;
    }

    // Otherwise, add to queue and post a process event if
    // NIC is not busy
    bufferQueue_push(queue, d->data);

    u64 time = timer_stop(timer);

    if (!module->isBusy) {
        UDPProcessEventData processEvent = {
            .module=module,
            .addr=d->addr
        };
        PostEvent(module->deviceID, GetFuncs(handleUDPProcessInEvent), &processEvent, UDPProcessEventData, time);
    }

    log(module->deviceID, "UDP: <- Queueing Data");
}

void handleUDPProcessOutEvent(EventData data) {
    // Check if we need to do an arp request
    UDPProcessEventData *e = data;
    UDPModule *module = e->module;

    Timer timer = timer_start();

    if (module->outgoingQueue.numBuffers == 0) {
        module->isBusy = false;
        return;
    }

    Buffer buff = bufferQueue_pop(&(module->outgoingQueue));

    // Fill UDP Header
    UDPHeader header = {0};
    header.srcPort = 0; // TODO: Fill these
    header.dstPort = 0; // TODO: Fill these
    header.length = (u16)sizeof(UDPHeader) + (u16)buff.dataSize;
    header.checksum = 0;

    Buffer newBuff = {
        .dataSize=header.length,
        .data=calloc(header.length, 1)
    };
    memcpy(newBuff.data, &header, sizeof(UDPHeader));
    memcpy(newBuff.data + sizeof(UDPHeader), buff.data, buff.dataSize);

    u64 time = timer_stop(timer);

    // Send buffer over wire
    IPInEventData eventData = {
        .module=module->ipModule,
        .data=newBuff,
        .addr=e->addr
    };
    PostEvent(module->deviceID, module->ipModule->onSendBuffer, &eventData, IPInEventData, time);

    // Set is busy
    module->isBusy = true;

    PostEvent(module->deviceID, GetFuncs(handleUDPProcessOutEvent), e, UDPProcessEventData, time);

    log(module->deviceID, "UDP: -> Sending Data");
}

void handleUDPProcessInEvent(void *data) {
    // Check if we need to do an arp request
    UDPProcessEventData *e = data;
    UDPModule *module = e->module;

    Timer timer = timer_start();

    if (module->incomingQueue.numBuffers == 0) {
        module->isBusy = false;
        return;
    }

    Buffer buff = bufferQueue_pop(&(module->incomingQueue));

    // Strip Header
    UDPHeader *header = (UDPHeader*)buff.data;

    Buffer newBuff = {
        .dataSize=header->length - sizeof(UDPHeader),
        .data=calloc(header->length - sizeof(UDPHeader), 1)
    };
    memcpy(newBuff.data, buff.data + sizeof(UDPHeader), newBuff.dataSize);

    u64 time = timer_stop(timer);

    // Figure out where to send data
    EchoClientEventData newEvent = {
        .buffer=newBuff,
        .client=module->echoClient,
        .addr=e->addr
    };
    PostEvent(module->deviceID, GetFuncs(handleEchoClientReceive), &newEvent, EchoClientEventData, time);

    // Set is busy
    module->isBusy = true;

    // Calculate the propagation and transmission delay
    // and create new nic process out event
    PostEvent(module->deviceID, GetFuncs(handleUDPProcessInEvent), e, UDPProcessEventData, time);

    log(module->deviceID, "UDP: <- ReceivingData, Forwarding Up");
}
