/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_BUFFERQUEUE_H_
#define AYANE_BUFFERQUEUE_H_

#include <stddef.h>
#include <vector>
#include <memory>
#include <atomic>

#include "Ayane/BufferPool.h"

namespace Ayane {
    
    class BufferQueue
    {
    public:
        
        BufferQueue( uint32_t count );
        ~BufferQueue();
        
        bool push ( ManagedBuffer &inBuffer );
        bool pop ( ManagedBuffer *outBuffer );
        
        uint32_t capacity() const;
        
        bool full() const;
        bool empty() const;
        
        void clear();
        
        
    private:
        AYANE_DISALLOW_COPY_AND_ASSIGN(BufferQueue);
        
        const uint32_t mCount;
        std::atomic_uint mWriteIndex;
        std::atomic_uint mReadIndex;
        
        // Dynamic elements array, initialized to the exact size in the
        // initialization list.
        std::vector<ManagedBuffer> mElements;
    };
    
}

#endif
