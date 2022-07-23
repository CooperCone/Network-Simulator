#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "event.h"
#include "devices/udpModule.h"
#include "devices/ipModule.h"
#include "devices/networkInterfaceCard.h"
#include "devices/wire.h"

static u64 time;
static EventQueue eventQueue;

int main(int argc, char **argv) {

    srand(5);

    // Configuration
    NetworkInterfaceCard card1 = {0};
    card1.outgoingQueue = bufferQueue_create(8);
    card1.incomingQueue = bufferQueue_create(8);
    card1.layer1Provider = stableWire_create();
    card1.layer1Provider->card = &card1;

    u8 mac1[] = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };
    memcpy(card1.address, mac1, sizeof(MACAddress));

    IPModule module1 = {0};
    module1.incomingQueue = bufferQueue_create(8);
    module1.outgoingQueue = bufferQueue_create(8);
    module1.layer2Provider = &card1;

    UDPModule udpModule1 = {0};
    udpModule1.incomingQueue = bufferQueue_create(8);
    udpModule1.outgoingQueue = bufferQueue_create(8);
    udpModule1.layer3Provider = &module1;

    NetworkInterfaceCard card2 = {0};
    card2.outgoingQueue = bufferQueue_create(8);
    card2.incomingQueue = bufferQueue_create(8);
    card2.layer1Provider = stableWire_create();
    card2.layer1Provider->card = &card2;

    u8 mac2[] = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };
    memcpy(card2.address, mac2, sizeof(MACAddress));

    layer1Provider_connect(card1.layer1Provider, card2.layer1Provider);

    // Set up initial traffic

    UDPQueueEventData queueEventData = {
        .module=&udpModule1
    };
    queueEventData.data.data = malloc(9);
    memcpy(queueEventData.data.data, "tmp data", 9);
    queueEventData.data.dataSize = 9;

    PostEvent(handleUDPModuleQueueOutEvent, &queueEventData, sizeof(queueEventData), 0);

    // Network Simulation
    while (eventQueue.numNodes > 0) {
        EventNode node = eventQueue_pop(&eventQueue);
        time = node.time;
        
        node.event->handleEvent(node.event->data);

        free(node.event->data);
        free(node.event);
    }
}

void PostEvent(HandleEvent handler, EventData data, u64 dataSize, u64 delay) {
    Event *event = calloc(1, sizeof(Event));
    event->handleEvent = handler;
    event->data = calloc(1, dataSize);
    memcpy(event->data, data, dataSize);

    EventNode node = {
        .event=event,
        .time=time + delay
    };

    eventQueue_push(&eventQueue, node);
}
