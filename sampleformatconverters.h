#pragma once

#include "formats.h"

namespace Ayane
{
#define ConvertFunctionName( type ) \
  convert##type##ToFloat32

#define ConvertFunction( type, expr ) \
  static void ConvertFunctionName( type ) (float *po, uint8_t *pi, unsigned int length) \
  { \
    for( unsigned int i = 0; i < length; ++i ) \
    { \
       po[i] = expr; \
       pi += sizeof(type); \
    }\
  }

  class SampleFormatConverters
  {

    public:

      typedef void ( *ConvertFunction ) ( float *, uint8_t *, unsigned int );

      ConvertFunction ( SampleInt16, * ( const int16_t* ) pi * ( 1.0f / ( 1 << 15 ) ) )
      ConvertFunction ( SampleInt24, * ( const int32_t* ) pi * ( 1.0f / ( 1 << 23 ) ) )
      ConvertFunction ( SampleInt32, * ( const int32_t* ) pi * ( 1.0f / ( 1 << 31 ) ) )

      static const ConvertFunction Converter[];

  };



}


