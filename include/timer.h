#pragma once

#include "util/types.h"

typedef struct {
    u64 startCycleCount;
} Timer;

void timer_init();

Timer timer_start();
u64 timer_stop(Timer timer);
