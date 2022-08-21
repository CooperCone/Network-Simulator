#pragma once

#include "devices/forward.h"
#include "collections/linkedList.h"
#include "buffer.h"
#include "event.h"

#define ARPOpcode_Request 1
#define ARPOpcode_Reply 2

typedef struct {
    u16 hardwareAddressSpace;
    u16 protocolAddressSpace;
    u8 hardwareAddrByteLength; // Assumed to be 6
    u8 protocolAddrByteLength; // Assumed to be 4
    u16 opcode;
    MACAddress senderMacAddr;
    IPAddress senderIPAddr;
    MACAddress targetMacAddr;
    IPAddress targetIPAddr;
} ARPHeader;

typedef struct {
    SLNode node;

    IPAddress addr;
    Buffer buffer;
} ARPWaitingList;

typedef struct {
    SLNode node;

    IPAddress ipAddr;
    MACAddress macAddr;
} ARPEntry;

typedef struct {
    struct ARPModule *module;
    MACAddress dstMac;
    IPAddress dstIP;
} ARPSendData;
void arpEntrySendEntriesWithIP(SLNode *node, ARPSendData *data);
bool arpEntryFindByIP(SLNode *list, IPAddress *addr);

typedef struct ARPModule {
    struct IPModule *ipModule;
    struct NetworkInterfaceCard *nic;

    u64 deviceID;

    ARPEntry *arpEntries;
    ARPWaitingList *waitingList;
} ARPModule;

typedef struct {
    ARPModule *module;
    IPAddress addr;
    Buffer buffer;
} ARPRequestData;

DeclareEvent(handleARPSendEvent, ARPRequestData);

typedef struct {
    ARPModule *module;
    Buffer buffer;
} ARPResponseData;

DeclareEvent(handleARPResponseEvent, ARPResponseData);
