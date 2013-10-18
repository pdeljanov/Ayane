#ifndef STARGAZER_STDLIB_AUDIO_BUFFERFORMAT_H_
#define STARGAZER_STDLIB_AUDIO_BUFFERFORMAT_H_

#include "channels.h"
#include "formats.h"

namespace Stargazer
{
    namespace Audio
    {
        
        class BufferFormat
        {
            friend class Buffer;
            
        public:
            BufferFormat( );
            BufferFormat( Channels channels, SampleRate sampleRate );
            BufferFormat( const BufferFormat &format );
            
            Channels channels() const;
            unsigned int channelCount() const;
            
            SampleRate sampleRate() const;
            
            bool isValid() const;
            
            BufferFormat& operator= ( const BufferFormat &right );
            
            bool operator== ( const BufferFormat& right ) const;
            bool operator!= ( const BufferFormat& right ) const;
            bool operator<  ( const BufferFormat& right ) const;
            bool operator<= ( const BufferFormat& right ) const;
            bool operator>  ( const BufferFormat& right ) const;
            bool operator>= ( const BufferFormat& right ) const;
            
        private:
            
            Channels m_channels;
            SampleRate m_sampleRate;
            unsigned int m_samplesPerFrame;
            
        };
        
        
    }
}

#endif
