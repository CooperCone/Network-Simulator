#include "devices/ipModule.h"

#include "devices/udpModule.h"
#include "devices/arpModule.h"

#include "log.h"
#include "timer.h"
#include "util/math.h"
#include "event.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void handleIPModuleQueueOutEvent(EventData data) {
    IPInEventData *d = data;
    IPModule *module = (IPModule*)d->module;
    BufferQueue *queue = &(module->outgoingQueue);

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
            .module=module,
            .addr=d->addr
        };
        PostEvent(module->deviceID, GetFuncs(handleIPProcessOutEvent), &processEvent, IPProcessEventData, time);
    }
    log(module->deviceID, "IP: -> Queueing Data");
}

void handleIPModuleQueueInEvent(EventData data) {
    IPInEventData *d = data;
    IPModule *module = (IPModule*)d->module;
    BufferQueue *queue = &(module->incomingQueue);

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
        PostEvent(module->deviceID, GetFuncs(handleIPProcessInEvent), &processEvent, IPProcessEventData, time);
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

    // Set IP Header
    IPHeader header = {0};
    header.version = 4;
    header.headerLength = 5;
    header.datagramLength = (u16)sizeof(IPHeader) + (u16)buff.dataSize;
    header.timeToLive = 255;
    header.upperLayerProtocol = 0; // TODO: Set this based on the upper protocol
    header.checksum = 0;
    header.srcIPAddr = module->address;
    header.dstIPAddr = e->addr;

    // IP Header Checksum
    header.checksum = internetChecksum((u16*)&header, sizeof(header) / 2);

    Buffer newBuff = {
        .dataSize=header.datagramLength,
        .data=calloc(header.datagramLength, 1)
    };
    memcpy(newBuff.data, &header, sizeof(IPHeader));
    memcpy(newBuff.data + sizeof(IPHeader), buff.data, buff.dataSize);

    u64 time = timer_stop(timer);

    // Send buffer to arp module
    ARPRequestData eventData = {
        .addr=header.dstIPAddr,
        .buffer=newBuff,
        .module=module->arpModule
    };
    PostEvent(module->deviceID, GetFuncs(handleARPSendEvent), &eventData, ARPRequestData, time);

    // Set is busy
    module->isBusy = true;

    PostEvent(module->deviceID, GetFuncs(handleIPProcessOutEvent), e, IPProcessEventData, time);

    IPStr ipAddr;
    ipAddr_toStr(header.dstIPAddr, ipAddr);
    log(module->deviceID, "IP: -> Sending Data to %s", ipAddr);
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

    // Strip IP Header
    IPHeader *header = (IPHeader*)buff.data;

    // Verify Checksum
    u16 checksum = header->checksum;
    header->checksum = 0;
    u16 checksumComplement = internetChecksumComplement((u16*)header, sizeof(IPHeader) / 2);
    if (checksumComplement + checksum != 0xFFFF) {
        printf("Invalid IP Checksum!!!\n");
        return;
    }

    Buffer newBuff = {
        .dataSize=header->datagramLength - sizeof(IPHeader),
        .data=calloc(header->datagramLength - sizeof(IPHeader), 1)
    };
    memcpy(newBuff.data, buff.data + sizeof(IPHeader), newBuff.dataSize);

    u64 time = timer_stop(timer);

    // Figure out where to send data
    UDPInEventData newEvent = {
        .data=newBuff,
        .module=module->udpModule,
        .addr=header->srcIPAddr
    };
    PostEvent(module->deviceID, module->udpModule->onReceiveBuffer, &newEvent, UDPInEventData, time);

    // Set is busy
    module->isBusy = true;

    // Calculate the propagation and transmission delay
    // and create new nic process out event
    PostEvent(module->deviceID, GetFuncs(handleIPProcessInEvent), e, IPProcessEventData, time);

    log(module->deviceID, "IP: <- Received Data, Forwarding Up");
}
