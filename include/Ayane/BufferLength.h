/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_BUFFERLENGTH_H_
#define AYANE_BUFFERLENGTH_H_

#include "Ayane/SampleFormats.h"
#include "Ayane/Duration.h"

namespace Ayane {
    
    /**
     *  BufferLength represents the length of a buffer in either time or
     *  dimensionless frame units.
     *
     *  The length of a buffer in absolute terms is simply the number of
     *  frames the buffer can hold. However, using a length in terms of
     *  frames is problematic because the length in time will vary depending
     *  on the sample rate.  BufferLength solves this problem by
     *  transparently converting between the two units.
     */
    class BufferLength
    {
        friend class Buffer;
        
    public:
        
        /*
         *  Enumeration of time base units used to represent the buffer
         *  length.
         */
        enum LengthUnits
        {
            /** The time base units are in audio frames. */
            kFrames,
            /** The time base units are in time units. */
            kTime
        };
        
        /**
         *  Instantiates a nil buffer length.
         */
        BufferLength();
        
        /**
         *  Instantiates a buffer length with a time duration
         */
        explicit BufferLength(const Duration &duration);
        
        /**
         *  Instantuates a buffer length with a duration in frames.
         */
        explicit BufferLength(unsigned int frames);
        
        /*
         *  Copy constructor. Don't mark as explicit.
         */
        BufferLength(const BufferLength& other);
        
        /**
         *  Gets the underlying unit used to represent the length.
         */
        LengthUnits units() const;
        
        /**
         *  Gets the duration in seconds. If the underlying unit is time,
         *  the rate parameter may be omitted.
         */
        double duration( SampleRate rate = 0 ) const;
        
        /**
         *  Gets the duration in number of frames.  If the underlying unit
         *  is frames, the rate parameter may be omitted.
         */
        unsigned int frames( SampleRate rate = 0 ) const;
        
        /**
         *  Returns true if the length is 0.
         */
        bool isNil() const;
        
        BufferLength& operator= ( const BufferLength& other );
        bool operator== ( const BufferLength& other ) const;
        
    private:
        
        LengthUnits mUnits;
        double mDuration;
        unsigned int mFrames;
        
    };
    
}

#endif
