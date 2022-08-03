#pragma once

#include "util/types.h"

void log(u64 deviceID, const char *msg, ...);
void logEvent(u64 deviceID, char *eventHandlerName);
