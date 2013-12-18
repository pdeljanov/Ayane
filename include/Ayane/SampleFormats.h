/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_SAMPLEFORMATS_H_
#define AYANE_SAMPLEFORMATS_H_

#include "Ayane/Attributes.h"
#include <cstdint>
#include <cmath>

/*
 *  Clamping functions from FFMPEG's libavcodec. Supplemented
 *  with signed 24 bit integer clamping.
 */

static inline uint8_t clip_uint8( int in )
{
    if ( in & ( ~0xFF ) )
        return ( -in ) >> 31;
    else
        return (uint8_t)in;
}

static inline int8_t clip_int8( int in )
{
    if ( ( in + 0x80 ) & ~0xFF )
        return ( in >> 31 ) ^ 0x7F;
    else
        return (int8_t)in;
}

static inline uint16_t clip_uint16( int in )
{
    if ( in & ( ~0xFFFF ) )
        return ( -in )>>31;
    else
        return (uint16_t)in;
}

static inline int16_t clip_int16( int in )
{
    if ( ( in + 0x8000 ) & ~0xFFFF )
        return ( in >> 31 ) ^ 0x7FFF;
    else
        return (int16_t)in;
}

static inline int32_t clip_int24( int in )
{
    if ( ( in + 0x800000 ) & ~0xFFFFFF )
        return ( in >> 31 ) ^ 0x7FFFFF;
    else
        return (int32_t)in;
}

static inline int32_t clip_int32( int64_t in )
{
    if ( ( in + 0x80000000u ) & ~0xFFFFFFFFul )
        return ( in >> 63 ) ^ 0x7FFFFFFF;
    else
        return (int32_t)in;
}

namespace Ayane {
    class RawBuffer;
    
    /** Enumeration of supported sample format data types. */
    typedef enum
    {
        /** Unsigned 8bit integer sample format. */
        kUInt8 = 0,
        
        /** Signed 16bit integer sample format. */
        kInt16,
        
        /** Signed 24bit integer sample format. */
        kInt24,
        
        /** Signed 32bit integer sample format. */
        kInt32,
        
        /** 32bit floating point sample format. */
        kFloat32,
        
        /** 64bit floating point sample format. */
        kFloat64
        
    } SampleFormat;
    
    
    /** Data type for a signed 32bit integer sample.  */
    typedef int32_t  SampleInt32;
    
    /** Data type for a signed 24bit integer sample. */
    typedef int32_t  SampleInt24;
    
    /** Data type for a signed 16bit sample. */
    typedef int16_t  SampleInt16;
    
    /** Data type for an unsigned 8bit sample. */
    typedef uint16_t SampleUInt8;
    
    /** Data type for a 32bit floating point sample. */
    typedef float    SampleFloat32;
    
    /** Data type for a 64bit floating point sample. */
    typedef double   SampleFloat64;
    
    
    /** Data type that should be used when representing a sample rate. */
    typedef unsigned int SampleRate;
    
    
    class SampleFormats
    {
        
    public:
        
        typedef struct
        {
            /** Friendly name. */
            const char *name;
            
            /** The size of the sample in memory. */
            unsigned int stride;
            
            /** The actual size of the sample. */
            unsigned int formatSize;
            
            /** The actual number of bits contained in one sample. */
            unsigned int numBits;
            
        } Descriptor;
        
        /** Retreives information about the specified sample format. */
        static const Descriptor &about( SampleFormat format )
        {
            return descriptorTable[format];
        }
        
        /**
         *  Converts a sample of InSampleType to OutSampleType.
         */
        template< typename InSampleType, typename OutSampleType >
        static force_inline OutSampleType convertSample( InSampleType );
        
        /**
         *  Converts many samples of InSampleType to OutSampleType.
         */
        template< typename InSampleType, typename OutSampleType >
        static void convertMany( InSampleType *src, OutSampleType *dest, int count )
        {
            for( int i = 0; i < count; ++i ) {
                dest[i] = SampleFormats::convertSample<InSampleType, OutSampleType>(src[i]);
            }
        }
        
        /**
         *  Converts many samples of InSampleType to OutSampleType with a
         *  custom source buffer stride.
         */
        template< typename InSampleType, typename OutSampleType >
        static void convertMany(InSampleType  *src, int srcStride,
                                OutSampleType *dest, int count )
        {
            OutSampleType *end = dest + count;
            
            while( dest != end ) {
                *dest = SampleFormats::convertSample<InSampleType, OutSampleType>(*src);
                src += srcStride;
                ++dest;
            }
        }
        
        /**
         *  Converts many samples of InSampleType to OutSampleType with a
         *  custom destination buffer stride.
         */
        template< typename InSampleType, typename OutSampleType >
        static void convertMany(InSampleType *src, OutSampleType *dest,
                                int destStride,
                                int count )
        {
            InSampleType *end = src + count;
            
            while( src != end ) {
                *dest = SampleFormats::convertSample<InSampleType, OutSampleType>(*src);
                ++src;
                dest += destStride;
            }
        }
        
        
        
    private:
        
        // Format descriptor table.
        // Printable Name, In Memory Size, Actual Size (bytes), Bit Width
        static constexpr Descriptor descriptorTable[] =
        {
            { "UInt8"  , sizeof(SampleUInt8)  , 1,  8 },
            { "Int16"  , sizeof(SampleInt16)  , 2, 16 },
            { "Int24"  , sizeof(SampleInt24)  , 3, 24 },
            { "Int32"  , sizeof(SampleInt32)  , 4, 32 },
            { "Float32", sizeof(SampleFloat32), 4, 32 },
            { "Float64", sizeof(SampleFloat64), 8, 64 }
        };
        
        
    };
    
    /* --- SampleFormats::convertSample(...) Specializations --- */
    
    /* SampleUInt8 convertSample(...) specializations */
    
    template<>
    force_inline SampleUInt8 SampleFormats::convertSample( SampleUInt8 si )
    { return si; }
    
    template<>
    force_inline SampleInt16 SampleFormats::convertSample( SampleUInt8 si )
    { return (si - 0x80) << 8; }
    
    template<>
    force_inline SampleInt32 SampleFormats::convertSample( SampleUInt8 si )
    { return (si - 0x80) << 24; }
    
    template<>
    force_inline SampleFloat32 SampleFormats::convertSample( SampleUInt8 si )
    { return (si - 0x80) * (1.0f / (1<<7)); }
    
    template<>
    force_inline SampleFloat64 SampleFormats::convertSample( SampleUInt8 si )
    { return (si - 0x80) * (1.0 / (1<<7)); }
    
    /* SampleInt16 convertSample(...) specializations */
    
    template<>
    force_inline SampleUInt8 SampleFormats::convertSample( SampleInt16 si )
    { return (si >> 8) + 0x80; }
    
    template<>
    force_inline SampleInt16 SampleFormats::convertSample( SampleInt16 si )
    { return si; }
    
    template<>
    force_inline SampleInt32 SampleFormats::convertSample( SampleInt16 si )
    { return (si) << 16; }
    
    template<>
    force_inline SampleFloat32 SampleFormats::convertSample( SampleInt16 si )
    { return (si) * (1.0f / (1<<15)); }
    
    template<>
    force_inline SampleFloat64 SampleFormats::convertSample( SampleInt16 si )
    { return (si) * (1.0 / (1<<15)); }
    
    /* SampleInt32 convertSample(...) specializations */
    
    template<>
    force_inline SampleUInt8 SampleFormats::convertSample( SampleInt32 si )
    { return (si >> 24 ) + 0x80; }
    
    template<>
    force_inline SampleInt16 SampleFormats::convertSample( SampleInt32 si )
    { return (si) >> 16; }
    
    template<>
    force_inline SampleInt32  SampleFormats::convertSample( SampleInt32 si )
    { return si; }
    
    template<>
    force_inline SampleFloat32 SampleFormats::convertSample( SampleInt32 si )
    { return (si) * (1.0f / (1u<<31)); }
    
    template<>
    force_inline SampleFloat64 SampleFormats::convertSample( SampleInt32 si )
    { return (si) * (1.0 / (1u<<31)); }
    
    /* SampleFloat32 convertSample(...) specializations */
    
    template<>
    force_inline SampleUInt8 SampleFormats::convertSample( SampleFloat32 si )
    { return clip_uint8( lrintf(si * (1<<7) ) + 0x80); }
    
    template<>
    force_inline SampleInt16 SampleFormats::convertSample( SampleFloat32 si )
    { return clip_int16( lrintf(si * (1<<15)) ); }
    
    template<>
    force_inline SampleInt32 SampleFormats::convertSample( SampleFloat32 si )
    { return clip_int32( llrintf(si * (1u<<31)) ); }
    
    template<>
    force_inline SampleFloat32 SampleFormats::convertSample( SampleFloat32 si )
    { return si; }
    
    template<>
    force_inline SampleFloat64 SampleFormats::convertSample( SampleFloat32 si )
    { return si; }
    
    /* SampleFloat64 convertSample(...) specializations */
    
    template<>
    force_inline SampleUInt8 SampleFormats::convertSample( SampleFloat64 si )
    { return clip_uint8( lrint(si * (1<<7)) + 0x80 ); }
    
    template<>
    force_inline SampleInt16 SampleFormats::convertSample( SampleFloat64 si )
    { return clip_int16( lrint(si * (1<<15)) ); }
    
    template<>
    force_inline SampleInt32 SampleFormats::convertSample( SampleFloat64 si )
    { return clip_int32( llrint(si * (1u<<31)) ); }
    
    template<>
    force_inline SampleFloat32 SampleFormats::convertSample( SampleFloat64 si )
    { return si; }
    
    template<>
    force_inline SampleFloat64 SampleFormats::convertSample( SampleFloat64 si )
    { return si; }
    
}



#endif
