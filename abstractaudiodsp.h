#pragma once

#include "abstractaudiosource.h"
#include "abstractaudiosink.h"

namespace Ayane
{

  class AbstractAudioDSP : public AbstractAudioSource, public AbstractAudioSink
  {

  };

}
