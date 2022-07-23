#include "devices/ipModule.h"

#include <assert.h>

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
    assert(false);
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
    assert(false);
}
