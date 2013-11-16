/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef STARGAZER_STDLIB_AUDIO_PIPELINE_H_
#define STARGAZER_STDLIB_AUDIO_PIPELINE_H_

#include <memory>
#include <vector>
#include <core/macros.h>
#include <core/dpointer.h>

namespace Stargazer {
    namespace Audio {
        
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
            
            
            const MessageBus &messageBus();
            
            bool activate();
            bool deactivate();
            bool play();
            bool stop();
            
            iterator begin();
            const_iterator begin() const;
            
            iterator end();
            const_iterator end() const;
            
            void addStage();
            void removeStage();
            
            Stage *operator[](int index);
            Stage *operator[](const std::string &name);
            
        private:
            STARGAZER_DISALLOW_COPY_AND_ASSIGN(Pipeline);
            
            PipelinePrivate *d_ptr;
            STARGAZER_DECLARE_PRIVATE(Pipeline);
        };
        
    }
}

#endif