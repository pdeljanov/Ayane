/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "bufferfactory.h"

using namespace Stargazer::Audio;

Buffer *BufferFactory::make(SampleFormat sampleFormat,
                            const BufferFormat &format,
                            const BufferLength &length)
{
    switch (sampleFormat)
    {
        case kInt16:
            return new Int16Buffer(format, length);
        case kInt32:
            return new Int32Buffer(format, length);
        case kFloat32:
            return new Float32Buffer(format, length);
        case kFloat64:
            return new Float64Buffer(format, length);
        default:
            return nullptr;
    }
}