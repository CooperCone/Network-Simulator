#pragma once

struct WireTerminal;

typedef void (*PhySendFunc)(struct WireTerminal*);

typedef struct WireTerminal {
    struct WireTerminal *other;
    PhySendFunc sendFunc;
    
} WireTerminal;

void wireTerminal_connect(WireTerminal *term1, WireTerminal *term2);

// Different funcs so we can do things like
// - simulate different bit errors
// - instrument and log out info

void phy_send(WireTerminal *terminal, char *buffer, int length);
