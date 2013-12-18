/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_AUDIO_BUFFERFACTORY_H_
#define AYANE_AUDIO_BUFFERFACTORY_H_

#include "Ayane/Buffer.h"

namespace Ayane {
        
    /**
     *  BufferFactory is a utility class to make constructing buffers of
     *  a specific type easier.
     */
    class BufferFactory
    {
    public:
        
        /**
         *  Creates a new buffer.
         */
        static Buffer* make(SampleFormat sampleFormat,
                            const BufferFormat &format,
                            const BufferLength &length);
      
    private:
        AYANE_DISALLOW_DEFAULT_CTOR_COPY_AND_ASSIGN(BufferFactory);
    };

}

#endif