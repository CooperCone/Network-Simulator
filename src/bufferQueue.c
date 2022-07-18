#include "bufferQueue.h"

#include <stdlib.h>
#include <assert.h>

BufferQueue bufferQueue_create(u64 maxBuffers) {
    BufferQueue queue = {0};
    queue.buffers = calloc(maxBuffers, sizeof(Buffer));
    queue.maxBuffers = maxBuffers;
    return queue;
}

void bufferQueue_push(BufferQueue *queue, Buffer buff) {
    assert(queue->numBuffers != queue->maxBuffers);

    queue->buffers[BufferQueueLast(queue)] = buff;
    queue->numBuffers++;
}

Buffer bufferQueue_pop(BufferQueue *queue) {
    assert(queue->numBuffers > 0);

    Buffer buff = queue->buffers[queue->startBuffer];
    queue->startBuffer = BufferQueueNext(queue, queue->startBuffer);
    queue->numBuffers--;
    return buff;
}
