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
            
            inline Channels channels() const
            { return m_channels; }
            
            inline unsigned int channelCount() const
            { return m_samplesPerFrame; }
            
            inline SampleRate sampleRate() const
            { return m_sampleRate; }
            
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
