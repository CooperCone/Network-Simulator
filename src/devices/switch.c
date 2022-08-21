#include "devices/switch.h"

#include "log.h"

void handleSwitchPortReceive(EventData data) {
    Layer2InEventData *event = data;
    SwitchPort *port = (SwitchPort*)event->provider;

    SwitchReceiveData newEvent = {
        .device=port->switchDevice,
        .portNum=port->portNumber,
        .buffer=event->data
    };
    PostEvent(port->deviceID, GetFuncs(handleSwitchReceive), &newEvent,
        SwitchReceiveData, 0);
}

void handleSwitchPortSend(EventData data) {
    SwitchPortSendData *event = data;
    SwitchPort *port = event->port;

    // Send buffer over wire
    Layer1ReceiveData receiveEventData = {
        .data=event->buffer,
        .receiver=port->provider.layer1Provider->other
    };
    PostEvent(port->deviceID, GetFuncs(handleLayer1Receive), &receiveEventData, Layer1ReceiveData, 0);
}

bool findSwitchPortByMAC(SLNode *node, MACAddress addr) {
    SwitchPortTable *table = (SwitchPortTable*)node;

    return macAddr_cmp(table->macAddr, addr);
}

void handleSwitchReceive(EventData data) {
    SwitchReceiveData *event = data;
    Switch *device = event->device;

    EthernetHeader *header = (EthernetHeader*)event->buffer.data;

    MACStr srcStr;
    macAddr_toStr(header->srcAddr, srcStr);
    MACStr dstStr;
    macAddr_toStr(header->dstAddr, dstStr);
    log(device->deviceID, "Switch: Received traffic from: %s to %s", srcStr, dstStr);

    // Update table with sender
    SLNode *senderNode = sll_find(&(device->table->node), findSwitchPortByMAC, header->srcAddr);
    if (senderNode == NULL) {
        SwitchPortTable newEntry = {
            .portNum=event->portNum
        };
        macAddr_copy(newEntry.macAddr, header->srcAddr);

        device->table = (SwitchPortTable*)sll_prepend(&(device->table->node), &(newEntry.node), sizeof(newEntry));

        log(device->deviceID, "Switch: No src in table, so adding entry to port: %lu", event->portNum);
    }
    else {
        SwitchPortTable *updatedNode = (SwitchPortTable*)senderNode;
        updatedNode->portNum = event->portNum;

        log(device->deviceID, "Switch: Found src in table, updating to: %lu", event->portNum);
    }

    // Check if we have an entry
    SLNode *dstNode = sll_find(&(device->table->node), findSwitchPortByMAC, header->dstAddr);
    if (dstNode == NULL || macAddr_cmp(header->dstAddr, MACBroadcast)) {
        // No entry, so flood
        for (u64 i = 0; i < device->numPorts; i++) {
            if (i == event->portNum)
                continue;
            
            SwitchPort *port = device->ports + i;
            SwitchPortSendData outData = {
                .port=port,
                .buffer=event->buffer
            };
            PostEvent(device->deviceID, GetFuncs(handleSwitchPortSend), &outData, SwitchPortSendData, 0);
        }

        log(device->deviceID, "Switch: No dst in table, so flood to all other ports");
    }
    else {
        // We have an entry, so forward to the correct port
        SwitchPortTable *sendToNode = (SwitchPortTable*)dstNode;

        SwitchPort *port = device->ports + sendToNode->portNum;
        SwitchPortSendData outData = {
            .port=port,
            .buffer=event->buffer
        };
        PostEvent(device->deviceID, GetFuncs(handleSwitchPortSend), &outData, SwitchPortSendData, 0);
    
        log(device->deviceID, "Switch: Found dst in table, sending to port: %lu", port->portNumber);
    }
}
