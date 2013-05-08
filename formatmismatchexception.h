#pragma once

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
