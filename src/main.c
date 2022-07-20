#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "physicalLayer.h"
#include "event.h"
#include "networkInterfaceCard.h"

static u64 time;
static EventQueue eventQueue;

int main(int argc, char **argv) {

    // Configuration
    NetworkInterfaceCard card1 = {
        .outgoingQueue=bufferQueue_create(8),
        .incomingQueue=bufferQueue_create(8),
        .terminal.card=&card1,
    };
    u8 mac1[] = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };
    memcpy(card1.address, mac1, sizeof(MACAddress));
    NetworkInterfaceCard card2 = {
        .outgoingQueue=bufferQueue_create(8),
        .incomingQueue=bufferQueue_create(8),
        .terminal.card=&card2,
    };
    u8 mac2[] = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };
    memcpy(card1.address, mac2, sizeof(MACAddress));
    wireTerminal_connect(&(card1.terminal), &(card2.terminal));

    // Set up initial traffic

    NICQueueEventData queueEventData = {
        .card=&card1,
    };
    queueEventData.data.data = malloc(9);
    memcpy(queueEventData.data.data, "tmp data", 9);
    queueEventData.data.dataSize = 9;

    PostEvent(handleNICQueueOutEvent, &queueEventData, sizeof(queueEventData), 0);

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
