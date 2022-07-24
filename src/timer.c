#include "timer.h"

#include "platform.h"
#include "util/math.h"

static u64 timerFreq;

void timer_init() {
    timerFreq = getCycleFreq();
}

Timer timer_start() {
    return (Timer){
        .startCycleCount=getCycleCount()
    };
}

u64 timer_stop(Timer timer) {
    u64 end = getCycleCount();
    f64 secs = (f64)(end - timer.startCycleCount) / (f64)timerFreq;
    return (u64)unitToNano(secs);
}
