/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "bufferlength.h"

using namespace Ayane;

BufferLength::BufferLength() :
    mUnits(kFrames), mDuration(0.0), mFrames(0)
{
}

BufferLength::BufferLength (const Duration &duration) :
    mUnits(kTime), mDuration(duration.totalSeconds()), mFrames ( 0 )
{
}

BufferLength::BufferLength ( unsigned int frames ) :
    mUnits(kFrames), mDuration(0), mFrames(frames)
{
}

BufferLength::BufferLength ( const BufferLength& other ) :
    mUnits(other.mUnits), mDuration(other.mDuration), mFrames(other.mFrames)
{
}

double BufferLength::duration(SampleRate rate) const {
    
    return ((mUnits == kTime) ? mDuration : (mFrames * rate));
}

unsigned int BufferLength::frames ( SampleRate rate ) const {
    
    return ((mUnits == kFrames) ? mFrames : (mDuration * rate));
}

bool BufferLength::isNil() const {
    
    if ((mFrames == 0) && (mDuration == 0.0)){
        return true;
    }
    
    return false;
}

BufferLength::LengthUnits BufferLength::units() const {
    
    return mUnits;
}

BufferLength& BufferLength::operator=(const BufferLength& other) {
    
    mUnits = other.mUnits;
    mDuration = other.mDuration;
    mFrames = other.mFrames;
    return (*this);
}

bool BufferLength::operator==(const BufferLength& other) const {
    
    return ((mUnits == other.mUnits) &&
            (mFrames == other.mFrames) &&
            (mDuration == other.mDuration));
}
