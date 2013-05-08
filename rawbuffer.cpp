#include "rawbuffer.h"
#include "logger.h"

using namespace Stargazer::Audio;

RawBuffer::RawBuffer() :
    m_buffer ( NULL ),
    m_length ( 0 ),
    m_consumed ( 0 ),
    m_format ( ( SampleFormat ) 0 )
{
    
}

RawBuffer::RawBuffer ( void* buffer, size_t length, SampleFormat format ) :
    m_buffer ( buffer ),
    m_length ( length ),
    m_consumed ( 0 ),
    m_format ( format )
{
}

size_t RawBuffer::length() const
{
    return m_length;
}

size_t RawBuffer::consumed() const
{
    return m_consumed;
}

size_t RawBuffer::remaining() const
{
    return m_length - m_consumed;
}

bool RawBuffer::isNil() const
{
    if ( ( m_buffer == NULL ) || ( m_length == 0 ) )
        return true;
    
    return false;
}

SampleFormat RawBuffer::sampleFormat() const
{
    return m_format;
}

const void* RawBuffer::buffer ( size_t consuming )
{
    uint8_t *out = static_cast<uint8_t*> ( m_buffer ) + ( m_consumed * SampleFormatDescriptors[m_format].size );
    m_consumed += consuming;
    return out;
}
