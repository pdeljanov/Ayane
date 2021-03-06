/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "Ayane/BufferPool.h"

using namespace Ayane;

ManagedBufferDeallocator::ManagedBufferDeallocator(){
    
}

ManagedBufferDeallocator::ManagedBufferDeallocator(std::weak_ptr<ManagedBufferOwner> &owner) :
    mOwner(owner)
{
}

void ManagedBufferDeallocator::operator()(Buffer *buffer) {
    
    if(std::shared_ptr<ManagedBufferOwner> owner = mOwner.lock()) {
        // Reset the buffer, then try to get the owner to reclaim it.
        buffer->reset();
        owner->reclaim(buffer);
    }
    else {
        delete buffer;
    }
}

BufferPoolPrivate::BufferPoolPrivate(SampleFormat format,
                                     const BufferFormat &bufferFormat,
                                     const BufferLength &bufferLength) :
mSampleFormat(format),
mBufferFormat(bufferFormat),
mBufferLength(bufferLength)
{
}

BufferPoolPrivate::~BufferPoolPrivate(){
}

void BufferPoolPrivate::setBufferTemplate(SampleFormat format,
                                          const BufferFormat &bufferFormat,
                                          const BufferLength &bufferLength)
{
    std::lock_guard<std::mutex> lock(mPoolMutex);
    
    mSampleFormat = format;
    mBufferFormat = bufferFormat;
    mBufferLength = bufferLength;
    
    // Clear the pool of old buffer types.
    std::stack<ManagedBuffer>().swap(mPool);
}

void BufferPoolPrivate::clear() {
    std::lock_guard<std::mutex> lock(mPoolMutex);
    std::stack<ManagedBuffer>().swap(mPool);
}

ManagedBuffer BufferPoolPrivate::acquire() {

    std::lock_guard<std::mutex> lock(mPoolMutex);

    ManagedBuffer buffer;
    
    if(mPool.empty())
    {
        Buffer *newBuffer = BufferFactory::make(mSampleFormat,
                                                mBufferFormat,
                                                mBufferLength);
        
        buffer = ManagedBuffer(newBuffer, ManagedBufferDeallocator(mWeakToSelf));
    }
    else
    {
        buffer = std::move(mPool.top());
        mPool.pop();
    }
    
    return buffer;
}

void BufferPoolPrivate::reclaim(Buffer *buffer)
{
    // TODO: Make lockless.
    std::lock_guard<std::mutex> lock(mPoolMutex);
    mPool.push(ManagedBuffer(buffer, ManagedBufferDeallocator(mWeakToSelf)));
}

void BufferPoolPrivate::preallocate(int count)
{
    // Lock the pool.
    std::lock_guard<std::mutex> lock(mPoolMutex);
    
    mWeakToSelf = this->shared_from_this();
    
    for(int i = 0; i < count; ++i) {
        Buffer *newBuffer = BufferFactory::make(mSampleFormat,
                                                mBufferFormat,
                                                mBufferLength);
        
        mPool.push(ManagedBuffer(newBuffer, ManagedBufferDeallocator(mWeakToSelf)));
    }
}

BufferPool BufferPoolFactory::create(SampleFormat format,
                                     const BufferFormat &bufferFormat,
                                     const BufferLength &bufferLength)
{
    return create(format, bufferFormat, bufferLength, 0);
}

BufferPool BufferPoolFactory::create(SampleFormat format,
                                     const BufferFormat &bufferFormat,
                                     const BufferLength &bufferLength,
                                     int count)
{
    BufferPool pool(new BufferPoolPrivate(format, bufferFormat, bufferLength));
    pool->preallocate(count);
    return pool;
}


