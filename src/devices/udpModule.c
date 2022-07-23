#include "devices/udpModule.h"

#include <assert.h>
#include <stdio.h>

void handleUDPModuleQueueOutEvent(EventData data) {
    UDPQueueEventData *d = data;
    UDPModule *module = d->module;
    BufferQueue *queue = &(d->module->outgoingQueue);

    // If queue full, drop traffic
    if (queue->numBuffers == queue->maxBuffers) {
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
}

void handleUDPModuleQueueInEvent(EventData data) {
    UDPQueueEventData *d = data;
    UDPModule *module = d->module;
    BufferQueue *queue = &(d->module->incomingQueue);

    // If queue is full, drop traffic
    if (queue->numBuffers == queue->maxBuffers) {
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

    printf("Got %d bytes\n", buff.dataSize);

    // TODO: IP Header
    printf("Data: %s\n", buff.data);

    // TODO:Figure out where to send data

    // Set is busy
    module->isBusy = true;

    // Calculate the propagation and transmission delay
    // and create new nic process out event
    PostEvent(handleUDPProcessInEvent, e, sizeof(UDPProcessEventData), 0);

    printf("UDP received data\n");
}
