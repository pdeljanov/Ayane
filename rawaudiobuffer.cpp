#include "rawaudiobuffer.h"
#include "sampleformatconverters.h"
#include "logger.h"
namespace Ayane
{

  RawAudioBuffer::RawAudioBuffer() :
    m_buffer ( NULL ),
    m_length ( 0 ),
    m_consumed ( 0 ),
    m_format ( ( SampleFormat ) 0 )
  {

  }

  RawAudioBuffer::RawAudioBuffer ( void* buffer, size_t length, SampleFormat format ) :
    m_buffer ( buffer ),
    m_length ( length ),
    m_consumed ( 0 ),
    m_format ( format )
  {
  }

  size_t RawAudioBuffer::length() const
  {
    return m_length;
  }

  size_t RawAudioBuffer::consumed() const
  {
    return m_consumed;
  }

  size_t RawAudioBuffer::remaining() const
  {
    return m_length - m_consumed;
  }

  bool RawAudioBuffer::isNil() const
  {
    if ( ( m_buffer == NULL ) || ( m_length == 0 ) )
      return true;

    return false;
  }

  SampleFormat RawAudioBuffer::sampleFormat() const
  {
    return m_format;
  }

  const void* RawAudioBuffer::buffer ( size_t consuming )
  {
    uint8_t *out = static_cast<uint8_t*> ( m_buffer ) + ( m_consumed * SampleFormatDescriptors[m_format].size );
    m_consumed += consuming;
    return out;
  }

}
