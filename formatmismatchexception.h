#ifndef STARGAZER_STDLIB_AUDIO_FORMATMISMATCHEXCEPTION_H_
#define STARGAZER_STDLIB_AUDIO_FORMATMISMATCHEXCEPTION_H_

#include "bufferformat.h"

#include <exception>

namespace Stargazer
{
    namespace Audio
    {
        
        class FormatMismatchException : public std::exception
        {
            
        public:
            FormatMismatchException( const BufferFormat &expected, const BufferFormat &received );
            
            virtual const char* what() const throw();
            
            BufferFormat expected() const;
            BufferFormat received() const;
            
        private:
            
            BufferFormat m_expected;
            BufferFormat m_received;
            
        };
        
    }
    
}

#endif
