/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef STARGAZER_STDLIB_AUDIO_CLOCK_H_
#define STARGAZER_STDLIB_AUDIO_CLOCK_H_

#include "abstractclock.h"

#include <list>
#include <mutex>
#include <condition_variable>

namespace Stargazer {
    namespace Audio {

        class ClockObserver;
        
        /**
         *  Represents a clock that is advanced asynchronously by an external
         *  driver.
         */
        class Clock : public AbstractClock {
            
        public:

            Clock();
            virtual ~Clock();
            
            /**
             *  Gets the current timestamp of the pipeline.
             */
            virtual double pipelineTime() const {
                return mPipelineTime;
            }
            
            /**
             *  Gets the current output (playback) timestamp.
             */
            virtual double presentationTime() const {
                return mPresentationTime;
            }
            
            /**
             *  Gets the time delta between the last two successive wait() 
             *  calls.
             */
            virtual double deltaTime() const {
                return mDeltaTime;
            }
            
            /**
             *  Starts the clock. Resets the running time.
             */
            virtual void start();
            
            /**
             *  Stops the clock.
             */
            virtual void stop();
            
            /**
             *  Resets the clock to the specified time.
             */
            virtual void reset( double time = 0.0 );
            
            /**
             *  Advances the presentation clock by the specified time delta.
             *  All threads that are blocked on a wait() will be unblocked.
             */
            virtual void advancePresentation( double delta );

            /**
             *  Advances the pipeline clock by the specified time delta.
             */
            virtual void advancePipeline( double delta );
            
            /**
             *  Waits for the clock to advance. Returns true if the clock is 
             *  running, or false if it was stopped.
             */
            virtual bool wait();
            
            /**
             *  Factory method for creating a clock observer that uses the
             *  instance of this clock as its reference.
             */
            ClockObserver *makeObserver();
            
            /**
             *  Removes a clock observer.
             */
            void removeObserver( ClockObserver *observer );
            
        private:
            STARGAZER_DISALLOW_COPY_AND_ASSIGN(Clock);
            
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
            
            // List of observing clocks.
            std::list<ClockObserver*> mObservers;
        };
        
        
        
    }
}

#endif