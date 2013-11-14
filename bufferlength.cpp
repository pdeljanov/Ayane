#include "bufferlength.h"

using namespace Stargazer::Audio;

BufferLength::BufferLength() :
    m_timeBase( Frames ), m_duration( 0 ), m_frames( 0 )
{
    
}

BufferLength::BufferLength ( double duration ) :
    m_timeBase ( Duration ), m_duration ( duration ), m_frames ( 0 )
{
}

BufferLength::BufferLength ( unsigned int frames ) :
    m_timeBase ( Frames ), m_duration ( 0 ), m_frames ( frames )
{
}

BufferLength::BufferLength ( const BufferLength& other ) :
    m_timeBase ( other.m_timeBase ), m_duration ( other.m_duration ), m_frames ( other.m_frames )
{
}

double BufferLength::duration ( SampleRate rate ) const
{
    return ( m_timeBase == Duration ) ? m_duration : m_frames * rate;
}

unsigned int BufferLength::frames ( SampleRate rate ) const
{
    return ( m_timeBase == Frames ) ? m_frames : ( m_duration / 1000.0 ) * rate;
}

bool BufferLength::isNil() const
{
    if ( ( m_frames == 0 ) && ( m_duration == 0.0f ) ) return true;
    return false;
}

BufferLength::TimeBase BufferLength::timeBase() const
{
    return m_timeBase;
}

BufferLength& BufferLength::operator= ( const BufferLength& other )
{
    m_timeBase = other.m_timeBase;
    m_duration = other.m_duration;
    m_frames = other.m_frames;
    return *this;
}

bool BufferLength::operator== ( const BufferLength& other ) const
{
    return (
            ( m_timeBase == other.m_timeBase ) &&
            ( m_frames == other.m_frames ) &&
            ( m_duration == other.m_duration )
            );
}
