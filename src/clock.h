/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_AUDIO_CLOCK_H_
#define AYANE_AUDIO_CLOCK_H_

#include "macros.h"
#include <list>
#include <mutex>
#include <condition_variable>

namespace Ayane {
    
    /**
     *  Represents a clock that is advanced asynchronously by an external
     *  driver.
     */
    class Clock {
        
    public:
        
        Clock();
        virtual ~Clock();
        
        /**
         *  Gets the current timestamp of the pipeline.
         */
        double pipelineTime() const {
            return mPipelineTime;
        }
        
        /**
         *  Gets the current output (playback) timestamp.
         */
        double presentationTime() const {
            return mPresentationTime;
        }
        
        /**
         *  Gets the time delta between the last two successive wait()
         *  calls.
         */
        double deltaTime() const {
            return mDeltaTime;
        }
        
        /**
         *  Starts the clock. Resets the running time.
         */
        void start();
        
        /**
         *  Stops the clock.
         */
        void stop();
        
        /**
         *  Resets the clock to the specified time.
         */
        void reset( double time = 0.0 );
        
        /**
         *  Advances the presentation clock by the specified time delta.
         *  All threads that are blocked on a wait() will be unblocked.
         */
        void advancePresentation( double delta );
        
        /**
         *  Advances the pipeline clock by the specified time delta.
         */
        void advancePipeline( double delta );
        
        /**
         *  Waits for the clock to advance. Returns true if the clock is
         *  running, or false if it was stopped.
         */
        bool wait();
        
    private:
        AYANE_DISALLOW_COPY_AND_ASSIGN(Clock);
        
        // Is the clock started?
        bool mStarted;
        
        // Pipeline time (current buffer timestamp).
        double mPipelineTime;
        
        // Presentation time (playback timestamp).
        double mPresentationTime;
        
        // Time between wait() calls.
        double mDeltaTime;
        
        // Latest update delta.
        double mUpdateDelta;
        
        // Mutex to protect state.
        std::mutex mStateMutex;
        
        // Condition variable to notify wait() of an advance().
        std::condition_variable mAdvanceNotification;
        
    };
    
}

#endif