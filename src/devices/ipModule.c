#include "devices/ipModule.h"

#include "devices/udpModule.h"

#include "log.h"
#include "timer.h"

#include <assert.h>
#include <stdio.h>

void handleIPModuleQueueOutEvent(EventData data) {
    IPQueueEventData *d = data;
    IPModule *module = d->module;
    BufferQueue *queue = &(d->module->outgoingQueue);

    Timer timer = timer_start();

    // If queue full, drop traffic
    if (queue->numBuffers == queue->maxBuffers) {
        log(module->deviceID, "IP: -> Queue Full, Dropping");
        return;
    }

    // Otherwise, add to queue and post a process event if
    // NIC is not busy
    bufferQueue_push(queue, d->data);

    u64 time = timer_stop(timer);

    if (!module->isBusy) {
        IPProcessEventData processEvent = {
            .module=module
        };
        PostEvent(handleIPProcessOutEvent, &processEvent, sizeof(processEvent), time);
    }
    log(module->deviceID, "IP: -> Queueing Data");
}

void handleIPModuleQueueInEvent(EventData data) {
    IPQueueEventData *d = data;
    IPModule *module = d->module;
    BufferQueue *queue = &(d->module->incomingQueue);

    Timer timer = timer_start();

    // If queue is full, drop traffic
    if (queue->numBuffers == queue->maxBuffers) {
        log(module->deviceID, "IP: <- Queue Full, Dropping");
        return;
    }

    // Otherwise, add to queue and post a process event if
    // NIC is not busy
    bufferQueue_push(queue, d->data);

    u64 time = timer_stop(timer);

    if (!module->isBusy) {
        IPProcessEventData processEvent = {
            .module=module
        };
        PostEvent(handleIPProcessInEvent, &processEvent, sizeof(processEvent), time);
    }

    log(module->deviceID, "IP: <- Queueing Data");
}

void handleIPProcessOutEvent(EventData data) {
    // Check if we need to do an arp request
    IPProcessEventData *e = data;
    IPModule *module = e->module;

    Timer timer = timer_start();

    if (module->outgoingQueue.numBuffers == 0) {
        module->isBusy = false;
        return;
    }

    Buffer buff = bufferQueue_pop(&(module->outgoingQueue));

    // TODO: Fill IP header

    u64 time = timer_stop(timer);

    // Send buffer over wire
    NICQueueEventData eventData = {
        .card=module->layer2Provider,
        .data=buff
    };
    PostEvent(handleNICQueueOutEvent, &eventData, sizeof(eventData), time);

    // Set is busy
    module->isBusy = true;

    PostEvent(handleIPProcessOutEvent, e, sizeof(IPProcessEventData), time);

    log(module->deviceID, "IP: -> Sending Data");
}

void handleIPProcessInEvent(EventData data) {
    // Check if we need to do an arp request
    IPProcessEventData *e = data;
    IPModule *module = e->module;

    Timer timer = timer_start();

    if (module->incomingQueue.numBuffers == 0) {
        module->isBusy = false;
        return;
    }

    Buffer buff = bufferQueue_pop(&(module->incomingQueue));

    u64 time = timer_stop(timer);

    // Figure out where to send data
    UDPQueueEventData newEvent = {
        .data=buff,
        .module=module->layer4Provider
    };
    PostEvent(handleUDPModuleQueueInEvent, &newEvent, sizeof(newEvent), time);

    // Set is busy
    module->isBusy = true;

    // Calculate the propagation and transmission delay
    // and create new nic process out event
    PostEvent(handleIPProcessInEvent, e, sizeof(IPProcessEventData), time);

    log(module->deviceID, "IP: <- Received Data, Forwarding Up");
}
