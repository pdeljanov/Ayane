#include "duration.h"

#include <cmath>

using namespace Stargazer::Audio;

Duration::Duration ( double seconds ) : m_time ( seconds )
{
    
}

Duration::Duration ( unsigned int minutes, double seconds )
{
    m_time = ( minutes * 60.0f ) + seconds;
}

Duration::Duration ( unsigned int hours, unsigned int minutes, double seconds )
{
    m_time = ( hours * 3600.0f ) + ( minutes * 60.0f ) + seconds;
}

Duration::Duration ( unsigned int days, unsigned int hours, unsigned int minutes, double seconds )
{
    m_time = ( days * 24.0f * 3600.0f ) + ( hours * 3600.0f ) + ( minutes * 60.0f ) + seconds;
}

double Duration::totalSeconds() const
{
    return m_time;
}

double Duration::totalMinutes() const
{
    return ( m_time / 60.0f );
}

double Duration::totalHours() const
{
    return ( m_time / 3600.0f );
}

double Duration::totalDays() const
{
    return ( m_time / ( 24.0f * 3600.0f ) );
}

double Duration::seconds() const
{
    return fmod ( m_time, 60.0f );
}

unsigned int Duration::minutes() const
{
    double minutes = m_time / 60.0f;
    return ( ( unsigned int ) minutes );
}

unsigned int Duration::hours() const
{
    double hours = m_time / 3600.0f;
    return ( ( unsigned int ) hours );
}

unsigned int Duration::days() const
{
    double days = m_time / ( 24.0f * 60.0f );
    return ( ( unsigned int ) days );
}

bool Duration::operator== ( const Duration& rhs ) const
{
    return ( m_time == rhs.m_time );
}

bool Duration::operator!= ( const Duration& rhs ) const
{
    return ( m_time != rhs.m_time );
}

bool Duration::operator< ( const Duration& rhs ) const
{
    return ( m_time < rhs.m_time );
}

bool Duration::operator<= ( const Duration& rhs ) const
{
    return ( m_time <= rhs.m_time );
}

bool Duration::operator> ( const Duration& rhs ) const
{
    return ( m_time > rhs.m_time );
}

bool Duration::operator>= ( const Duration& rhs ) const
{
    return ( m_time >= rhs.m_time );
}

Duration& Duration::operator= ( const Duration& rhs )
{
    m_time = rhs.m_time;
    return *this;
}

Duration Duration::operator+ ( const Duration& rhs )
{
    return Duration ( m_time + rhs.m_time );
}

Duration Duration::operator- ( const Duration& rhs )
{
    return Duration ( m_time - rhs.m_time );
}
