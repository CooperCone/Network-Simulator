#include "util/types.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void ipAddr_toStr(IPAddress addr, IPStr outStr) {
    sprintf(outStr, "%d.%d.%d.%d", addr.b1, addr.b2, addr.b3, addr.b4);
}

IPAddress ipAddr_fromStr(IPStr str) {
    IPAddress outAddr = {0};
    
    char *s = str;
    u8 idx = 0;

    while (idx < 4) {
        int byte = atoi(s);
        outAddr.bs[idx] = (u8)byte;
        idx++;

        if (idx == 4)
            break;

        while (*s != '.') {
            s++;
        }
        s++;
    }

    return outAddr;
}

void macAddr_copy(MACAddress dst, MACAddress src) {
    memcpy(dst, src, 6);
}

void macAddr_toStr(MACAddress addr, MACStr outStr) {
    sprintf(outStr, "%x-%x-%x-%x-%x-%x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}
