#ifndef STARGAZER_STDLIB_AUDIO_SAMPLEFORMATS_H_
#define STARGAZER_STDLIB_AUDIO_SAMPLEFORMATS_H_

/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-07.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */

/** @file formats.h
 * \brief Contains type definitions for audio sample format types.
 **/

#include <core/attributes.h>
#include <cstdint>
#include <cmath>


#define DeclareConvertMany( itype, otype ) \
inline static void convert##itype##To##otype ( Sample##itype *in, Sample##otype *out, unsigned int numSamples ) \
{ \
converterTable[itype*6 + otype]( reinterpret_cast<uint8_t*>(in), reinterpret_cast<uint8_t*>(out), numSamples ); \
}


#define ConvDeclPrivName( itype, otype ) \
_convert##itype##To##otype

#define ConvDeclPriv( itype, otype ) \
static void ConvDeclPrivName( itype, otype ) ( uint8_t *in, uint8_t *out, unsigned int numSamples )

// [ (itype * NumberOfSampleFormats) + otype ] =
#define ConvPair( itype, otype ) \
&ConvDeclPrivName( itype, otype )

// [ (itype * NumberOfSampleFormats) + otype ] =
#define ConvPairNop( itype, otype ) \
&_convertNop

/*
 Clamping functions from FFMPEG's libavcodec. Updated for C++11, and supplemented
 with signed 24 bit integer clamping.
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

namespace Stargazer
{
    namespace Audio
    {
        class RawBuffer;
        
        /** Enumeration of supported sample format data types. */
        typedef enum
        {
            /** Unsigned 8bit integer sample format. */
            UInt8 = 0,
            
            /** Signed 16bit integer sample format. */
            Int16,
            
            /** Signed 24bit integer sample format. */
            Int24,
            
            /** Signed 32bit integer sample format. */
            Int32,
            
            /** 32bit floating point sample format. */
            Float32,
            
            /** 64bit floating point sample format. */
            Float64
            
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
            static const Descriptor &about( SampleFormat format );
            
            /**
             *  Converts a sample of InSampleType to OutSampleType.
             */
            template< typename InSampleType, typename OutSampleType >
            static force_inline OutSampleType convertSample( InSampleType );
            

            
            /* Public declarations for convert* functions */
            
            DeclareConvertMany( UInt8, Int16 )
            DeclareConvertMany( UInt8, Int24 )
            DeclareConvertMany( UInt8, Int32 )
            DeclareConvertMany( UInt8, Float32 )
            DeclareConvertMany( UInt8, Float64 )
            
            DeclareConvertMany( Int16, UInt8 )
            DeclareConvertMany( Int16, Int24 )
            DeclareConvertMany( Int16, Int32 )
            DeclareConvertMany( Int16, Float32 )
            DeclareConvertMany( Int16, Float64 )
            
            DeclareConvertMany( Int24, UInt8 )
            DeclareConvertMany( Int24, Int16 )
            DeclareConvertMany( Int24, Int32 )
            DeclareConvertMany( Int24, Float32 )
            DeclareConvertMany( Int24, Float64 )
            
            DeclareConvertMany( Int32, UInt8 )
            DeclareConvertMany( Int32, Int16 )
            DeclareConvertMany( Int32, Int24 )
            DeclareConvertMany( Int32, Float32 )
            DeclareConvertMany( Int32, Float64 )
            
            DeclareConvertMany( Float32, UInt8 )
            DeclareConvertMany( Float32, Int16 )
            DeclareConvertMany( Float32, Int24 )
            DeclareConvertMany( Float32, Int32 )
            DeclareConvertMany( Float32, Float64 )
            
            DeclareConvertMany( Float64, UInt8 )
            DeclareConvertMany( Float64, Int16 )
            DeclareConvertMany( Float64, Int24 )
            DeclareConvertMany( Float64, Int32 )
            DeclareConvertMany( Float64, Float32 )
            
            
        private:
            
            typedef void ( *ConvertFunction ) ( uint8_t*, uint8_t*, unsigned int );
            
            // No-op convert function.
            static void _convertNop( uint8_t*, uint8_t*, unsigned int ) { return; }
            
            /* Private convert functions. */
            
            ConvDeclPriv( UInt8, Int16 );
            ConvDeclPriv( UInt8, Int24 );
            ConvDeclPriv( UInt8, Int32 );
            ConvDeclPriv( UInt8, Float32 );
            ConvDeclPriv( UInt8, Float64 );
            
            ConvDeclPriv( Int16, UInt8 );
            ConvDeclPriv( Int16, Int24 );
            ConvDeclPriv( Int16, Int32 );
            ConvDeclPriv( Int16, Float32 );
            ConvDeclPriv( Int16, Float64 );
            
            ConvDeclPriv( Int24, UInt8 );
            ConvDeclPriv( Int24, Int16 );
            ConvDeclPriv( Int24, Int32 );
            ConvDeclPriv( Int24, Float32 );
            ConvDeclPriv( Int24, Float64 );
            
            ConvDeclPriv( Int32, UInt8 );
            ConvDeclPriv( Int32, Int16 );
            ConvDeclPriv( Int32, Int24 );
            ConvDeclPriv( Int32, Float32 );
            ConvDeclPriv( Int32, Float64 );
            
            ConvDeclPriv( Float32, UInt8 );
            ConvDeclPriv( Float32, Int16 );
            ConvDeclPriv( Float32, Int24 );
            ConvDeclPriv( Float32, Int32 );
            ConvDeclPriv( Float32, Float64 );
            
            ConvDeclPriv( Float64, UInt8 );
            ConvDeclPriv( Float64, Int16 );
            ConvDeclPriv( Float64, Int24 );
            ConvDeclPriv( Float64, Int32 );
            ConvDeclPriv( Float64, Float32 );
   
            // Format descriptor table.
            static constexpr Descriptor descriptorTable[] =
            {
                { "uint8"  , sizeof(SampleUInt8)  , 1,  8 },
                { "int16"  , sizeof(SampleInt16)  , 2, 16 },
                { "int24"  , sizeof(SampleInt24)  , 3, 24 },
                { "int32"  , sizeof(SampleInt32)  , 4, 32 },
                { "float32", sizeof(SampleFloat32), 4, 32 },
                { "float64", sizeof(SampleFloat64), 8, 64 }
            };
            
            // Convert function matrix.
            static constexpr ConvertFunction converterTable[] =
            {
                ConvPairNop( UInt8, UInt8  ),
                ConvPair( UInt8  , Int16   ),
                ConvPair( UInt8  , Int24   ),
                ConvPair( UInt8  , Int32   ),
                ConvPair( UInt8  , Float32 ),
                ConvPair( UInt8  , Float64 ),
                ConvPair( Int16  , UInt8   ),
                ConvPairNop( Int16, Int16  ),
                ConvPair( Int16  , Int24   ),
                ConvPair( Int16  , Int32   ),
                ConvPair( Int16  , Float32 ),
                ConvPair( Int16  , Float64 ),
                ConvPair( Int24  , UInt8   ),
                ConvPair( Int24  , Int16   ),
                ConvPairNop( Int24, Int24  ),
                ConvPair( Int24  , Int32   ),
                ConvPair( Int24  , Float32 ),
                ConvPair( Int24  , Float64 ),
                ConvPair( Int32  , UInt8   ),
                ConvPair( Int32  , Int16   ),
                ConvPair( Int32  , Int24   ),
                ConvPairNop( Int32, Int32  ),
                ConvPair( Int32  , Float32 ),
                ConvPair( Int32  , Float64 ),
                ConvPair( Float32, UInt8   ),
                ConvPair( Float32, Int16   ),
                ConvPair( Float32, Int24   ),
                ConvPair( Float32, Int32   ),
                ConvPairNop( Float32, Float32 ),
                ConvPair( Float32, Float64 ),
                ConvPair( Float64, UInt8   ),
                ConvPair( Float64, Int16   ),
                ConvPair( Float64, Int24   ),
                ConvPair( Float64, Int32   ),
                ConvPair( Float64, Float32 ),
                ConvPairNop( Float64, Float64 )
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
        force_inline SampleFloat64  SampleFormats::convertSample( SampleFloat32 si )
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
        
        
        


            
        
        force_inline const SampleFormats::Descriptor &SampleFormats::about(SampleFormat format)
        {
            return descriptorTable[format];
        }

        
        
        
        
    }
}



#endif
