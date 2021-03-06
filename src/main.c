#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "event.h"
#include "timer.h"
#include "util/math.h"
#include "devices/udpModule.h"
#include "devices/ipModule.h"
#include "devices/networkInterfaceCard.h"
#include "devices/echoClient.h"
#include "devices/wire.h"

static u64 time;
static EventQueue eventQueue;

int main(int argc, char **argv) {

    srand(7);

    timer_init();

    // Configuration
    NetworkInterfaceCard card1 = {0};
    card1.deviceID = 1;
    card1.outgoingQueue = bufferQueue_create(8);
    card1.incomingQueue = bufferQueue_create(8);
    card1.provider.layer1Provider = stableWire_create(3, megaToUnit(300));
    card1.provider.layer1Provider->layer2Provider = &(card1.provider);
    card1.provider.onReceiveBuffer = handleNICQueueInEvent;
    card1.provider.onSendBuffer = handleNICQueueOutEvent;

    u8 mac1[] = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };
    memcpy(card1.address, mac1, sizeof(MACAddress));

    IPModule module1 = {0};
    module1.deviceID = 1;
    module1.incomingQueue = bufferQueue_create(8);
    module1.outgoingQueue = bufferQueue_create(8);
    module1.provider.layer2Provider = &(card1.provider);
    module1.provider.onReceiveBuffer = handleIPModuleQueueInEvent;
    module1.provider.onSendBuffer = handleIPModuleQueueOutEvent;

    card1.provider.layer3Provider = &(module1.provider);

    UDPModule udpModule1 = {0};
    udpModule1.deviceID = 1;
    udpModule1.incomingQueue = bufferQueue_create(8);
    udpModule1.outgoingQueue = bufferQueue_create(8);
    udpModule1.layer3Provider = &(module1.provider);

    module1.provider.layer4Provider = &udpModule1;

    EchoClient echo1 = {0};
    echo1.id = 1;
    echo1.udp = &udpModule1;

    udpModule1.layer7Provider = &echo1;

    NetworkInterfaceCard card2 = {0};
    card2.deviceID = 2;
    card2.outgoingQueue = bufferQueue_create(8);
    card2.incomingQueue = bufferQueue_create(8);
    card2.provider.layer1Provider = stableWire_create(3, megaToUnit(300));
    card2.provider.layer1Provider->layer2Provider = &(card2.provider);
    card2.provider.onReceiveBuffer = handleNICQueueInEvent;
    card2.provider.onSendBuffer = handleNICQueueOutEvent;

    IPModule module2 = {0};
    module2.deviceID = 2;
    module2.incomingQueue = bufferQueue_create(8);
    module2.outgoingQueue = bufferQueue_create(8);
    module2.provider.layer2Provider = &(card2.provider);
    module2.provider.onReceiveBuffer = handleIPModuleQueueInEvent;
    module2.provider.onSendBuffer = handleIPModuleQueueOutEvent;

    card2.provider.layer3Provider = &(module2.provider);

    UDPModule udpModule2 = {0};
    udpModule2.deviceID = 2;
    udpModule2.incomingQueue = bufferQueue_create(8);
    udpModule2.outgoingQueue = bufferQueue_create(8);
    udpModule2.layer3Provider = &(module2.provider);

    module2.provider.layer4Provider = &udpModule2;

    EchoClient echo2 = {0};
    echo2.id = 2;
    echo2.udp = &udpModule2;

    udpModule2.layer7Provider = &echo2;

    u8 mac2[] = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };
    memcpy(card2.address, mac2, sizeof(MACAddress));

    layer1Provider_connect(card1.provider.layer1Provider, card2.provider.layer1Provider);

    // Set up initial traffic

    echoClient_send(&echo1, "This is a message");

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

u64 CurTime() {
    return time;
}
