#include "formatmismatchexception.h"

using namespace Stargazer::Audio;

FormatMismatchException::FormatMismatchException ( const BufferFormat &expected, const BufferFormat &received ) :
    m_expected ( expected ),
    m_received ( received )
{
    
}

const char* FormatMismatchException::what() const throw()
{
    return "The requested operation required matching audio formats.";
}

BufferFormat FormatMismatchException::expected() const
{
    return m_expected;
}

BufferFormat FormatMismatchException::received() const
{
    return m_received;
}
