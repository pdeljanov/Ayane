#include "audiobufferformat.h"

namespace Ayane
{

  AudioBufferFormat::AudioBufferFormat() : m_channels ( 0 ), m_sampleRate ( 0 ), m_samplesPerFrame ( 0 )
  {

  }

  AudioBufferFormat::AudioBufferFormat ( Channels channels, SampleRate sampleRate ) :
    m_channels ( channels ), m_sampleRate ( sampleRate ), m_samplesPerFrame ( 0 )
  {

    // # of set bits in 0x00 to 0x0f
    const static int lookup[] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

    for ( ; channels != 0; channels >>= 4 )
      m_samplesPerFrame += lookup[ channels & 0x0f ];

  }

  AudioBufferFormat::AudioBufferFormat ( const Ayane::AudioBufferFormat& format ) :
    m_channels ( format.m_channels ), m_sampleRate ( format.m_sampleRate ), m_samplesPerFrame ( format.m_samplesPerFrame )
  {

  }

  Channels AudioBufferFormat::channels() const
  {
    return m_channels;
  }

  unsigned int AudioBufferFormat::channelCount() const
  {
    return m_samplesPerFrame;
  }

  SampleRate AudioBufferFormat::sampleRate() const
  {
    return m_sampleRate;
  }

  bool AudioBufferFormat::isValid() const
  {
    return ( ( m_channels > 0 ) && ( m_sampleRate > 0 ) );
  }

  AudioBufferFormat& AudioBufferFormat::operator= ( const Ayane::AudioBufferFormat& right )
  {
    m_channels = right.m_channels;
    m_sampleRate = right.m_sampleRate;
    m_samplesPerFrame = right.m_samplesPerFrame;
    return *this;
  }

  bool AudioBufferFormat::operator== ( const AudioBufferFormat& right ) const
  {
    return ( m_channels == right.m_channels ) && ( m_sampleRate == right.m_sampleRate );
  }

  bool AudioBufferFormat::operator!= ( const AudioBufferFormat& right ) const
  {
    return ( ( m_channels != right.m_channels ) || ( m_sampleRate != right.m_sampleRate ) );
  }

  bool AudioBufferFormat::operator< ( const AudioBufferFormat& right ) const
  {
    if( m_sampleRate < right.m_sampleRate )
    {
      return true;
    }
    else if( m_sampleRate == right.m_sampleRate )
    {
      return ( m_samplesPerFrame < right.m_samplesPerFrame );
    }
    else
    {
      return false;
    }
  }

  bool AudioBufferFormat::operator<= ( const AudioBufferFormat& right ) const
  {
    if( m_sampleRate < right.m_sampleRate )
    {
      return true;
    }
    else if( m_sampleRate == right.m_sampleRate )
    {
      return ( m_samplesPerFrame <= right.m_samplesPerFrame );
    }
    else
    {
      return false;
    }
  }

  bool AudioBufferFormat::operator> ( const AudioBufferFormat& right ) const
  {
    return !( *this <= right );
  }

  bool AudioBufferFormat::operator>= ( const AudioBufferFormat& right ) const
  {
    return !( *this < right );
  }

}
