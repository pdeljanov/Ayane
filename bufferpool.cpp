/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "bufferpool.h"

#include <iostream>

using namespace Stargazer::Audio;

ManagedBufferDeallocator::ManagedBufferDeallocator(){
    
}

ManagedBufferDeallocator::ManagedBufferDeallocator(std::weak_ptr<ManagedBufferOwner> &owner) :
    mOwner(owner)
{
}

void ManagedBufferDeallocator::operator()(Buffer *buffer) {
    
    if( mOwner.expired() ) {
        delete buffer;
        return;
    }
    
    if( std::shared_ptr<ManagedBufferOwner> owner = mOwner.lock() ) {
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
}

ManagedBuffer BufferPoolPrivate::acquire() {

    std::lock_guard<std::mutex> lock(mPoolMutex);

    ManagedBuffer buffer;
    
    if( mPool.empty() )
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
    std::lock_guard<std::mutex> lock(mPoolMutex);
    
    buffer->reset();
    
    mPool.push(ManagedBuffer(buffer, ManagedBufferDeallocator(mWeakToSelf)));
}

void BufferPoolPrivate::preallocate( int count )
{
    
    // Lock the pool.
    std::lock_guard<std::mutex> lock(mPoolMutex);
    
    mWeakToSelf = this->shared_from_this();
    
    for( int i = 0; i < count; ++i ) {
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


