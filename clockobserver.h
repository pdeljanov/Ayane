/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef STARGAZER_STDLIB_AUDIO_CLOCKOBSERVER_H_
#define STARGAZER_STDLIB_AUDIO_CLOCKOBSERVER_H_

#include "abstractclock.h"
#include "clock.h"

namespace Stargazer {
    namespace Audio {
        
        /**
         *  A clock observer is a proxy object that forwards calls to an 
         *  underlying clock object. Clock observers are readonly in the sense
         *  that any operation that may modify the underlying clock is a no-op.
         */
        class ClockObserver : public AbstractClock {

            friend class Clock;
            
        public:
            
            virtual ~ClockObserver();
            
            virtual inline double pipelineTime() const {
                return mClock->pipelineTime();
            }
            
            virtual inline double presentationTime() const {
                return mClock->presentationTime();
            }
            
            virtual inline double deltaTime() const {
                return mClock->deltaTime();
            }
            
            virtual void start();
            virtual void stop();
            virtual void reset( double );
            virtual void advancePresentation( double );
            virtual void advancePipeline( double );
            
            virtual inline bool wait() {
                return mClock->wait();
            }
            
        private:
            STARGAZER_DISALLOW_COPY_AND_ASSIGN(ClockObserver);
            
            ClockObserver( Clock *source );
            
            Clock *mClock;
            
        };
        
        
    }
}

#endif