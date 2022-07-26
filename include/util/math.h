#pragma once

#include "util/types.h"

#define speedOfLight 300000000

#define unitToMili(unit) (1000 * (unit))
#define unitToMicro(unit) (1000 * (unitToMili(unit)))
#define unitToNano(unit) (1000 * (unitToMicro(unit)))

#define kiloToUnit(unit) ((unit) * 1000)
#define megaToUnit(unit) (1000 * (kiloToUnit(unit)))
#define gigaToUnit(unit) (1000 * (megaToUnit(unit)))

inline u16 internetChecksumComplement(u16 *bytes, u64 length) {
    u32 checksum = 0;
    for (u64 i = 0; i < length; i++) {
        checksum += bytes[i];

        // Wrap around carry out bits
        checksum += (checksum >> 16);

        // Clear carry out bits
        checksum &= 0x00FF;
    }
    return (u16)checksum;
}

inline u16 internetChecksum(u16 *bytes, u64 length) {
    return ~internetChecksumComplement(bytes, length);
}
