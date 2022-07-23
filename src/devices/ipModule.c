#include "devices/ipModule.h"

#include "devices/udpModule.h"

#include <assert.h>
#include <stdio.h>

void handleIPModuleQueueOutEvent(EventData data) {
    IPQueueEventData *d = data;
    IPModule *module = d->module;
    BufferQueue *queue = &(d->module->outgoingQueue);

    // If queue full, drop traffic
    if (queue->numBuffers == queue->maxBuffers) {
        return;
    }

    // Otherwise, add to queue and post a process event if
    // NIC is not busy
    bufferQueue_push(queue, d->data);

    if (!module->isBusy) {
        IPProcessEventData processEvent = {
            .module=module
        };
        PostEvent(handleIPProcessOutEvent, &processEvent, sizeof(processEvent), 0);
    }
}

void handleIPModuleQueueInEvent(EventData data) {
    IPQueueEventData *d = data;
    IPModule *module = d->module;
    BufferQueue *queue = &(d->module->incomingQueue);

    // If queue is full, drop traffic
    if (queue->numBuffers == queue->maxBuffers) {
        return;
    }

    // Otherwise, add to queue and post a process event if
    // NIC is not busy
    bufferQueue_push(queue, d->data);

    if (!module->isBusy) {
        IPProcessEventData processEvent = {
            .module=module
        };
        PostEvent(handleIPProcessInEvent, &processEvent, sizeof(processEvent), 0);
    }
}

void handleIPProcessOutEvent(EventData data) {
    // Check if we need to do an arp request
    IPProcessEventData *e = data;
    IPModule *module = e->module;

    if (module->outgoingQueue.numBuffers == 0) {
        module->isBusy = false;
        return;
    }

    Buffer buff = bufferQueue_pop(&(module->outgoingQueue));

    // TODO: Fill IP header

    // Send buffer over wire
    NICQueueEventData eventData = {
        .card=module->layer2Provider,
        .data=buff
    };
    PostEvent(handleNICQueueOutEvent, &eventData, sizeof(eventData), 0);

    // Set is busy
    module->isBusy = true;

    PostEvent(handleIPProcessOutEvent, e, sizeof(IPProcessEventData), 0);
}

void handleIPProcessInEvent(EventData data) {
    // Check if we need to do an arp request
    IPProcessEventData *e = data;
    IPModule *module = e->module;

    if (module->incomingQueue.numBuffers == 0) {
        module->isBusy = false;
        return;
    }

    Buffer buff = bufferQueue_pop(&(module->incomingQueue));

    // Figure out where to send data
    UDPQueueEventData newEvent = {
        .data=buff,
        .module=module->layer4Provider
    };
    PostEvent(handleUDPModuleQueueInEvent, &newEvent, sizeof(newEvent), 0);

    // Set is busy
    module->isBusy = true;

    // Calculate the propagation and transmission delay
    // and create new nic process out event
    PostEvent(handleIPProcessInEvent, e, sizeof(IPProcessEventData), 0);

    printf("IP received data\n");
}
