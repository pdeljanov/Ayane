/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "Ayane/Pipeline.h"
#include "Ayane/MessageBus.h"
#include "Ayane/Stage.h"
#include "Ayane/Trace.h"

using namespace Ayane;

namespace Ayane {
        
    class PipelinePrivate {
    public:
        PipelinePrivate() : mState(Stage::kDeactivated)
        {
        }
        
        /**
         *  Searches the Stages inserted into the pipeline for clock 
         *  providers, and chooses the slowest one.
         */
        ClockProvider *selectPipelineClockProvider() const;
        
        // Pipeline state (same as Stage states)
        Stage::State mState;
        std::mutex mStateMutex;

        // Message bus
        MessageBus mMessageBus;
        
        // Stage vector
        std::vector<Pipeline::StageType> mStages;
    };

}

ClockProvider *PipelinePrivate::selectPipelineClockProvider() const {
    
    // Use the first clock provider we find.
    
    return nullptr;
}



Pipeline::Pipeline() : d_ptr(new PipelinePrivate)
{
    
}

Pipeline::~Pipeline()
{
    delete d_ptr;
}

MessageBus &Pipeline::messageBus() {
    A_D(Pipeline);
    return d->mMessageBus;
}

Pipeline::iterator Pipeline::begin(){
    A_D(Pipeline);
    return d->mStages.begin();
}

Pipeline::const_iterator Pipeline::begin() const {
    A_D(const Pipeline);
    return d->mStages.begin();
}

Pipeline::iterator Pipeline::end(){
    A_D(Pipeline);
    return d->mStages.end();
}


Pipeline::const_iterator Pipeline::end() const {
    A_D(const Pipeline);
    return d->mStages.end();
}


void Pipeline::addStage(std::unique_ptr<Stage> stage) {
    A_D(Pipeline);
    
    std::lock_guard<std::mutex> lock(d->mStateMutex);
    
    // Add the Stage.
    d->mStages.push_back(std::move(stage));
}






bool Pipeline::activate() {
    A_D(Pipeline);

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
                
                ERROR_THIS("Pipeline::activate") << "Stage " << iter->get() <<
                " failed to activate." << std::endl;
                
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
        WARNING_THIS("Pipeline::activate") << "The pipeline has already been "
        "activated." << std::endl;

        return false;
    }
}


bool Pipeline::deactivate() {
    A_D(Pipeline);
    
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
    A_D(Pipeline);
    std::lock_guard<std::mutex> lock(d->mStateMutex);
    
    if( d->mState == Stage::kActivated ) {
        
        // Try to find a clock provider.
        ClockProvider *clockProvider = d->selectPipelineClockProvider();
        
        if(clockProvider == nullptr){
            ERROR_THIS("Pipeline::play") << "Could not acquire a clock "
            "provider for the pipeline." << std::endl;
            return false;
        }
        
        // TODO: Configure the clock provider.
        
        for (iterator iter = d->mStages.begin(), end = d->mStages.end();
             iter != end; ++iter)
        {
            (*iter)->play(*clockProvider);
        }
        
        // Success
        return true;
    }
    else {
        WARNING_THIS("Pipeline::activate") << "The pipeline is not activated."
        << std::endl;

        return false;
    }
    
    
    
}

bool Pipeline::stop() {
    A_D(Pipeline);
    
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
        WARNING_THIS("Pipeline::activate") << "The pipeline is not playing."
        << std::endl;
        
        return false;
    }
}




