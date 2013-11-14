#ifndef STARGAZER_STDLIB_AUDIO_BUFFERQUEUE_H_
#define STARGAZER_STDLIB_AUDIO_BUFFERQUEUE_H_

#include <stddef.h>

#include "buffer.h"

#include <vector>
#include <memory>
#include <atomic>

namespace Stargazer {
    namespace Audio {

        
        class BufferQueue
        {
        public:

            BufferQueue( uint32_t count );
            ~BufferQueue();

            bool push ( std::unique_ptr<Buffer> &inBuffer );
            bool pop ( std::unique_ptr<Buffer> *outBuffer );
            
            uint32_t capacity() const;
            
            bool full() const;
            bool empty() const;
            
            void clear();


        private:
            STARGAZER_DISALLOW_COPY_AND_ASSIGN(BufferQueue);

            const uint32_t mCount;
            std::atomic_uint mWriteIndex;
            std::atomic_uint mReadIndex;

            // Dynamic elements array, initialized to the exact size in the
            // initialization list.
            std::vector<std::unique_ptr<Buffer>> mElements;
        };
        
        
        
    }

}

#endif
