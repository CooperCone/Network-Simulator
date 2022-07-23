#pragma once

#include "buffer.h"

// Buffer that can't be resized
typedef struct {
    Buffer *buffers;
    u64 startBuffer;
    u64 numBuffers;
    u64 maxBuffers;
} BufferQueue;

#define BufferQueueLast(queue) (queue->startBuffer + queue->numBuffers) % queue->maxBuffers
#define BufferQueueNext(queue, idx) (idx + 1) % queue->maxBuffers

BufferQueue bufferQueue_create(u64 maxBuffers);
void bufferQueue_push(BufferQueue *queue, Buffer buff);
Buffer bufferQueue_pop(BufferQueue *queue);
