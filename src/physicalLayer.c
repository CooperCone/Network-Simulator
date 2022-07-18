#include "physicalLayer.h"

#include <stdio.h>

#include "networkInterfaceCard.h"

void wireTerminal_connect(WireTerminal *term1, WireTerminal *term2) {
    term1->other = term2;
    term2->other = term1;
}

void handleWireTerminalReceive(EventData data) {
    // Send card to incoming queue
    WireTerminalReceiveData *d = data;

    NICQueueEventData newData = {
        .card=d->receiver,
        .data=d->data
    };

    printf("Posting event\n");

    PostEvent(handleNICQueueInEvent, &newData, sizeof(newData), 0);

    printf("Receiving data\n");
}
