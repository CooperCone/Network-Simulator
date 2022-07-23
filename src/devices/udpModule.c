#include "devices/udpModule.h"

#include <assert.h>

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
    assert(false);
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
    assert(false);
}
