#include "invalidstateexception.h"

namespace Ayane
{

  const char* InvalidStateException::what() const throw()
  {
    return "Calling this function is not valid in the object's current state.";
  }

}
