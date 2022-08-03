#include "layers/layer1.h"

#include "layers/layer2.h"

void layer1Provider_connect(Layer1Provider *provider1, Layer1Provider *provider2) {
    provider1->other = provider2;
    provider2->other = provider1;
}

void handleLayer1Receive(EventData data) {
    // Send card to incoming queue
    Layer1ReceiveData *d = data;

    Layer2InEventData newData = {
        .provider=d->receiver->layer2Provider,
        .data=d->data
    };

    d->receiver->injectError(d->receiver, &(newData.data));

    // No delay because propagation and transmission delays are already accounted for?
    // Should those actually be here?
    PostEvent(d->receiver->deviceID, d->receiver->layer2Provider->onReceiveBuffer, &newData, sizeof(newData), 0);
}