/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_AUDIO_BUFFERFRAMES_H_
#define AYANE_AUDIO_BUFFERFRAMES_H_

#include <cstdio>
#include "attributes.h"
#include "bufferformat.h"
#include "formats.h"

namespace Ayane {
    
    /**
     *  Mono represents a frame of audio with 1 channel.
     */
    template< typename SampleType >
    struct Mono
    {
    public:
        union
        {
            /** Mono channel (physical output is hardware dependent). */
            SampleType FC;
            SampleType raw[1] = { 0 };
        };
    };
    
    
    /**
     *  Stereo represents a frame of audio with 2 (Left, Right) channels.
     */
    template< typename SampleType >
    struct Stereo
    {
    public:
        union
        {
            struct
            {
                /** Left channel. */
                SampleType FL;
                /** Right channel. */
                SampleType FR;
            };
            /** Raw sample access */
            SampleType raw[2] = { 0 };
        };
    };
    
    
    /**
     *  Stereo21 represents a frame of audio with 2.1 (Left, Right, LFE) channels.
     */
    template< typename SampleType >
    struct Stereo21
    {
    public:
        union
        {
            struct
            {
                /** Left channel. */
                SampleType FL;
                /** Right channel. */
                SampleType FR;
                /** Low frequency channel. */
                SampleType LFE;
            };
            /** Raw sample access */
            SampleType raw[3] = { 0 };
        };
    };
    
    /**
     *  MultiChannel3 is the base storage for Stereo30/31, and Surround30/31.
     */
    template< typename SampleType >
    struct MultiChannel3
    {
    public:
        union
        {
            struct
            {
                /** Left channel. */
                SampleType FL;
                /** Right channel. */
                SampleType FR;
                /** Front center channel. */
                SampleType FC;
                /** Low frequency channel. */
                SampleType LFE;
                /** Back center channel */
                SampleType BC;
            };
            /** Raw sample access */
            SampleType raw[5] = { 0 };
        };
    };
    
    /**
     *  MultiChannel4 is the base storage for Quad40/41, and Surround40/41.
     */
    template< typename SampleType >
    struct MultiChannel4
    {
    public:
        union
        {
            struct
            {
                /** Left channel. */
                SampleType FL;
                /** Right channel. */
                SampleType FR;
                /** Front center channel. */
                SampleType FC;
                /** Low frequency channel. */
                SampleType LFE;
                /** Back left channel. */
                SampleType BL;
                /** Back right channel. */
                SampleType BR;
                /** Back center channel */
                SampleType BC;
            };
            /** Raw sample access */
            SampleType raw[7] = { 0 };
        };
    };
    
    
    /**
     *  MultiChannel5 is the base storage for Surround50/51, and Surround50Side/51.
     */
    template< typename SampleType >
    struct MultiChannel5
    {
    public:
        union
        {
            struct
            {
                /** Left channel. */
                SampleType FL;
                /** Right channel. */
                SampleType FR;
                /** Front center channel. */
                SampleType FC;
                /** Low frequency channel. */
                SampleType LFE;
                /** Back left channel. */
                SampleType BL;
                /** Back right channel. */
                SampleType BR;
                /** Side left channel. */
                SampleType SL;
                /** Side right channel. */
                SampleType SR;
            };
            /** Raw sample access */
            SampleType raw[8] = { 0 };
        };
    };
    
    /**
     *  MultiChannel6 is the base storage for Surround60/61, and Surround60Side/61.
     */
    template< typename SampleType >
    struct MultiChannel6
    {
    public:
        union
        {
            struct
            {
                /** Left channel. */
                SampleType FL;
                /** Right channel. */
                SampleType FR;
                /** Front center channel. */
                SampleType FC;
                /** Low frequency channel. */
                SampleType LFE;
                /** Back left channel. */
                SampleType BL;
                /** Back right channel. */
                SampleType BR;
                /** Back center channel. */
                SampleType BC;
                /** Side left channel. */
                SampleType SL;
                /** Side right channel. */
                SampleType SR;
            };
            /** Raw sample access */
            SampleType raw[9] = { 0 };
        };
    };
    
    /**
     *  MultiChannel7 is the base storage for Surround70Front/71, Surround70Side/71,
     *  and Surround70/71.
     */
    template< typename SampleType >
    struct MultiChannel7
    {
    public:
        union
        {
            struct
            {
                /** Left channel. */
                SampleType FL;
                /** Right channel. */
                SampleType FR;
                /** Front center channel. */
                SampleType FC;
                /** Low frequency channel. */
                SampleType LFE;
                /** Back left channel. */
                SampleType BL;
                /** Back right channel. */
                SampleType BR;
                /** Front left of center channel. */
                SampleType FLc;
                /** Front right of center channel. */
                SampleType FRc;
                /** Side left channel. */
                SampleType SL;
                /** Side right channel. */
                SampleType SR;
            };
            /** Raw sample access */
            SampleType raw[10] = { 0 };
        };
    };
    
    
    /**
     *  Stereo30 represents a frame of audio with 3.0 (Left, Right, Front Center) channels.
     */
    template< typename SampleType >
    struct Stereo30 : public MultiChannel3<SampleType>
    {
    private:
        using MultiChannel3<SampleType>::LFE;
        using MultiChannel3<SampleType>::BC;
    };
    
    /**
     *  Surround30 represents a frame of audio with 3.0 (Left, Right, Back Center) channels.
     */
    template< typename SampleType >
    struct Surround30 : public MultiChannel3<SampleType>
    {
    private:
        using MultiChannel3<SampleType>::LFE;
        using MultiChannel3<SampleType>::FC;
    };
    
    /**
     *  Stereo31 represents a frame of audio with 3.1 (Left, Right, Front Center, LFE) channels.
     */
    template< typename SampleType >
    struct Stereo31 : public MultiChannel3<SampleType>
    {
    private:
        using MultiChannel4<SampleType>::BC;
    };
    
    /**
     *  Surround31 represents a frame of audio with 3.1 (Left, Right, Back Center, LFE) channels.
     */
    template< typename SampleType >
    struct Surround31 : public MultiChannel3<SampleType>
    {
    private:
        using MultiChannel4<SampleType>::FC;
    };
    
    
    /**
     *  Quad40 represents a frame of audio with 4.0 (Left, Right, Back Left, Back Right) channels.
     */
    template< typename SampleType >
    struct Quad40 : public MultiChannel4<SampleType>
    {
    private:
        using MultiChannel4<SampleType>::LFE;
        using MultiChannel4<SampleType>::BL;
        using MultiChannel4<SampleType>::BR;
    };
    
    /**
     *  Surround40 represents a frame of audio with 4.0 (Left, Right, Back Center, Front Center) channels.
     */
    template< typename SampleType >
    struct Surround40 : public MultiChannel4<SampleType>
    {
    private:
        using MultiChannel4<SampleType>::LFE;
        using MultiChannel4<SampleType>::FC;
        using MultiChannel4<SampleType>::BC;
    };
    
    /**
     *  Quad40 represents a frame of audio with 4.1 (Left, Right, Back Left, Back Right, LFE) channels.
     */
    template< typename SampleType >
    struct Quad41 : public MultiChannel4<SampleType>
    {
    private:
        using MultiChannel4<SampleType>::BL;
        using MultiChannel4<SampleType>::BR;
    };
    
    /**
     *  Surround40 represents a frame of audio with 4.1 (Left, Right, Back Center, Front Center, LFE) channels.
     */
    template< typename SampleType >
    struct Surround41 : public MultiChannel4<SampleType>
    {
    private:
        using MultiChannel4<SampleType>::FC;
        using MultiChannel4<SampleType>::BC;
    };
    
    
    /**
     *  Surround50 represents a frame of audio with 5.0 (Left, Right, Front Center, Back Left, Back Right) channels.
     */
    template< typename SampleType >
    struct Surround50 : public MultiChannel5<SampleType>
    {
    private:
        using MultiChannel5<SampleType>::LFE;
        using MultiChannel5<SampleType>::SL;
        using MultiChannel5<SampleType>::SR;
    };
    
    /**
     *  Surround50Side represents a frame of audio with 5.0 (Left, Right, Front Center, Side Left, Side Right) channels.
     */
    template< typename SampleType >
    struct Surround50Side : public MultiChannel5<SampleType>
    {
    private:
        using MultiChannel5<SampleType>::LFE;
        using MultiChannel5<SampleType>::BL;
        using MultiChannel5<SampleType>::BR;
    };
    
    /**
     *  Surround51 represents a frame of audio with 5.1 (Left, Right, Front Center, Back Left, Back Right, LFE) channels.
     */
    template< typename SampleType >
    struct Surround51 : public MultiChannel5<SampleType>
    {
    private:
        using MultiChannel5<SampleType>::SL;
        using MultiChannel5<SampleType>::SR;
    };
    
    /**
     *  Surround51Side represents a frame of audio with 5.1 (Left, Right, Front Center, Side Left, Side Right, LFE) channels.
     */
    template< typename SampleType >
    struct Surround51Side : public MultiChannel5<SampleType>
    {
    private:
        using MultiChannel5<SampleType>::BL;
        using MultiChannel5<SampleType>::BR;
    };
    
    
    /**
     *  Surround60 represents a frame of audio with 6.0 (Left, Right, Front Center,
     *  Back Left, Back Right, Back Center) channels.
     */
    template< typename SampleType >
    struct Surround60 : public MultiChannel6<SampleType>
    {
    private:
        using MultiChannel6<SampleType>::LFE;
        using MultiChannel6<SampleType>::SL;
        using MultiChannel6<SampleType>::SR;
    };
    
    /**
     *  Surround60Side represents a frame of audio with 6.0 (Left, Right, Front Center,
     *  Side Left, Side Right, Back Center) channels.
     */
    template< typename SampleType >
    struct Surround60Side : public MultiChannel6<SampleType>
    {
    private:
        using MultiChannel6<SampleType>::LFE;
        using MultiChannel6<SampleType>::BL;
        using MultiChannel6<SampleType>::BR;
    };
    
    /**
     *  Surround61 represents a frame of audio with 6.1 (Left, Right, Front Center,
     *  Back Left, Back Right, Back Center, LFE) channels.
     */
    template< typename SampleType >
    struct Surround61 : public MultiChannel6<SampleType>
    {
    private:
        using MultiChannel6<SampleType>::SL;
        using MultiChannel6<SampleType>::SR;
    };
    
    /**
     *  Surround61Side represents a frame of audio with 6.1 (Left, Right, Front Center,
     *  Side Left, Side Right, Back Center, LFE) channels.
     */
    template< typename SampleType >
    struct Surround61Side : public MultiChannel6<SampleType>
    {
    private:
        using MultiChannel6<SampleType>::BL;
        using MultiChannel6<SampleType>::BR;
    };
    
    
    /**
     *  Surround70Front represents a frame of audio with 7.0 (Left, Right, Front Center,
     *  Back Left, Back Right, Front Left of Center, Front Right of Center)
     *  channels.
     */
    template< typename SampleType >
    struct Surround70Front : public MultiChannel7<SampleType>
    {
    private:
        using MultiChannel7<SampleType>::LFE;
        using MultiChannel7<SampleType>::SL;
        using MultiChannel7<SampleType>::SR;
    };
    
    /**
     *  Surround70Side represents a frame of audio with 7.0 (Left, Right, Front Center,
     *  Front Left of Center, Front Right of Center, Side Left, Side Right)
     *  channels.
     */
    template< typename SampleType >
    struct Surround70Side : public MultiChannel7<SampleType>
    {
    private:
        using MultiChannel7<SampleType>::LFE;
        using MultiChannel7<SampleType>::BL;
        using MultiChannel7<SampleType>::BR;
    };
    
    /**
     *  Surround70 represents a frame of audio with 7.0 (Left, Right, Front Center,
     *  Back Left, Back Right, Side Left, Side Right) channels.
     */
    template< typename SampleType >
    struct Surround70 : public MultiChannel7<SampleType>
    {
    private:
        using MultiChannel7<SampleType>::LFE;
        using MultiChannel7<SampleType>::FLc;
        using MultiChannel7<SampleType>::FRc;
    };
    
    /**
     *  Surround71Front represents a frame of audio with 7.1 (Left, Right, Front Center,
     *  Back Left, Back Right, Front Left of Center, Front Right of Center, LFE)
     *  channels.
     */
    template< typename SampleType >
    struct Surround71Front : public MultiChannel7<SampleType>
    {
    private:
        using MultiChannel7<SampleType>::SL;
        using MultiChannel7<SampleType>::SR;
    };
    
    /**
     *  Surround71Side represents a frame of audio with 7.1 (Left, Right, Front Center,
     *  Front Left of Center, Front Right of Center, Side Left, Side Right, LFE)
     *  channels.
     */
    template< typename SampleType >
    struct Surround71Side : public MultiChannel7<SampleType>
    {
    private:
        using MultiChannel7<SampleType>::BL;
        using MultiChannel7<SampleType>::BR;
    };
    
    /**
     *  Surround71 represents a frame of audio with 7.1 (Left, Right, Front Center,
     *  Back Left, Back Right, Side Left, Side Right, LFE) channels.
     */
    template< typename SampleType >
    struct Surround71 : public MultiChannel7<SampleType>
    {
    private:
        using MultiChannel7<SampleType>::FLc;
        using MultiChannel7<SampleType>::FRc;
    };
    
}

#endif