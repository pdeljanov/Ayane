/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "duration.h"

#include <cmath>

using namespace Ayane;

Duration::Duration ( double seconds ) : mTime ( seconds )
{
}

Duration::Duration ( unsigned int minutes, double seconds )
{
    mTime = ( minutes * 60.0f ) + seconds;
}

Duration::Duration ( unsigned int hours, unsigned int minutes, double seconds )
{
    mTime = ( hours * 3600.0f ) + ( minutes * 60.0f ) + seconds;
}

Duration::Duration ( unsigned int days, unsigned int hours, unsigned int minutes, double seconds )
{
    mTime = ( days * 24.0f * 3600.0f ) + ( hours * 3600.0f ) + ( minutes * 60.0f ) + seconds;
}

double Duration::totalSeconds() const
{
    return mTime;
}

double Duration::totalMinutes() const
{
    return ( mTime / 60.0f );
}

double Duration::totalHours() const
{
    return ( mTime / 3600.0f );
}

double Duration::totalDays() const
{
    return ( mTime / ( 24.0f * 3600.0f ) );
}

double Duration::seconds() const
{
    return fmod ( mTime, 60.0f );
}

unsigned int Duration::minutes() const
{
    double minutes = mTime / 60.0f;
    return ( ( unsigned int ) minutes );
}

unsigned int Duration::hours() const
{
    double hours = mTime / 3600.0f;
    return ( ( unsigned int ) hours );
}

unsigned int Duration::days() const
{
    double days = mTime / ( 24.0f * 60.0f );
    return ( ( unsigned int ) days );
}

bool Duration::operator== ( const Duration& rhs ) const
{
    return ( mTime == rhs.mTime );
}

bool Duration::operator!= ( const Duration& rhs ) const
{
    return ( mTime != rhs.mTime );
}

bool Duration::operator< ( const Duration& rhs ) const
{
    return ( mTime < rhs.mTime );
}

bool Duration::operator<= ( const Duration& rhs ) const
{
    return ( mTime <= rhs.mTime );
}

bool Duration::operator> ( const Duration& rhs ) const
{
    return ( mTime > rhs.mTime );
}

bool Duration::operator>= ( const Duration& rhs ) const
{
    return ( mTime >= rhs.mTime );
}

Duration& Duration::operator= ( const Duration& rhs )
{
    mTime = rhs.mTime;
    return *this;
}

Duration Duration::operator+ ( const Duration& rhs )
{
    return Duration ( mTime + rhs.mTime );
}

Duration Duration::operator- ( const Duration& rhs )
{
    return Duration ( mTime - rhs.mTime );
}
