#pragma once

#include "util/types.h"

typedef struct {
    u8 *data;
    u64 dataSize;
} Buffer;

u16 buffer_checksum16(Buffer buff);
u32 buffer_checksum32(Buffer buff);
