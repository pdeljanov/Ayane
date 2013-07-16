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

#include <cstdint>

#define ConvDecl( itype, otype ) \
inline static void convert##itype##To##otype ( Sample##itype *in, Sample##otype *out, unsigned int numSamples ) \
{ \
converterTable[itype*NumberOfSampleFormats + otype]( reinterpret_cast<uint8_t*>(in), reinterpret_cast<uint8_t*>(out), numSamples ); \
}

#define ConvDeclPrivName( itype, otype ) \
_convert##itype##To##otype

#define ConvDeclPriv( itype, otype ) \
static void ConvDeclPrivName( itype, otype ) ( uint8_t *in, uint8_t *out, unsigned int numSamples )

#define ConvPair( itype, otype ) \
[ (itype * NumberOfSampleFormats) + otype ] = &ConvDeclPrivName( itype, otype )

#define ConvPairNop( itype, otype ) \
[ (itype * NumberOfSampleFormats) + otype ] = &_convertNop

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
            Float64,
            
            /** Used to determine the number of supported sample formats. */
            NumberOfSampleFormats
            
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
            
            typedef struct _Descriptor
            {
                /** Friendly name. */
                const char *name;
                
                /** The size of the sample in memory. */
                unsigned int stride;
                
                /** The actual size of the sample. */
                unsigned int formatSize;
                
                /** The actual number of bits contained in one sample. */
                unsigned int numBits;
            }
            Descriptor;
            
            /** Retreives information about the specified sample format. */
            static const Descriptor &about( SampleFormat format );
            
            static void convert( const RawBuffer &in, RawBuffer &out, unsigned int numSamples );
            
            ConvDecl( UInt8, Int16 );
            ConvDecl( UInt8, Int24 );
            ConvDecl( UInt8, Int32 );
            ConvDecl( UInt8, Float32 );
            ConvDecl( UInt8, Float64 );
            
            ConvDecl( Int16, UInt8 );
            ConvDecl( Int16, Int24 );
            ConvDecl( Int16, Int32 );
            ConvDecl( Int16, Float32 );
            ConvDecl( Int16, Float64 );
            
            ConvDecl( Int24, UInt8 );
            ConvDecl( Int24, Int16 );
            ConvDecl( Int24, Int32 );
            ConvDecl( Int24, Float32 );
            ConvDecl( Int24, Float64 );
            
            ConvDecl( Int32, UInt8 );
            ConvDecl( Int32, Int16 );
            ConvDecl( Int32, Int24 );
            ConvDecl( Int32, Float32 );
            ConvDecl( Int32, Float64 );
            
            ConvDecl( Float32, UInt8 );
            ConvDecl( Float32, Int16 );
            ConvDecl( Float32, Int24 );
            ConvDecl( Float32, Int32 );
            ConvDecl( Float32, Float64 );
            
            ConvDecl( Float64, UInt8 );
            ConvDecl( Float64, Int16 );
            ConvDecl( Float64, Int24 );
            ConvDecl( Float64, Int32 );
            ConvDecl( Float64, Float32 );
            
            
        private:
            
            constexpr static const Descriptor descriptorTable[] =
            {
                { "uint8"  , sizeof(SampleUInt8)  , 1,  8 },
                { "int16"  , sizeof(SampleInt16)  , 2, 16 },
                { "int24"  , sizeof(SampleInt24)  , 3, 24 },
                { "int32"  , sizeof(SampleInt32)  , 4, 32 },
                { "float32", sizeof(SampleFloat32), 4, 32 },
                { "float64", sizeof(SampleFloat64), 8, 64 }
            };
            
            typedef void ( *ConvertFunction ) ( uint8_t*, uint8_t*, unsigned int );
            
            static void _convertNop( uint8_t*, uint8_t*, unsigned int ) { return; }
            
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
   
            
            constexpr static const ConvertFunction converterTable[] =
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
        
        
        inline const SampleFormats::Descriptor &SampleFormats::about(SampleFormat format)
        {
            return descriptorTable[format];
        }

        
        
    }
}
