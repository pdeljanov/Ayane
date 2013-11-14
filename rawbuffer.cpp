/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "rawbuffer.h"
#include "buffer.h"

using namespace Stargazer::Audio;

RawBuffer::RawBuffer(uint32_t frames, uint32_t channels, SampleFormat format,
                     bool planar) :
mFormat(format),
mDataLayoutIsPlanar(planar),
mFrames(frames),
mReadIndex(0),
mWriteIndex(0),
mChannelCount(channels)
{
    
    mStride = planar ? 1 : channels;
}

RawBuffer& RawBuffer::operator>> (Buffer& buffer) {
    buffer << (*this);
    return (*this);
}

RawBuffer& RawBuffer::operator<< (Buffer& buffer) {
    buffer >> (*this);
    return (*this);
}