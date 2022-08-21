#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "event.h"
#include "timer.h"
#include "log.h"
#include "util/math.h"
#include "devices/udpModule.h"
#include "devices/ipModule.h"
#include "devices/networkInterfaceCard.h"
#include "devices/echoClient.h"
#include "devices/arpModule.h"
#include "devices/wire.h"
#include "devices/switch.h"

static u64 time;
static EventQueue eventQueue;

int main(int argc, char **argv) {

    srand(7);

    timer_init();

    // Configuration
    NetworkInterfaceCard card1 = {0};
    card1.deviceID = 1;
    card1.incomingQueue = bufferQueue_create(8);
    card1.provider.layer1Provider = stableWire_create(1, 3, megaToUnit(300));
    card1.provider.layer1Provider->layer2Provider = &(card1.provider);
    card1.provider.onReceiveBuffer = GetFuncs(handleNICQueueInEvent);

    u8 mac1[] = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };
    memcpy(card1.address, mac1, sizeof(MACAddress));

    ARPModule arp1 = {0};
    arp1.nic = &card1;
    arp1.deviceID = 1;

    card1.arpModule = &arp1;

    IPModule module1 = {0};
    module1.deviceID = 1;
    module1.incomingQueue = bufferQueue_create(8);
    module1.outgoingQueue = bufferQueue_create(8);
    module1.address = ipAddr_fromStr("192.168.1.1");
    module1.arpModule = &arp1;
    module1.onReceiveBuffer = GetFuncs(handleIPModuleQueueInEvent);
    module1.onSendBuffer = GetFuncs(handleIPModuleQueueOutEvent);

    arp1.ipModule = &module1;
    card1.ipModule = &module1;

    UDPModule udpModule1 = {0};
    udpModule1.deviceID = 1;
    udpModule1.incomingQueue = bufferQueue_create(8);
    udpModule1.outgoingQueue = bufferQueue_create(8);
    udpModule1.ipModule = &module1;
    udpModule1.onReceiveBuffer = GetFuncs(handleUDPModuleQueueInEvent);
    udpModule1.onSendBuffer = GetFuncs(handleUDPModuleQueueOutEvent);

    module1.udpModule = &udpModule1;

    EchoClient echo1 = {0};
    echo1.id = 1;
    echo1.udpModule = &udpModule1;

    udpModule1.echoClient = &echo1;

    NetworkInterfaceCard card2 = {0};
    card2.deviceID = 2;
    card2.incomingQueue = bufferQueue_create(8);
    card2.provider.layer1Provider = stableWire_create(1, 3, megaToUnit(300));
    card2.provider.layer1Provider->layer2Provider = &(card2.provider);
    card2.provider.onReceiveBuffer = GetFuncs(handleNICQueueInEvent);

    ARPModule arp2 = {0};
    arp2.nic = &card2;
    arp2.deviceID = 2;

    card2.arpModule = &arp2;

    IPModule module2 = {0};
    module2.deviceID = 2;
    module2.incomingQueue = bufferQueue_create(8);
    module2.outgoingQueue = bufferQueue_create(8);
    module2.address = ipAddr_fromStr("192.168.1.2");
    module2.arpModule = &arp2;
    module2.onReceiveBuffer = GetFuncs(handleIPModuleQueueInEvent);
    module2.onSendBuffer = GetFuncs(handleIPModuleQueueOutEvent);

    arp2.ipModule = &module2;
    card2.ipModule = &module2;

    UDPModule udpModule2 = {0};
    udpModule2.deviceID = 2;
    udpModule2.incomingQueue = bufferQueue_create(8);
    udpModule2.outgoingQueue = bufferQueue_create(8);
    udpModule2.ipModule = &module2;
    udpModule2.onReceiveBuffer = GetFuncs(handleUDPModuleQueueInEvent);
    udpModule2.onSendBuffer = GetFuncs(handleUDPModuleQueueOutEvent);

    module2.udpModule = &udpModule2;

    EchoClient echo2 = {0};
    echo2.id = 2;
    echo2.udpModule = &udpModule2;

    udpModule2.echoClient = &echo2;

    u8 mac2[] = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };
    memcpy(card2.address, mac2, sizeof(MACAddress));

    Switch switch1 = {0};
    switch1.deviceID = 3;
    switch1.numPorts = 2;
    switch1.ports = malloc(sizeof(SwitchPort) * switch1.numPorts);
    
    switch1.ports[0].deviceID = 3;
    switch1.ports[0].portNumber = 0;
    switch1.ports[0].switchDevice = &switch1;
    switch1.ports[0].provider.layer1Provider = stableWire_create(1, 3, megaToUnit(300));
    switch1.ports[0].provider.layer1Provider->layer2Provider = &(switch1.ports[0].provider);
    switch1.ports[0].provider.onReceiveBuffer = GetFuncs(handleSwitchPortReceive);

    switch1.ports[1].deviceID = 3;
    switch1.ports[1].portNumber = 1;
    switch1.ports[1].switchDevice = &switch1;
    switch1.ports[1].provider.layer1Provider = stableWire_create(1, 3, megaToUnit(300));
    switch1.ports[1].provider.layer1Provider->layer2Provider = &(switch1.ports[1].provider);
    switch1.ports[1].provider.onReceiveBuffer = GetFuncs(handleSwitchPortReceive);

    layer1Provider_connect(card1.provider.layer1Provider, switch1.ports[0].provider.layer1Provider);
    layer1Provider_connect(card2.provider.layer1Provider, switch1.ports[1].provider.layer1Provider);

    // Set up initial traffic
    
    IPAddress dst = ipAddr_fromStr("192.168.1.2");
    echoClient_send(&echo1, "This is a message", dst);

    // Network Simulation
    while (eventQueue.numNodes > 0) {
        EventNode node = eventQueue_pop(&eventQueue);
        time = node.time;

        logEvent(node.event->deviceID, node.event->eventFuncs.getName());

        node.event->eventFuncs.handleEvent(node.event->data);

        free(node.event->data);
        free(node.event);
    }
}

void _postEvent(u64 deviceID, EventFuncs eventFuncs, EventData data, u64 dataSize, char *eventName, u64 delay, char *file, int line)
{
    char *expectedName = eventFuncs.getEventName();
    if (strcmp(expectedName, eventName) != 0) {
        char *handlerName = eventFuncs.getName();
        printf("EVENT ERROR!!!\n");
        printf("Expected event: %s to have event type: %s\n", handlerName, expectedName);
        printf("Instead, it was: %s\n", eventName);
        printf("Came from %s:%d\n", file, line);
        assert(false);
    }

    Event *event = calloc(1, sizeof(Event));
    event->deviceID = deviceID;
    event->eventFuncs = eventFuncs;
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
