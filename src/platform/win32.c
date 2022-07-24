#include "platform.h"

#include "Windows.h"

#include <assert.h>
#include <stdbool.h>

u64 getCycleFreq() {
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)) {
        assert(false);
    }

    return (u64)freq.QuadPart;
}

u64 getCycleCount() {
    LARGE_INTEGER count;
    if (!QueryPerformanceCounter(&count)) {
        assert(false);
    }

    return (u64)count.QuadPart;
}
