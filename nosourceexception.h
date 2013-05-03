#pragma once

#include <exception>

namespace Ayane
{

  class NoSourceException : public std::exception
  {

    public:
      virtual const char* what() const throw();
  };

}
