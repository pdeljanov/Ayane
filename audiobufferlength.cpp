#include "audiobufferlength.h"

namespace Ayane
{
  AudioBufferLength::AudioBufferLength() : 
    m_timeBase( Frames ), m_duration( 0 ), m_frames( 0 )
  {

  }

  AudioBufferLength::AudioBufferLength ( double duration ) :
    m_timeBase ( Duration ), m_duration ( duration ), m_frames ( 0 )
  {
  }

  AudioBufferLength::AudioBufferLength ( unsigned int frames ) :
    m_timeBase ( Frames ), m_duration ( 0 ), m_frames ( frames )
  {
  }

  AudioBufferLength::AudioBufferLength ( const AudioBufferLength& other ) :
    m_timeBase ( other.m_timeBase ), m_duration ( other.m_duration ), m_frames ( other.m_frames )
  {
  }

  double AudioBufferLength::duration ( SampleRate rate ) const
  {
    return ( m_timeBase == Duration ) ? m_duration : m_frames * rate;
  }

  unsigned int AudioBufferLength::frames ( SampleRate rate ) const
  {
    return ( m_timeBase == Frames ) ? m_frames : ( m_duration / 1000.0f ) * rate;
  }

  bool AudioBufferLength::isNil() const
  {
    if ( ( m_frames == 0 ) && ( m_duration == 0.0f ) ) return true;
    return false;
  }

  AudioBufferLength::TimeBase AudioBufferLength::timeBase() const
  {
    return m_timeBase;
  }

  AudioBufferLength& AudioBufferLength::operator= ( const AudioBufferLength& other )
  {
    m_timeBase = other.m_timeBase;
    m_duration = other.m_duration;
    m_frames = other.m_frames;
    return *this;
  }

  bool AudioBufferLength::operator== ( const AudioBufferLength& other ) const
  {
    return ( ( m_timeBase == other.m_timeBase ) && ( m_frames == other.m_frames ) && ( m_duration == other.m_duration ) );
  }

}
