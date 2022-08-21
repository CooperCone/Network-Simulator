#include "log.h"

#include "event.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

static struct {
    FILE **files;
    u64 numFiles;
} fileArray;

FILE *eventFile;

void log(u64 deviceID, const char *msg, ...) {
    assert(deviceID > 0);
    // Our ids start with 1
    deviceID--;

    while (fileArray.numFiles <= deviceID) {
        fileArray.numFiles++;
        fileArray.files = realloc(fileArray.files, sizeof(FILE*) * fileArray.numFiles);

        char *format = "log/device_%d.log";
        u64 currentID = fileArray.numFiles - 1;
        int size = snprintf(NULL, 0, format, currentID + 1);
        char *fileName = malloc(size + 1);
        sprintf(fileName, format, currentID + 1);

        fileArray.files[currentID] = fopen(fileName, "w");
    }

    va_list args;
    va_start(args, msg);

    FILE *file = fileArray.files[deviceID];
    fprintf(file, "%ld ", CurTime());
    vfprintf(file, msg, args);
    fprintf(file, "\n");
    fflush(file);

    va_end(args);
}

void logEvent(u64 deviceID, char *eventHandlerName) {
    if (eventFile == NULL) {
        char *name = "log/events.log";
        eventFile = fopen(name, "w");
    }

    fprintf(eventFile, "%ld: %s\n", deviceID, eventHandlerName);
    fflush(eventFile);
}
