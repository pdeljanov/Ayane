#pragma once

#include "formats.h"

/*  Ayane
 *
 *  Created by Philip Deljanov on 12-01-06.
 *  Copyright 2012 Philip Deljanov. All rights reserved.
 *
 */

/** @file audiobufferlength.h
  * \brief Represents an audio buffer length which may be specified in frames or time duration.
**/

namespace Ayane
{

  class AudioBufferLength
  {
    friend class AudioBuffer;
    
    public:
      
      enum TimeBase
      {
	Frames,
	Duration
      };
      
      AudioBufferLength( );
      AudioBufferLength( double duration );
      AudioBufferLength( unsigned int frames );
      AudioBufferLength ( const AudioBufferLength& other );
      
      TimeBase timeBase() const;
      
      double duration( SampleRate rate = 0 ) const;
      unsigned int frames( SampleRate rate = 0 ) const;
      
      bool isNil() const;
      
      AudioBufferLength& operator= ( const AudioBufferLength& other );
      bool operator== ( const AudioBufferLength& other ) const;
      
    private:
    
      TimeBase m_timeBase;
      double m_duration;
      unsigned int m_frames;

  };

}

