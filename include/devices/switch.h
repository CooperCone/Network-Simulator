#pragma once

#include "layers/layer2.h"
#include "layers/layer1.h"
#include "devices/forward.h"

#include "collections/linkedList.h"

#include "event.h"

// TODO: Should this really be a layer2 provider?
// This implies that a switch port is a layer2
// device, but a switch is not, when this is really
// not the case
typedef struct {
    Layer2Provider provider;

    struct Switch *switchDevice;

    u64 deviceID;
    u64 portNumber;
} SwitchPort;

DeclareEvent(handleSwitchPortReceive, Layer2InEventData)

typedef struct {
    SwitchPort *port;
    Buffer buffer;
} SwitchPortSendData;
DeclareEvent(handleSwitchPortSend, SwitchPortSendData)

typedef struct {
    SLNode node;

    u64 portNum;
    MACAddress macAddr;
} SwitchPortTable;

bool findSwitchPortByMAC(SLNode *node, MACAddress addr);

typedef struct Switch {
    u64 deviceID;

    SwitchPortTable *table;

    u64 numPorts;
    SwitchPort *ports;    
} Switch;

typedef struct {
    Switch *device;
    u64 portNum;
    Buffer buffer;
} SwitchReceiveData;

DeclareEvent(handleSwitchReceive, SwitchReceiveData)
