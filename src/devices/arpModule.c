#include "devices/arpModule.h"

#include "layers/layer2.h"
#include "devices/networkInterfaceCard.h"
#include "devices/ipModule.h"

#include "log.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void arpEntrySendEntriesWithIP(SLNode *node, ARPSendData *data) {
    ARPWaitingList *waitingList = (ARPWaitingList*)node;
    if (waitingList->addr.w != data->dstIP.w) {
        return;
    }

    // Found a waiting node with the right ip address
    NICQueueOutData eventData = {
        .card=data->module->nic,
        .data=waitingList->buffer,
        .higherProtocol=EtherType_IPv4,
    };
    macAddr_copy(eventData.dstAddr, data->dstMac);
    PostEvent(data->module->deviceID, GetFuncs(handleNICQueueOutEvent), &eventData, NICQueueOutData, 0);
}

bool arpEntryFindByIP(SLNode *list, IPAddress *addr) {
    ARPEntry *entry = (ARPEntry*)list;

    return entry->ipAddr.w == addr->w;
}

void handleARPSendEvent(EventData data) {
    ARPRequestData *d = data;
    ARPModule *module = d->module;

    // // For now, just forward data to NIC
    // NICQueueOutData eventData = {
    //     .card=module->nic,
    //     .data=d->buffer,
    //     .higherProtocol=EtherType_IPv4
    // };
    // PostEvent(module->deviceID, GetFuncs(handleNICQueueOutEvent), &eventData, NICQueueOutData, 0);

    // Check if we have the ip address in the table
    SLNode *node = sll_find(&(module->arpEntries->node), arpEntryFindByIP, &(d->addr));
    if (node == NULL) {
        // If not, store it in requests data struture
        ARPWaitingList newWaitingNode = {
            .addr=d->addr,
            .buffer=d->buffer,
        };
        module->waitingList = (ARPWaitingList*)sll_prepend(&(module->waitingList->node), &(newWaitingNode.node), sizeof(newWaitingNode));

        // Send an arp request out
        ARPHeader header = {
            .hardwareAddressSpace=1, // TODO: Don't hardcode this
            .protocolAddressSpace=EtherType_IPv4, // TODO: Figure out what it actually is
            .hardwareAddrByteLength=6,
            .protocolAddrByteLength=4,
            .opcode=ARPOpcode_Request,
            .senderIPAddr=module->ipModule->address,
            .targetIPAddr=d->addr,
        };
        macAddr_copy(header.senderMacAddr, module->nic->address);

        Buffer arpBuffer;
        arpBuffer.dataSize = sizeof(header);
        arpBuffer.data = malloc(arpBuffer.dataSize);
        memcpy(arpBuffer.data, &header, sizeof(header));

        NICQueueOutData arpRequestEvent = {
            .card=module->nic,
            .data=arpBuffer,
            .higherProtocol=EtherType_ARP
        };
        PostEvent(module->deviceID, GetFuncs(handleNICQueueOutEvent), &arpRequestEvent, NICQueueOutData, 0);
    
        IPStr str;
        ipAddr_toStr(d->addr, str);
        log(module->deviceID, "ARP: Didn't find entry for %s, sending out arp request.", str);
    }
    else {
        // If so, send out the data
        ARPEntry *entry = (ARPEntry*)node;

        NICQueueOutData eventData = {
            .card=module->nic,
            .data=d->buffer,
            .higherProtocol=EtherType_IPv4
        };
        macAddr_copy(eventData.dstAddr, entry->macAddr);
        PostEvent(module->deviceID, GetFuncs(handleNICQueueOutEvent), &eventData, NICQueueOutData, 0);

        IPStr ipStr;
        ipAddr_toStr(entry->ipAddr, ipStr);
        MACStr macStr;
        macAddr_toStr(entry->macAddr, macStr);
        log(module->deviceID, "ARP: found entry for %s: %s. Sending out data.", ipStr, macStr);
    }
}

void handleARPResponseEvent(EventData data) {
    ARPResponseData *d = data;
    ARPModule *module = d->module;
    Buffer buff = d->buffer;

    ARPHeader *header = (ARPHeader*)buff.data;

    // Algorithm from rfc 826
    // https://www.rfc-editor.org/rfc/rfc826.html
    if (header->hardwareAddressSpace != 1) { // TODO: Dont hardcode this
        log(module->deviceID, "ARP: <- Unexpected hardware address space %d", header->hardwareAddressSpace);
        return;
    }

    if (header->hardwareAddrByteLength != 6) {
        log(module->deviceID, "ARP: <- Unexpected hardware addr length %d", header->hardwareAddrByteLength);
        return;
    }

    if (header->protocolAddressSpace != EtherType_IPv4) {
        log(module->deviceID, "ARP: <- Unknown protocol %d", header->protocolAddressSpace);
        return;
    }

    if (header->protocolAddrByteLength != 4) {
        log(module->deviceID, "ARP: <- Unexpected protocol addr length %d", header->protocolAddrByteLength);
        return;
    }

    bool mergeFlag = false;

    ARPEntry *foundEntry = (ARPEntry*)sll_find(&(module->arpEntries->node), arpEntryFindByIP, &(header->senderIPAddr));
    if (foundEntry != NULL) {
        macAddr_copy(foundEntry->macAddr, header->senderMacAddr);
        mergeFlag = true;

        IPStr ipStr;
        ipAddr_toStr(header->senderIPAddr, ipStr);
        MACStr macStr;
        macAddr_toStr(header->senderMacAddr, macStr);
        log(module->deviceID, "ARP: Updating entry for %s to %s", ipStr, macStr);
    }

    if (header->targetIPAddr.w != module->ipModule->address.w) {
        log(module->deviceID, "ARP: arp request wasn't to us.");
        return;
    }

    if (!mergeFlag) {
        ARPEntry newEntry = {
            .ipAddr=header->senderIPAddr,
        };
        macAddr_copy(newEntry.macAddr, header->senderMacAddr);
        module->arpEntries = (ARPEntry*)sll_prepend(&(module->arpEntries->node), &(newEntry.node), sizeof(newEntry));

        IPStr ipStr;
        ipAddr_toStr(header->senderIPAddr, ipStr);
        MACStr macStr;
        macAddr_toStr(header->senderMacAddr, macStr);
        log(module->deviceID, "ARP: Adding entry for %s to %s", ipStr, macStr);
    }

    if (header->opcode == ARPOpcode_Request) {
        IPAddress tmpIP = header->senderIPAddr;
        header->senderIPAddr = header->targetIPAddr;
        header->targetIPAddr = tmpIP;

        macAddr_copy(header->targetMacAddr, header->senderMacAddr);
        macAddr_copy(header->senderMacAddr, module->nic->address);

        header->opcode = ARPOpcode_Reply;
        
        Buffer newBuff;
        newBuff.dataSize = sizeof(ARPHeader);
        newBuff.data = malloc(newBuff.dataSize);
        memcpy(newBuff.data, header, newBuff.dataSize);

        NICQueueOutData arpRequestEvent = {
            .card=module->nic,
            .data=newBuff,
            .higherProtocol=EtherType_ARP
        };
        PostEvent(module->deviceID, GetFuncs(handleNICQueueOutEvent), &arpRequestEvent, NICQueueOutData, 0);
    
        MACStr macStr;
        macAddr_toStr(module->nic->address, macStr);
        log(module->deviceID, "ARP: Got request, so responding with %s", macStr);
    }
    else {
        // Look for all waiting buffers and send them with the new mac addr
        MACStr macStr;
        macAddr_toStr(module->nic->address, macStr);
        log(module->deviceID, "ARP: Got response, so loop through all buffers and send them.");

        ARPSendData sendData = {
            .module=module,
            .dstIP=header->senderIPAddr,
        };
        macAddr_copy(sendData.dstMac, header->senderMacAddr);
        sll_foreach(&(module->waitingList->node), arpEntrySendEntriesWithIP, &sendData);
    }    
}
