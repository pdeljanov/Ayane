#pragma once

/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-08.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */

/** @file audiobuffer.h
  * \brief Encapsulates a buffer of audio data.
**/

#include <stddef.h>

#include "audiobufferlength.h"
#include "audiobufferformat.h"
#include "rawaudiobuffer.h"
#include "duration.h"
#include "formats.h"

namespace Ayane
{
  class AudioBufferPrivate;

  class AudioBuffer
  {

    public:

      /** Enumeration of fixed point sample depths. **/
      enum FixedPointDepth
      {
        Depth24bit,
        Deptch32bit
      };

      AudioBuffer ( const AudioBufferFormat &format, const AudioBufferLength &length );
      AudioBuffer ( const AudioBuffer &source );
      ~AudioBuffer();

      /** Get the audio buffer's timestamp.  A buffer's timestamp is the duration of time elapsed since the start
        * of the stream.
      **/
      const Duration &timestamp() const;

      void setTimestamp ( const Duration &timestamp );

      /** Get the duration of the buffer.
       **/
      Duration duration() const;

      /** Returns the amount of samples the buffer can contain.
        * \deprecated This makes a leaky abstraction!!
       **/
      unsigned int length() const;

      /** Returns the amount of frames the buffer can contain. **/
      unsigned int frames() const;


      /**
      * Returns the multichannel audio format mode descriptor.
      *
      * \return The multichannel audio format.
      **/
      const AudioBufferFormat &format() const;

      /**
        * Fills the audio buffer with as many frames as possible from the source.
      *
      * Depending on the number of frames contained in the raw buffer, and the capacity
      * of the frames in the audio buffer the number of frames actually copied will vary.
      *
      * \note Only full frames are copied. If the raw buffer ends with a partial frame it will
      *       not be copied.
      *
      * \param buffer A handle to the raw buffer.
      * \param offset The number of frames to offset into the audio buffer.
      *
      * \returns The number of full frames copied.
      **/
      unsigned int fill ( RawAudioBuffer &buffer, unsigned int offset );
      
      unsigned int fill ( SampleFloat32 *buffer, unsigned int offset, unsigned int n);

      /** Replaces the contents of this buffer with the contents of the source buffer.
        *
      * The source buffer must have the same sample rate or else the copy will fail.
      * If the source buffer contains more samples than this one, the copy will be truncated.
      *
      * \param source The source buffer to copy from.
      *
      * \return True on success, false otherwise.
      **/
      bool copy ( const AudioBuffer &source );

	  RawAudioBuffer samples() const;
	  
      /** Returns a pointer to the underlying SampleFloat32 buffer. **/
      //const SampleFloat32 *samples() const;

    private:

      AudioBufferPrivate *d;

  };

}
