/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "Ayane/RawBuffer.h"
#include "Ayane/Buffer.h"

using namespace Ayane;

RawBuffer& RawBuffer::operator>> (Buffer& buffer) {
    buffer << (*this);
    return (*this);
}

RawBuffer& RawBuffer::operator<< (Buffer& buffer) {
    buffer >> (*this);
    return (*this);
}
