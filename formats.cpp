/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-07.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */
#include "formats.h"
#include "rawbuffer.h"


using namespace Stargazer::Audio;

#define ConvPrivImpl( itype, otype, expr )                      \
void SampleFormats::ConvDeclPrivName( itype, otype )            \
    ( uint8_t *si, uint8_t *so, unsigned int length )           \
    {                                                           \
        uint8_t *end = so + length * sizeof(Sample##otype);     \
        while(so < end) {                                       \
            *(Sample##otype*)so = expr;                         \
            si += sizeof(Sample##itype);                        \
            so += sizeof(Sample##otype);                        \
        }                                                       \
    }

// Define storage for descriptor and converter tables.
constexpr SampleFormats::Descriptor SampleFormats::descriptorTable[];
constexpr SampleFormats::ConvertFunction SampleFormats::converterTable[];




/*
 Conversion functions inspired by FFMPEG's libavcodec. Updated for C++11, and supplmented
 with signed 24 bit format conversions.
 */


ConvPrivImpl( UInt8, Int16   , ( *( const SampleUInt8* )si - 0x80 ) << 8  )
ConvPrivImpl( UInt8, Int24   , ( *( const SampleUInt8* )si - 0x80 ) << 16 )
ConvPrivImpl( UInt8, Int32   , ( *( const SampleUInt8* )si - 0x80 ) << 24 )
ConvPrivImpl( UInt8, Float32 , ( *( const SampleUInt8* )si - 0x80 ) * ( 1.0f / (1<<7) ) )
ConvPrivImpl( UInt8, Float64 , ( *( const SampleUInt8* )si - 0x80 ) * ( 1.0 / (1<<7) ) )

ConvPrivImpl( Int16, UInt8   , ( *( const SampleInt16* )si >> 8 ) + 0x80 )
ConvPrivImpl( Int16, Int24   , ( *( const SampleInt16* )si ) << 8 )
ConvPrivImpl( Int16, Int32   , ( *( const SampleInt16* )si ) << 16 )
ConvPrivImpl( Int16, Float32 , ( *( const SampleInt16* )si ) * ( 1.0f / (1<<15) ) )
ConvPrivImpl( Int16, Float64 , ( *( const SampleInt16* )si ) * ( 1.0 / (1<<15) ) )

ConvPrivImpl( Int24, UInt8   , ( *( const SampleInt24* )si >> 16 ) + 0x80 )
ConvPrivImpl( Int24, Int16   , ( *( const SampleInt24* )si ) >> 8 )
ConvPrivImpl( Int24, Int32   , ( *( const SampleInt24* )si ) << 8 )
ConvPrivImpl( Int24, Float32 , ( *( const SampleInt24* )si ) * ( 1.0f / (1<<23) ) )
ConvPrivImpl( Int24, Float64 , ( *( const SampleInt24* )si ) * ( 1.0 / (1<<23) ) )

ConvPrivImpl( Int32, UInt8   , ( *( const SampleInt32* )si >> 24 ) + 0x80 )
ConvPrivImpl( Int32, Int16   , ( *( const SampleInt32* )si ) >> 16 )
ConvPrivImpl( Int32, Int24   , ( *( const SampleInt32* )si ) >> 8 )
ConvPrivImpl( Int32, Float32 , ( *( const SampleInt32* )si ) * ( 1.0f / (1u<<31) ) )
ConvPrivImpl( Int32, Float64 , ( *( const SampleInt32* )si ) * ( 1.0 / (1u<<31) ) )

ConvPrivImpl( Float32, UInt8  , clip_uint8( lrintf(  *( const SampleFloat32* )si * (1<<7)   ) + 0x80 ) )
ConvPrivImpl( Float32, Int16  , clip_int16( lrintf(  *( const SampleFloat32* )si * (1<<15)  )        ) )
ConvPrivImpl( Float32, Int24  , clip_int24( lrintf(  *( const SampleFloat32* )si * (1<<23)  )        ) )
ConvPrivImpl( Float32, Int32  , clip_int32( llrintf( *( const SampleFloat32* )si * (1u<<31) )        ) )
ConvPrivImpl( Float32, Float64, ( *( const SampleFloat32* )si ) )

ConvPrivImpl( Float64, UInt8  , clip_uint8( lrint(  *( const SampleFloat64* )si * (1<<7)   ) + 0x80 ) )
ConvPrivImpl( Float64, Int16  , clip_int16( lrint(  *( const SampleFloat64* )si * (1<<15)  )        ) )
ConvPrivImpl( Float64, Int24  , clip_int24( lrint(  *( const SampleFloat64* )si * (1<<23)  )        ) )
ConvPrivImpl( Float64, Int32  , clip_int32( llrint( *( const SampleFloat64* )si * (1u<<31) )        ) )
ConvPrivImpl( Float64, Float32, ( *( const SampleFloat64*)si ) )