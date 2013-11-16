/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "bufferqueue.h"

using namespace Stargazer::Audio;

BufferQueue::BufferQueue( uint32_t count ) :
mCount(++count), mWriteIndex(0), mReadIndex(0), mElements(mCount)
{
    
}

BufferQueue::~BufferQueue() {
}

uint32_t BufferQueue::capacity() const {
    return mCount - 1;
}

bool BufferQueue::full() const {

    int writeIndex = mWriteIndex.load(std::memory_order_relaxed);
    int readIndex = mReadIndex.load(std::memory_order_relaxed);
    int newWriteIndex = (writeIndex + 1) % mCount;

    return (newWriteIndex == readIndex);
}

bool BufferQueue::empty() const {
    return (mWriteIndex == mReadIndex);
}

void BufferQueue::clear() {
    // TODO: Make safe.
    mElements.clear();
    mReadIndex = 0;
    mWriteIndex = 0;
}

bool BufferQueue::push( ManagedBuffer &inBuffer) {
    
    int writeIndex = mWriteIndex.load(std::memory_order_relaxed);
    int readIndex = mReadIndex.load(std::memory_order_relaxed);
    
    int newWriteIndex = (writeIndex + 1) % mCount;
    
    // If the new write index is equal to the read index, the queue is
    // full.
    if ( newWriteIndex == readIndex ) {
        return false;
    }
    
    // Pass ownership of the buffer to the queue.
    mElements[writeIndex] = std::move(inBuffer);
    
    // Store the new write index.
    mWriteIndex.store(newWriteIndex, std::memory_order_relaxed);
    
    return true;
}

bool BufferQueue::pop( ManagedBuffer *outBuffer) {
    
    int writeIndex = mWriteIndex.load(std::memory_order_relaxed);
    int readIndex = mReadIndex.load(std::memory_order_relaxed);
    
    // If the read index is equal to the write index, the queue is
    // empty.
    if( readIndex == writeIndex ) {
        return false;
    }
    
    // Pass ownership of the buffer from the queue to the caller.
    *outBuffer = std::move(mElements[readIndex]);
    
    // Store the new read index.
    int newReadIndex = (readIndex + 1) % mCount;
    mReadIndex.store(newReadIndex, std::memory_order_relaxed);
    
    return true;
}
