/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "pipeline.h"

using namespace Stargazer::Audio;

namespace Stargazer {
    namespace Audio {
        
        class PipelinePrivate {
        public:
            
            
        };
        
    }
}

Pipeline::Pipeline() : d_ptr(new PipelinePrivate)
{
    
}

Pipeline::~Pipeline()
{
    delete d_ptr;
}