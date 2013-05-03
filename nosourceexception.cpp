#include "nosourceexception.h"

namespace Ayane
{

  const char* NoSourceException::what() const throw()
  {
    return "There is no source attached to this object's sink interface.";
  }

}
