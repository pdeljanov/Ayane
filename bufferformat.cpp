#include "bufferformat.h"

using namespace Stargazer::Audio;

BufferFormat::BufferFormat() :
    m_channels ( 0 ),
    m_sampleRate ( 0 ),
    m_samplesPerFrame ( 0 )
{
    
}

BufferFormat::BufferFormat ( Channels channels, SampleRate sampleRate ) :
    m_channels ( channels ),
    m_sampleRate ( sampleRate ),
    m_samplesPerFrame ( 0 )
{
    
    // # of set bits in 0x00 to 0x0f
    const static int lookup[] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
    
    for ( ; channels != 0; channels >>= 4 )
        m_samplesPerFrame += lookup[ channels & 0x0f ];
    
}

BufferFormat::BufferFormat ( const BufferFormat& format ) :
    m_channels ( format.m_channels ),
    m_sampleRate ( format.m_sampleRate ),
    m_samplesPerFrame ( format.m_samplesPerFrame )
{
    
}

Channels BufferFormat::channels() const
{
    return m_channels;
}

unsigned int BufferFormat::channelCount() const
{
    return m_samplesPerFrame;
}

SampleRate BufferFormat::sampleRate() const
{
    return m_sampleRate;
}

bool BufferFormat::isValid() const
{
    return ( ( m_channels > 0 ) && ( m_sampleRate > 0 ) );
}

BufferFormat& BufferFormat::operator= ( const BufferFormat& right )
{
    m_channels = right.m_channels;
    m_sampleRate = right.m_sampleRate;
    m_samplesPerFrame = right.m_samplesPerFrame;
    return *this;
}

bool BufferFormat::operator== ( const BufferFormat& right ) const
{
    return ( m_channels == right.m_channels ) && ( m_sampleRate == right.m_sampleRate );
}

bool BufferFormat::operator!= ( const BufferFormat& right ) const
{
    return ( ( m_channels != right.m_channels ) || ( m_sampleRate != right.m_sampleRate ) );
}

bool BufferFormat::operator< ( const BufferFormat& right ) const
{
    if( m_sampleRate < right.m_sampleRate )
    {
        return true;
    }
    else if( m_sampleRate == right.m_sampleRate )
    {
        return ( m_samplesPerFrame < right.m_samplesPerFrame );
    }
    else
    {
        return false;
    }
}

bool BufferFormat::operator<= ( const BufferFormat& right ) const
{
    if( m_sampleRate < right.m_sampleRate )
    {
        return true;
    }
    else if( m_sampleRate == right.m_sampleRate )
    {
        return ( m_samplesPerFrame <= right.m_samplesPerFrame );
    }
    else
    {
        return false;
    }
}

bool BufferFormat::operator> ( const BufferFormat& right ) const
{
    return !( *this <= right );
}

bool BufferFormat::operator>= ( const BufferFormat& right ) const
{
    return !( *this < right );
}
