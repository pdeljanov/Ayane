#pragma once

#include <exception>

namespace Ayane
{

  class InvalidStateException : public std::exception
  {

    public:
      virtual const char* what() const throw();
  };

}
