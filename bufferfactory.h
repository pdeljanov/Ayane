#ifndef STARGAZER_STDLIB_AUDIO_BUFFERFACTORY_H_
#define STARGAZER_STDLIB_AUDIO_BUFFERFACTORY_H_

#include "buffer.h"

namespace Stargazer {
    namespace Audio {
        
        class BufferFactory
        {
        public:
            
            static Buffer* make(SampleFormat sampleFormat,
                                const BufferFormat &format,
                                const BufferLength &length);
            
        };
        
    }
}

#endif