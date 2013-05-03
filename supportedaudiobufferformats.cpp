#include "supportedaudiobufferformats.h"

namespace Ayane
{

  SupportedAudioBufferFormats::SupportedAudioBufferFormats() :
    m_supportsAny ( false )
  {

  }

  SupportedAudioBufferFormats::SupportedAudioBufferFormats ( bool supportsAny ) :
    m_supportsAny ( supportsAny )
  {

  }

  bool SupportedAudioBufferFormats::supports ( const AudioBufferFormat& format ) const
  {
    if ( m_supportsAny )
    {
      return true;
    }

    return ( find ( format ) != end() );
  }

  bool SupportedAudioBufferFormats::supportsAnyFormat() const
  {
    return m_supportsAny;
  }


}
