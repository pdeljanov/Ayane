#include "formatmismatchexception.h"

namespace Ayane
{

  FormatMismatchException::FormatMismatchException ( const AudioBufferFormat &expected, const AudioBufferFormat &received ) :
    m_expected ( expected ),
    m_received ( received )
  {

  }

  const char* FormatMismatchException::what() const throw()
  {
    return "The requested operation required matching audio formats.";
  }

  AudioBufferFormat FormatMismatchException::expected() const
  {
    return m_expected;
  }

  AudioBufferFormat FormatMismatchException::received() const
  {
    return m_received;
  }

}
