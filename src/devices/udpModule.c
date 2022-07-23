#include "devices/udpModule.h"

#include "devices/echoClient.h"

#include "log.h"

#include <assert.h>
#include <stdio.h>

void handleUDPModuleQueueOutEvent(EventData data) {
    UDPQueueEventData *d = data;
    UDPModule *module = d->module;
    BufferQueue *queue = &(d->module->outgoingQueue);

    // If queue full, drop traffic
    if (queue->numBuffers == queue->maxBuffers) {
        log(module->deviceID, "UDP: -> Queue Full, Dropping");
        return;
    }

    // Otherwise, add to queue and post a process event if
    // NIC is not busy
    bufferQueue_push(queue, d->data);

    if (!module->isBusy) {
        UDPProcessEventData processEvent = {
            .module=module
        };
        PostEvent(handleUDPProcessOutEvent, &processEvent, sizeof(processEvent), 0);
    }

    log(module->deviceID, "UDP: -> Queueing Data");
}

void handleUDPModuleQueueInEvent(EventData data) {
    UDPQueueEventData *d = data;
    UDPModule *module = d->module;
    BufferQueue *queue = &(d->module->incomingQueue);

    // If queue is full, drop traffic
    if (queue->numBuffers == queue->maxBuffers) {
        log(module->deviceID, "UDP: <- Queue Full, Dropping");
        return;
    }

    // Otherwise, add to queue and post a process event if
    // NIC is not busy
    bufferQueue_push(queue, d->data);

    if (!module->isBusy) {
        UDPProcessEventData processEvent = {
            .module=module
        };
        PostEvent(handleUDPProcessInEvent, &processEvent, sizeof(processEvent), 0);
    }

    log(module->deviceID, "UDP: <- Queueing Data");
}

void handleUDPProcessOutEvent(EventData data) {
    // Check if we need to do an arp request
    UDPProcessEventData *e = data;
    UDPModule *module = e->module;

    if (module->outgoingQueue.numBuffers == 0) {
        module->isBusy = false;
        return;
    }

    Buffer buff = bufferQueue_pop(&(module->outgoingQueue));

    // TODO: Fill IP header

    // Send buffer over wire
    IPQueueEventData eventData = {
        .module=module->layer3Provider,
        .data=buff
    };
    PostEvent(handleIPModuleQueueOutEvent, &eventData, sizeof(eventData), 0);

    // Set is busy
    module->isBusy = true;

    PostEvent(handleUDPProcessOutEvent, e, sizeof(IPProcessEventData), 0);

    log(module->deviceID, "UDP: -> Sending Data");
}

void handleUDPProcessInEvent(EventData data) {
    // Check if we need to do an arp request
    UDPProcessEventData *e = data;
    UDPModule *module = e->module;

    if (module->incomingQueue.numBuffers == 0) {
        module->isBusy = false;
        return;
    }

    Buffer buff = bufferQueue_pop(&(module->incomingQueue));

    // Figure out where to send data
    EchoClientEventData newEvent = {
        .buffer=buff,
        .client=module->layer7Provider
    };
    PostEvent(handleEchoClientReceive, &newEvent, sizeof(newEvent), 0);

    // Set is busy
    module->isBusy = true;

    // Calculate the propagation and transmission delay
    // and create new nic process out event
    PostEvent(handleUDPProcessInEvent, e, sizeof(UDPProcessEventData), 0);

    log(module->deviceID, "UDP: <- ReceivingData, Forwarding Up");
}
