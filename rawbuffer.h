#pragma once

#include <stddef.h>

#include "formats.h"

namespace Stargazer
{
    namespace Audio
    {
        
        /** Wraps a memory buffer containing audio in an arbitrary data format.
         *
         * RawAudioBuffer wraps an arbitrary memory buffer containing audio data. It stores
         * information such as the data format and the size of the buffer.
         **/
        class RawBuffer
        {
            friend class Buffer;
            
        public:
            
            RawBuffer();
            
            RawBuffer ( void *buffer, size_t size, SampleFormat format );
            
            /** Gets the length of the buffer.
             *
             * \returns The length of the buffer in samples.
             **/
            size_t length() const;
            
            /** Gets the number of samples consumed.
             *
             * \returns The number of samples consumed in the buffer.
             **/
            size_t consumed() const;
            
            /** Gets the number of samples remaining in the buffer
             *
             * \returns The number of samples remaining in the buffer
             **/
            size_t remaining() const;
            
            /** Check whether the audio buffer is nil.
             *
             * \returns True is the buffer is a null pointer, or the length is zero.
             **/
            bool isNil() const;
            
            /** Gets the format of the audio samples stored in the buffer.
             *
             * \returns Returns the sample format of the underlying buffer.
             **/
            SampleFormat sampleFormat() const;
            
            /** Gets a pointer to the buffer and records any data consumption.
             *
             * \param consuming The number of samples that will be consumed.
             *
             * \returns A read-only pointer to the buffer.
             **/
            const void *buffer ( size_t consuming );
            
        private:
            
            void *m_buffer;
            size_t m_length;
            size_t m_consumed;
            size_t m_sampleSize;
            SampleFormat m_format;
            
        };
        
    }
}
