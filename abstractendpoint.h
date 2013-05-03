#pragma once

#include "abstractaudiosource.h"

namespace Ayane
{

  class AbstractEndPoint
  {
    public:

      virtual AbstractAudioSource::Resolve resolveAll() = 0;

  };

}
