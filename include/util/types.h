#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef union {
    u32 w;
    struct {
        u8 b1;
        u8 b2;
        u8 b3;
        u8 b4;
    };
    u8 bs[4];
} IPAddress;
typedef char IPStr[16];

void ipAddr_toStr(IPAddress addr, IPStr outStr);
IPAddress ipAddr_fromStr(IPStr str);

typedef u8 MACAddress[6];
typedef char MACStr[18];

#define MACBroadcast ((MACAddress){ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF })

bool macAddr_cmp(MACAddress addr1, MACAddress addr2);
void macAddr_copy(MACAddress dst, MACAddress src);
void macAddr_toStr(MACAddress addr, MACStr outStr);
