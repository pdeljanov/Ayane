/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef STARGAZER_STDLIB_AUDIO_BUFFERPOOL_H_
#define STARGAZER_STDLIB_AUDIO_BUFFERPOOL_H_

#include "bufferfactory.h"

#include <memory>
#include <mutex>
#include <stack>

namespace Stargazer {
    namespace Audio {

        class ManagedBufferOwner {
        public:
            virtual void reclaim(Buffer *buffer) = 0;
        };

        class ManagedBufferDeallocator {
        public:
            ManagedBufferDeallocator();
            ManagedBufferDeallocator(std::weak_ptr<ManagedBufferOwner> &owner);
            
            void operator()(Buffer *buffer);
        private:
            std::weak_ptr<ManagedBufferOwner> mOwner;
        };


        typedef std::unique_ptr<Buffer, ManagedBufferDeallocator> ManagedBuffer;
        
        class BufferPoolPrivate : public ManagedBufferOwner,
                                  public std::enable_shared_from_this<BufferPoolPrivate>
        {
            friend class ManagedBufferDeallocator;
            friend class BufferPoolFactory;
            
        public:
            ~BufferPoolPrivate();

            void setBufferTemplate(SampleFormat format,
                                   const BufferFormat &bufferFormat,
                                   const BufferLength &bufferLength);
            
            ManagedBuffer acquire();
            
        protected:
            virtual void reclaim(Buffer *buffer);
            
        private:
            STARGAZER_DISALLOW_COPY_AND_ASSIGN(BufferPoolPrivate);
            
            BufferPoolPrivate(SampleFormat format,
                              const BufferFormat &bufferFormat,
                              const BufferLength &bufferLength);
            
            
            void preallocate(int count);
            
            std::mutex mPoolMutex;
            std::stack<ManagedBuffer> mPool;
            
            SampleFormat mSampleFormat;
            BufferFormat mBufferFormat;
            BufferLength mBufferLength;
            
            std::weak_ptr<ManagedBufferOwner> mWeakToSelf;
        };
        
        
        typedef std::shared_ptr<BufferPoolPrivate> BufferPool;
        
        
        class BufferPoolFactory {
        public:

            static BufferPool create(SampleFormat format,
                                     const BufferFormat &bufferFormat,
                                     const BufferLength &bufferLength);
            
            static BufferPool create(SampleFormat format,
                                     const BufferFormat &bufferFormat,
                                     const BufferLength &bufferLength,
                                     int count);
            
        private:
            STARGAZER_DISALLOW_DEFAULT_CTOR_COPY_AND_ASSIGN(BufferPoolFactory);
        };
        
        


        
    }
}

#endif
