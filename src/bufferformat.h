/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_AUDIO_BUFFERFORMAT_H_
#define AYANE_AUDIO_BUFFERFORMAT_H_

#include "channels.h"
#include "formats.h"

namespace Ayane {
        
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

#endif
