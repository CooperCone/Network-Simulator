#include "layers/layer3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ipAddr_copy(IPAddress dst, IPAddress src) {
    memcpy(dst, src, sizeof(IPAddress));
}

void ipAddr_toStr(IPAddress addr, IPStr outStr) {
    sprintf(outStr, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
}

void ipAddr_fromStr(IPStr str, IPAddress addr) {
    char *s = str;
    u8 idx = 0;

    while (idx < 4) {
        int byte = atoi(s);
        addr[idx] = (u8)byte;
        idx++;

        if (idx == 4)
            break;

        while (*s != '.') {
            s++;
        }
        s++;
    }
}
