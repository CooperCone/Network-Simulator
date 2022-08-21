#include "platform.h"

#include <time.h>
#include <assert.h>
#include <stdbool.h>

// Linux has a nanosecond timer, so we can just divide by
// 1, because each "cycle" is a nanosecond
u64 getCycleFreq() {
    return 1;
}

u64 getCycleCount() {
    struct timespec res;
    clock_gettime(CLOCK_MONOTONIC_RAW, &res);

    return res.tv_nsec;
}