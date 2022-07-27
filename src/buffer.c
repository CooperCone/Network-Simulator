#include "buffer.h"

#include <stdio.h>

u16 buffer_checksum16(Buffer buff) {
    u16 val = 0;

    for (u64 i = 0; i < buff.dataSize; i++) {
        val += buff.data[i];
    }

    return val;
}

u32 buffer_checksum32(Buffer buff) {
    u32 val = 0;

    for (u64 i = 0; i < buff.dataSize; i++) {
        val += buff.data[i];
    }

    return val;

}
