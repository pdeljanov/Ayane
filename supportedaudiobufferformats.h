#pragma once

#include <set>

#include "audiobufferformat.h"
#include "formats.h"
#include "channels.h"

namespace Ayane
{

  class SupportedAudioBufferFormats : public std::set<AudioBufferFormat>
  {

    public:
      SupportedAudioBufferFormats();
      SupportedAudioBufferFormats ( bool supportsAny );

      typedef std::set<AudioBufferFormat>::iterator iterator;
      typedef std::set<AudioBufferFormat>::const_iterator const_iterator;

      bool supports ( const AudioBufferFormat &format ) const;

      bool supportsAnyFormat() const;

    private:

      bool m_supportsAny;

  };

}
