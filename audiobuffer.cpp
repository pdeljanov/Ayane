/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-07.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */

#include <cstring>

#include "audiobuffer.h"
#include "memoryoperations.h"
#include "audiobufferformat.h"
#include "audiobufferlength.h"
#include "rawaudiobuffer.h"
#include "sampleformatconverters.h"

namespace Ayane
{

  class AudioBufferPrivate
  {
    public:

      SampleFloat32 *buffer;

      unsigned int samples;

      AudioBufferFormat format;
      AudioBufferLength length;

      Duration timestamp;

    public:

      AudioBufferPrivate() :
        buffer ( NULL ),
        samples ( 0 ),
        timestamp ( 0.0f )
      {
      }

  };

  AudioBuffer::AudioBuffer ( const AudioBufferFormat &format, const AudioBufferLength &length ) : d ( new AudioBufferPrivate )
  {
    d->format = format;
    d->length = length;

    // Calculate the total number of samples this buffer must store.
    d->samples = length.frames ( d->format.sampleRate() ) * d->format.channelCount();

    size_t bytes = sizeof ( SampleFloat32 ) * d->samples;

    d->buffer = static_cast<SampleFloat32*> ( MemoryOperations::getMemoryOperations().malloc16ByteAligned ( bytes ) );
  }

  AudioBuffer::AudioBuffer ( const AudioBuffer &source ) : d ( new AudioBufferPrivate )
  {
    d->format = source.d->format;
    d->length = source.d->length;
    d->samples = source.d->samples;

    size_t bytes = sizeof ( SampleFloat32 ) * d->samples;

    d->buffer = static_cast<SampleFloat32*> ( MemoryOperations::getMemoryOperations().malloc16ByteAligned ( bytes ) );

    memcpy ( d->buffer, source.d->buffer, bytes );
  }

  AudioBuffer::~AudioBuffer( )
  {
    MemoryOperations::getMemoryOperations().freeAligned ( d->buffer );
    delete d;
  }

  Duration AudioBuffer::duration() const
  {
    return Duration ( d->length.duration ( d->format.m_sampleRate ) );
  }

  const Duration& AudioBuffer::timestamp() const
  {
    return d->timestamp;
  }

  void AudioBuffer::setTimestamp ( const Duration& timestamp )
  {
    d->timestamp = timestamp;
  }

  const AudioBufferFormat& AudioBuffer::format() const
  {
    return d->format;
  }

  unsigned int AudioBuffer::length() const
  {
    return d->samples;
  }

  unsigned int AudioBuffer::frames() const
  {
    return d->length.frames ( d->format.m_sampleRate );
  }

  unsigned int AudioBuffer::fill ( RawAudioBuffer &buffer, unsigned int offset )
  {
    // Calculate the amount of samples that can be copied into the buffer
    unsigned int length = buffer.m_length - buffer.m_consumed;

    if ( length > ( d->samples - offset ) ) length = d->samples - offset;

    SampleFormatConverters::Converter[buffer.m_format]( &d->buffer[offset], ( uint8_t* ) buffer.buffer ( length ), length );

    return length;
  }
  
  unsigned int AudioBuffer::fill ( SampleFloat32* buffer, unsigned int offset, unsigned int n )
  {
    // Calculate the amount of samples that can be copied into the buffer
    if ( n > ( d->samples - offset ) ) n = d->samples - offset;

    for( unsigned int i = offset; i < n; ++i )
      d->buffer[i] = buffer[i];
    
    return n;    
  }

  bool AudioBuffer::copy ( const AudioBuffer& source )
  {
    if ( d->format == source.d->format )
    {
      d->timestamp = source.d->timestamp;

      size_t length = ( d->samples > source.d->samples ) ? source.d->samples : d->samples;

      memcpy ( d->buffer, source.d->buffer, length * sizeof ( SampleFloat32 ) );

      return true;
    }
    else
    {
      return false;
    }
  }

	RawAudioBuffer AudioBuffer::samples() const
	{
		return RawAudioBuffer( d->buffer, d->samples, Float32 );
	}
	
  /*const SampleFloat32 * AudioBuffer::samples() const
  {
    return d->buffer;
  }*/

}
