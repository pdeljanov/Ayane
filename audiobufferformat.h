#pragma once

#include "channels.h"
#include "formats.h"

namespace Ayane
{

  class AudioBufferFormat
  {
    friend class AudioBuffer;

    public:
      AudioBufferFormat( );
      AudioBufferFormat( Channels channels, SampleRate sampleRate );
      AudioBufferFormat( const AudioBufferFormat &format );
      
      Channels channels() const;
      unsigned int channelCount() const;

      SampleRate sampleRate() const;
      
      bool isValid() const;
      
      AudioBufferFormat& operator= ( const AudioBufferFormat &right );
      
      bool operator== ( const AudioBufferFormat& right ) const;
      bool operator!= ( const AudioBufferFormat& right ) const;
      bool operator<  ( const AudioBufferFormat& right ) const;
      bool operator<= ( const AudioBufferFormat& right ) const;
      bool operator>  ( const AudioBufferFormat& right ) const;
      bool operator>= ( const AudioBufferFormat& right ) const;
      
  private:
    
    Channels m_channels;
    SampleRate m_sampleRate;
    unsigned int m_samplesPerFrame;
    
  };
  

}
