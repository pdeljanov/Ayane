#ifndef STARGAZER_STDLIB_AUDIO_FLOAT32BUFFER_H_
#define STARGAZER_STDLIB_AUDIO_FLOAT32BUFFER_H_

#include "buffer.h"

namespace Stargazer {
    namespace Audio {
     
        class Float32Buffer : public Buffer
        {
        public:
            
            Float32Buffer( const BufferFormat &format, const BufferLength &length );
            ~Float32Buffer();
            
            /** Gets the underlying sample format identifier for the buffer. */
            static SampleFormat identifier() { return Float32; }
            
            virtual force_inline SampleFormat sampleFormat() const
            { return Float32Buffer::identifier(); }
            
            // Mono
            virtual Float32Buffer& operator<< (const Mono<SampleInt16>& );
            virtual Float32Buffer& operator<< (const Mono<SampleFloat32>& );
            
            // Stereo20
            virtual Float32Buffer& operator<< ( const Stereo20<SampleInt16>& );
            virtual Float32Buffer& operator<< ( const Stereo20<SampleFloat32>& );
            
            /**
             *  Writes a frame of audio data to the buffer. Any necessary conversions
             *  are handled automatically. This is equivalent to using the left shift
             *  operator.
             *  @param frame The audio frame.
             */
            template<typename InFrameType>
            force_inline Float32Buffer &write( const InFrameType &frame );
            
        private:
            SampleFloat32 *m_buffer;
            Mapper<SampleFloat32> m_mapper;

        };
        
        template< typename InFrameType >
        force_inline Float32Buffer &Float32Buffer::write( const InFrameType &f )
        {
            f.write(m_mapper);
            ++m_mapper;
            return (*this);
        }

        
    }
}

#endif
