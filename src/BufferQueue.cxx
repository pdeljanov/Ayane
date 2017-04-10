/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "Ayane/BufferQueue.h"

using namespace Ayane;

BufferQueue::BufferQueue( uint32_t count ) :
    mCount(count),
    mWriteIndex(0),
    mReadIndex(0),
    mElements(count)
{
}

BufferQueue::~BufferQueue() {
}

uint32_t BufferQueue::capacity() const {
    return mCount;
}

bool BufferQueue::full() const {

    unsigned int writeIndex = mWriteIndex.load();
    unsigned int readIndex = mReadIndex.load();

    return (writeIndex - readIndex + 1 >= mCount);
}

bool BufferQueue::empty() const {
    return (mWriteIndex == mReadIndex);
}

void BufferQueue::clear() {

    // Swap a new vector in place. This will force a reallocation
    // (unlike clear()!!) which will cause the ManagedBuffer to destroy the
    // actual buffer. vector::clear() DOES NOT WORK!
    std::vector<ManagedBuffer>(mCount).swap(mElements);
    mReadIndex = 0;
    mWriteIndex = 0;
}

bool BufferQueue::push( ManagedBuffer &inBuffer) {

    unsigned int writeIndex = mWriteIndex.load();
    unsigned int readIndex = mReadIndex.load();

    // If the new write index is equal to the read index, the queue is
    // full.
    if (writeIndex - readIndex >= mCount) {
        return false;
    }

    // Pass ownership of the buffer to the queue.
    mElements[writeIndex % mCount] = std::move(inBuffer);
    
    // Store the new write index.
    mWriteIndex.store(++writeIndex);


    return true;
}

bool BufferQueue::pop( ManagedBuffer *outBuffer) {

    unsigned int writeIndex = mWriteIndex.load();
    unsigned int readIndex = mReadIndex.load();
    
    // If the read index is equal to the write index, the queue is
    // empty.
    if( readIndex == writeIndex ) {
        return false;
    }

    // Pass ownership of the buffer from the queue to the caller.
    *outBuffer = std::move(mElements[readIndex % mCount]);
    
    // Store the new read index.
    mReadIndex.store(++readIndex);
    
    return true;
}
