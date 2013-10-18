#ifndef STARGAZER_STDLIB_AUDIO_BUFFERLENGTH_H_
#define STARGAZER_STDLIB_AUDIO_BUFFERLENGTH_H_

#include "formats.h"

/*  Ayane
 *
 *  Created by Philip Deljanov on 12-01-06.
 *  Copyright 2012 Philip Deljanov. All rights reserved.
 *
 */

/** @file bufferlength.h
 * \brief Represents an audio buffer length which may be specified in frames or time duration.
 **/

namespace Stargazer
{
    namespace Audio
    {
        
        class BufferLength
        {
            friend class Buffer;
            
        public:
            
            enum TimeBase
            {
                Frames,
                Duration
            };
            
            BufferLength( );
            BufferLength( double duration );
            BufferLength( unsigned int frames );
            BufferLength( const BufferLength& other );
            
            TimeBase timeBase() const;
            
            double duration( SampleRate rate = 0 ) const;
            unsigned int frames( SampleRate rate = 0 ) const;
            
            bool isNil() const;
            
            BufferLength& operator= ( const BufferLength& other );
            bool operator== ( const BufferLength& other ) const;
            
        private:
            
            TimeBase m_timeBase;
            double m_duration;
            unsigned int m_frames;
            
        };
        
    }
    
}

#endif
