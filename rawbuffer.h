#ifndef STARGAZER_STDLIB_AUDIO_RAWBUFFER_H_
#define STARGAZER_STDLIB_AUDIO_RAWBUFFER_H_

#include <cstddef>

#include "formats.h"

namespace Stargazer
{
    namespace Audio
    {
        

    
        /** Wraps a memory buffer containing audio in a semi-arbitrary data format.
         *
         * RawAudioBuffer wraps an arbitrary memory buffer containing audio data. It stores
         * information such as the data format and the size of the buffer. The raw data may
         * be accessed using the overloaded shift operators.
         *
         * \note In debug mode, bounds checking is applied to all operations that modify
         *       the underlying buffer.
         **/
        class RawBuffer
        {
            friend class Buffer;
            
        public:
            
            RawBuffer ( uint8_t *buffer, size_t length, SampleFormat format ) :
                m_buffer( buffer ),
                m_format( format ),
                m_stride( SampleFormats::about(format).stride ),
                m_end( buffer + length ),
                m_readPointer( buffer ),
                m_writePointer( buffer )
            {
                // Nothing to do here. :)
            }
            
            /** Gets the length of the buffer.
             *
             * \returns The length of the buffer in samples.
             **/
            size_t length() const
            {
                return (m_end - m_buffer);
            }
            
            /** Gets the number of samples consumed.
             *
             * \returns The number of samples consumed in the buffer.
             **/
            size_t consumed() const
            {
                return static_cast<size_t>(m_readPointer - m_buffer) / m_stride;
            }
            
            /** Gets the number of samples remaining in the buffer
             *
             * \returns The number of samples remaining in the buffer
             **/
            size_t remaining() const
            {
                return length() - consumed();
            }
            
            /** Check whether the audio buffer is nil.
             *
             * \returns True is the buffer is a null pointer, or the length is zero.
             **/
            bool isNil() const
            {
                return ( (m_buffer == nullptr) || (length() == 0) );
            }
            
            /** Gets the format of the audio samples stored in the buffer.
             *
             * \returns Returns the sample format of the underlying buffer.
             **/
            SampleFormat sampleFormat() const
            {
                return m_format;
            }

            
            template< typename T >
            inline RawBuffer& operator << ( const T& value )
            {
                
#if defined(DEBUG)
                if( m_writePointer == m_end )
                    throw std::exception();
#endif
                    
                *(T*)m_writePointer = value;
                m_writePointer += m_stride;
                return *this;
            }
            
            
            template< typename T >
            inline RawBuffer& operator >> ( T& value )
            {
                
#if defined(DEUBG)
                if( m_readPointer == m_end )
                    throw std::exception();
#endif
                    
                value = *(T*)m_readPointer;
                m_readPointer += m_stride;
                return *this;
            }
            
            
            
        private:
            
            const uint8_t *m_buffer;
            const SampleFormat m_format;
            const unsigned int m_stride;
            const uint8_t *m_end;
            
            uint8_t *m_readPointer;
            uint8_t *m_writePointer;
            
            
        };
        
    }
}

#endif
