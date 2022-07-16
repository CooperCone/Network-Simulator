#include "physicalLayer.h"

void wireTerminal_connect(WireTerminal *term1, WireTerminal *term2) {
    term1->other = term2;
    term2->other = term1;
}

void phy_send(WireTerminal *terminal, char *buffer, int length);
