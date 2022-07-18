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
    NetworkInterfaceCard card2 = {
        .outgoingQueue=bufferQueue_create(8),
        .incomingQueue=bufferQueue_create(8),
        .terminal.card=&card2,
    };
    wireTerminal_connect(&(card1.terminal), &(card2.terminal));

    // Set up initial traffic

    NICQueueEventData queueEventData = {
        .card=&card1,
        .data=(Buffer){0}
    };

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
