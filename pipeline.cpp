/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "pipeline.h"
#include "messagebus.h"
#include "stage.h"
#include "trace.h"

using namespace Stargazer::Audio;

namespace Stargazer {
    namespace Audio {
        
        class PipelinePrivate {
        public:
            PipelinePrivate() : mState(Stage::kDeactivated)
            {
            }
            
            /**
             *  Searches the Stages inserted into the pipeline for clock 
             *  providers, and chooses the slowest one.
             */
            ClockProvider &selectPipelineClockProvider() const;
            
            // Pipeline state (same as Stage states)
            Stage::State mState;
            std::mutex mStateMutex;
            
            // Message bus
            MessageBus mMessageBus;
            
            // Stage vector
            std::vector<Pipeline::StageType> mStages;
        };
        
    }
}

ClockProvider &PipelinePrivate::selectPipelineClockProvider() const {
    
    // Use the first clock provider we find.
    
    
}



Pipeline::Pipeline() : d_ptr(new PipelinePrivate)
{
    
}

Pipeline::~Pipeline()
{
    delete d_ptr;
}

MessageBus &Pipeline::messageBus() {
    S_D(Pipeline);
    return d->mMessageBus;
}

bool Pipeline::activate() {
    S_D(Pipeline);

    std::lock_guard<std::mutex> lock(d->mStateMutex);
    
    if( d->mState == Stage::kDeactivated ) {
        
        // Start the message bus.
        d->mMessageBus.start();
        
        // Activate each inserted Stage.
        for (iterator iter = d->mStages.begin(), end = d->mStages.end();
             iter != end; ++iter)
        {
            // Activate stage with ownership given to the pipeline.
            if( !(*iter)->activate(this) ){
                
                WARNING_THIS("Pipeline::activate") << "Stage " << iter->get() <<
                " failed to activated." << std::endl;
                
                // Deactivate already activated Stages.
                for(end = iter, iter = d->mStages.begin();
                    iter != end; ++iter)
                {
                    (*iter)->deactivate();
                }
                
                return false;
            }
        }

        // Record state.
        d->mState = Stage::kActivated;
        
        // Success!
        return true;
        
    }
    else {
        return false;
    }
}


bool Pipeline::deactivate() {
    S_D(Pipeline);
    
    std::lock_guard<std::mutex> lock(d->mStateMutex);

    if( d->mState == Stage::kDeactivated ) {
        return false;
    }
    
    // If playing stop first.
    if( d->mState == Stage::kPlaying ) {
        stop();
    }
    
    if( d->mState == Stage::kActivated ) {
        
        // Deactivate each inserted Stage.
        for (iterator iter = d->mStages.begin(), end = d->mStages.end();
             iter != end; ++iter)
        {
            (*iter)->deactivate();
        }
        
        // Stop the message bus.
        d->mMessageBus.stop();
        
        // Record the new state.
        d->mState = Stage::kDeactivated;
    }
    
    // Success
    return true;
}

bool Pipeline::play() {
    S_D(Pipeline);
    std::lock_guard<std::mutex> lock(d->mStateMutex);
    
    if( d->mState == Stage::kActivated ) {
        
        ClockProvider &clockProvider = d->selectPipelineClockProvider();
        
        for (iterator iter = d->mStages.begin(), end = d->mStages.end();
             iter != end; ++iter)
        {
            (*iter)->play(clockProvider);
        }
        
        // Success
        return true;
    }
    else {
        return false;
    }
    
    
    
}

bool Pipeline::stop() {
    S_D(Pipeline);
    
    std::lock_guard<std::mutex> lock(d->mStateMutex);

    if( d->mState == Stage::kPlaying ) {
        
        for (iterator iter = d->mStages.begin(), end = d->mStages.end();
             iter != end; ++iter)
        {
            (*iter)->stop();
        }
        
        // Record new state.
        d->mState = Stage::kActivated;
        
        // Success
        return true;
    }
    else {
        return false;
    }
}




