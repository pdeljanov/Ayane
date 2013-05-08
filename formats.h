#pragma once

/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-07.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */

/** @file formats.h
 * \brief Contains type definitions for audio sample format types.
 **/

#include <stdint.h>

namespace Stargazer
{
    namespace Audio
    {
        
        enum SampleFormat
        {
            SignedInt16 = 0,
            SignedInt24,
            SignedInt32,
            UnsignedInt16,
            UnsignedInt24,
            UnsignedInt32,
            Float32,
            Float64,
            NumberOfSampleFormats
        };
        
        typedef int32_t  SampleInt32;
        typedef uint32_t SampleUInt32;
        typedef int32_t  SampleInt24;
        typedef uint32_t SampleUInt24;
        typedef int16_t  SampleInt16;
        typedef uint16_t SampleUInt16;
        typedef float    SampleFloat32;
        typedef double   SampleFloat64;
        
        typedef struct SampleFormatDescriptor
        {
            int size;
            int packedSize;
        }
        SampleFormatDescriptor;
        
        static const SampleFormatDescriptor SampleFormatDescriptors[NumberOfSampleFormats] =
        {
            { sizeof(SampleInt16),   2 },
            { sizeof(SampleInt24),   3 },
            { sizeof(SampleInt32),   4 },
            { sizeof(SampleUInt16),  2 },
            { sizeof(SampleUInt24),  3 },
            { sizeof(SampleUInt32),  4 },
            { sizeof(SampleFloat32), 4 },
            { sizeof(SampleFloat64), 8 }
        };	
        
        typedef unsigned int SampleRate;
        
    }
}
