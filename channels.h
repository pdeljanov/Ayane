/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef STARGAZER_STDLIB_AUDIO_CHANNELS_H_
#define STARGAZER_STDLIB_AUDIO_CHANNELS_H_

#include <core/macros.h>
#include <cstdint>

namespace Stargazer
{
    namespace Audio
    {
        
        /** Type used for storing a channel list (upto 32). */
        typedef uint32_t Channels;
        
        /** Enumeration of channels. **/
        enum Channel
        {
            /** Front left (FL) */
            kFrontLeft           = 1<<0,
            
            /** Front right (FR) */
            kFrontRight          = 1<<1,
            
            /** Front centre (FC) */
            kFrontCenter         = 1<<2,
            
            /** Low frequency (LFE) */
            kLowFrequencyOne     = 1<<3,
            
            /** Back left (BL) */
            kBackLeft            = 1<<4,
            
            /** Back right (BR) */
            kBackRight           = 1<<5,
            
            /** Front left of center (FLc) */
            kFrontLeftOfCenter   = 1<<6,
            
            /** Front right of center (FRc) */
            kFrontRightOfCenter  = 1<<7,
            
            /** Back center (BC) */
            kBackCenter          = 1<<8,
            
            /** Side left (SL) */
            kSideLeft            = 1<<9,
            
            /** Side right (SR) */
            kSideRight           = 1<<10
            
            /** Front left height (FLh) */
            /*kFrontLeftHeight     = 1<<12,*/
            
            /** Front right height (FRh) */
            /*kFrontRightHeight    = 1<<14*/
        };
        
        const int kMaximumChannels = 11;
        
        const Channels kChannelMask = 0x7FF;

        static Channel kCanonicalChannels[] =
        {
            kFrontLeft,
            kFrontRight,
            kFrontCenter,
            kLowFrequencyOne,
            kBackLeft,
            kBackRight,
            kFrontLeftOfCenter,
            kFrontRightOfCenter,
            kBackCenter,
            kSideLeft,
            kSideRight
        };
        
        
        /**
         *  CanonicalChannels is a utility class for dealing with audio channels
         *  in Stargazer Audio specific ordering.
         */
        class CanonicalChannels {
            
        public:
            
            /**
             *  Gets the canonical index of a channel.
             */
            static int indexOf( Channel name ) {
                return __builtin_ctz(name);
            }
            
        private:
            STARGAZER_DISALLOW_DEFAULT_CTOR_COPY_AND_ASSIGN(CanonicalChannels);
            
        };
        

        /** Enumeration of common channel layouts. **/
        enum ChannelLayout
        {
            /** Mono 1.0 */
            kMono10          = kFrontCenter,
            
            /** Stereo 2.0 */
            kStereo20        = kFrontLeft | kFrontRight,
            
            /** Stereo 2.1 */
            kStereo21        = kStereo20 | kLowFrequencyOne,
            
            /** Stereo 3.0 */
            kStereo30        = kFrontLeft | kFrontRight | kFrontCenter,
            
            /** Stereo 3.1 */
            kStereo31        = kStereo30 | kLowFrequencyOne,
            
            /** Surround 3.0 */
            kSurround30      = kFrontLeft | kFrontRight | kBackCenter,
            
            /** Surround 3.1 */
            kSurround31      = kSurround30 | kLowFrequencyOne,
            
            /** Quad 4.0 */
            kQuad40          = kFrontLeft | kFrontRight | kBackLeft | kBackRight,
            
            /** Quad 4.1 */
            kQuad41          = kQuad40 | kLowFrequencyOne,
            
            /** Surround 4.0 */
            kSurround40      = kFrontLeft | kFrontRight | kFrontCenter | kBackCenter,
            
            /** Surround 4.1 */
            kSurround41      = kSurround40 | kLowFrequencyOne,
            
            /** Surround 5.0 */
            kSurround50      = kFrontLeft | kFrontRight | kFrontCenter | kBackLeft | kBackRight,
            
            /** Surround 5.1 */
            kSurround51      = kSurround50 | kLowFrequencyOne,
            
            /** Side 5.0 */
            kSurround50Side  = kFrontLeft | kFrontRight | kFrontCenter | kSideLeft | kSideRight,
            
            /** Surround 5.1 using side channels. */
            kSurround51Side  = kSurround50Side | kLowFrequencyOne,
            
            /** Surround 6.0 */
            kSurround60      = kFrontLeft | kFrontRight | kFrontCenter | kBackLeft | kBackRight | kBackCenter,
            
            /** Surround 6.1 */
            kSurround61      = kSurround60 | kLowFrequencyOne,
            
            /** Surround 6.0 side using side channels. */
            kSurround60Side  = kFrontLeft | kFrontRight | kFrontCenter | kSideLeft | kSideRight | kBackCenter,
            
            /** Side 6.1 */
            kSurround61Side  = kSurround60Side | kLowFrequencyOne,
            
            /** Surround 7.0 [Front, Back, FrontOfCentre] */
            kSurround70Front = kFrontLeft | kFrontRight | kFrontCenter | kBackLeft | kBackRight | kFrontLeftOfCenter | kFrontRightOfCenter,
            
            /** Surround 7.1 [Front, Back, FrontOfCentre] */
            kSurround71Front = kSurround70Front | kLowFrequencyOne,
            
            /** Surround 7.0 [Front, FrontOfCentre, Side] */
            kSurround70Side  = kFrontLeft | kFrontRight | kFrontCenter | kFrontLeftOfCenter | kFrontRightOfCenter | kSideLeft | kSideRight,
            
            /** Surround 7.1 [Front, FrontOfCentre, Side] */
            kSurround71Side  = kSurround70Side | kLowFrequencyOne,
            
            /** Surround 7.0 [Front, Back, Side] aka the canonical 7.0 surround sound */
            kSurround70      = kFrontLeft | kFrontRight | kFrontCenter | kBackLeft | kBackRight | kSideLeft | kSideRight,
            
            /** Surround 7.1 [Front, Back, Side] aka the canonical 7.1 surround sound */
            kSurround71      = kSurround70 | kLowFrequencyOne,
            
            /** Surround 9.0 */
            /*kSurround90     = kSurround70 | kFrontLeftOfCenter | kFrontRightOfCenter,*/
            
            /** Surround 9.1 */
            /*kSurround91     = kSurround90 | kLowFrequencyOne,*/
            
            /** Surround 11.0 */
            /*kSurround110    = kSurround90 | kFrontLeftHeight | kFrontRightHeight,*/
            
            /** Surround 11.1 */
            /*kSurround111    = kSurround110 | kLowFrequencyOne*/
        };
        
        
    }
    
}

#endif
