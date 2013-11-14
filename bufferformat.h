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
            BufferFormat();
            BufferFormat(Channels channels, SampleRate sampleRate);
            BufferFormat(const BufferFormat &format);
            
            inline Channels channels() const {
                return mChannels;
            }
            
            inline unsigned int channelCount() const {
                return mSamplesPerFrame;
            }
            
            inline SampleRate sampleRate() const {
                return mSampleRate;
            }
            
            bool isValid() const;
            
            BufferFormat& operator= ( const BufferFormat &right );
            
            bool operator== ( const BufferFormat& right ) const;
            bool operator!= ( const BufferFormat& right ) const;
            bool operator<  ( const BufferFormat& right ) const;
            bool operator<= ( const BufferFormat& right ) const;
            bool operator>  ( const BufferFormat& right ) const;
            bool operator>= ( const BufferFormat& right ) const;
            
        private:
            
            Channels mChannels;
            SampleRate mSampleRate;
            unsigned int mSamplesPerFrame;
            
        };
        
        
    }
}

#endif
