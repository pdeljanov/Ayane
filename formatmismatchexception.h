#pragma once

#include "audiobufferformat.h"

#include <exception>

namespace Ayane
{

  class FormatMismatchException : public std::exception
  {

    public:
      FormatMismatchException( const AudioBufferFormat &expected, const AudioBufferFormat &received );

      virtual const char* what() const throw();

      AudioBufferFormat expected() const;
      AudioBufferFormat received() const;
      
  private:
    
    AudioBufferFormat m_expected;
    AudioBufferFormat m_received;

  };

}
