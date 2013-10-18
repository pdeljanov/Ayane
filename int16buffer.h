#ifndef STARGAZER_STDLIB_AUDIO_INT16BUFFER_H_
#define STARGAZER_STDLIB_AUDIO_INT16BUFFER_H_

#include "buffer.h"

namespace Stargazer {
    namespace Audio {
        
        class Int16Buffer : public Buffer
        {
        public:
            
            Int16Buffer( const BufferFormat &format, const BufferLength &length );
            ~Int16Buffer();
            
            /** Gets the underlying sample format identifier for the buffer. */
            static SampleFormat identifier() { return Int16; }
            
            virtual force_inline SampleFormat sampleFormat() const
            { return Int16Buffer::identifier(); }
            
            // Mono
            virtual Int16Buffer& operator<< (const Mono<SampleInt16>& );
            virtual Int16Buffer& operator<< (const Mono<SampleFloat32>& );
            
            // Stereo20
            virtual Int16Buffer& operator<< ( const Stereo20<SampleInt16>& );
            virtual Int16Buffer& operator<< ( const Stereo20<SampleFloat32>& );
            
            /**
             *  Writes a frame of audio data to the buffer. Any necessary conversions
             *  are handled automatically. This is equivalent to using the left shift
             *  operator.
             *  @param frame The audio frame.
             */
            template<typename InFrameType>
            force_inline Int16Buffer &write( const InFrameType &frame );
            
        private:
            SampleInt16 *m_buffer;
            Mapper<SampleInt16> m_mapper;
            
        };
        
        template< typename InFrameType >
        force_inline Int16Buffer &Int16Buffer::write( const InFrameType &f )
        {
            f.write(m_mapper);
            ++m_mapper;
            return (*this);
        }
        
    }
}

#endif
