/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_PIPELINE_H_
#define AYANE_PIPELINE_H_

#include <memory>
#include <vector>

#include "Ayane/Macros.h"
#include "Ayane/DPointer.h"

namespace Ayane {
    
    class Stage;
    class MessageBus;
    class PipelinePrivate;
    
    /**
     *  A pipeline is a collection of related Stages.
     */
    class Pipeline {
    public:
        
        Pipeline();
        ~Pipeline();
        
        typedef std::unique_ptr<Stage> StageType;
        typedef std::vector<StageType>::const_iterator const_iterator;
        typedef std::vector<StageType>::iterator iterator;
        
        MessageBus &messageBus();
        
        bool activate();
        bool deactivate();
        bool play();
        bool stop();
        
        iterator begin();
        const_iterator begin() const;
        
        iterator end();
        const_iterator end() const;
        
        /**
         *  Adds a Stage to the pipeline.
         */
        void addStage(std::unique_ptr<Stage> stage);
        void removeStage();
        
        Stage *operator[](int index);
        Stage *operator[](const std::string &name);
        
    private:
        AYANE_DISALLOW_COPY_AND_ASSIGN(Pipeline);
        
        PipelinePrivate *d_ptr;
        AYANE_DECLARE_PRIVATE(Pipeline);
    };
    
}

#endif