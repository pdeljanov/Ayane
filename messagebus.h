/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef STARGAZER_STDLIB_AUDIO_ABSTRACTMESSAGEBUS_H_
#define STARGAZER_STDLIB_AUDIO_ABSTRACTMESSAGEBUS_H_

#include <core/macros.h>
#include <core/dpointer.h>

#include "duration.h"

namespace Stargazer {
    namespace Audio {
        
        /**
         *  Enumeration of possible messages.
         */
        typedef enum {
            
            /** Nil message. */
            kNil            = 0,
            
            /** Error message. */
            kError          = 1<<0,
            
            /** Warning message. */
            kWarning        = 1<<1,
            
            /** Informational/Trace message. */
            kTrace          = 1<<2,
            
            /** Duration change event. */
            kDuration       = 1<<3,
            
            /** Progress event. */
            kProgress       = 1<<4,
            
            /** Pipeline has reached end of stream on all sources. */
            kEndOfStream    = 1<<5,
            
            /** The clock was lost. */
            kClockLost      = 1<<6
            
        } MessageType;
        
        
        /** Message base class. */
        class MessageBase {
        protected:
            
            MessageBase(MessageType type) : mType(type)
            {
            }
            
            MessageType mType;
            
        public:
            
            /**
             *  Gets the type of the message.
             */
            MessageType typeOf() const {
                return mType;
            }

        };
        
        class ErrorMessage : public MessageBase {

        public:
            ErrorMessage() : MessageBase(kError)
            {
            }
            
            static MessageType type() { return kError; }
            
        };
        
        class WarningMessage : public MessageBase {
        public:
            WarningMessage() : MessageBase(kWarning)
            {
            }
            
            static MessageType type() { return kWarning; }
            
        };
        
        class TraceMessage : public MessageBase {
        public:
            TraceMessage() : MessageBase(kTrace)
            {
            }
            
            static MessageType type() { return kTrace; }
            
        };
        
        class DurationMessage : public MessageBase {
        public:
            DurationMessage(const Duration &duration) :
            MessageBase(kDuration),
            mDuration(duration)
            {
            }
            
            static MessageType type() { return kDuration; }
            
        private:
            Duration mDuration;
        };
        
        class ProgressMessage : public MessageBase {
        public:
            ProgressMessage() : MessageBase(kProgress)
            {
            }
            
            static MessageType type() { return kProgress; }
            
        };
        
        class EndOfStreamMessage : public MessageBase {
        public:
            EndOfStreamMessage() : MessageBase(kEndOfStream)
            {
            }
            
            static MessageType type() { return kEndOfStream; }
            
        };
        
        class ClockLostMessage : public MessageBase {
        public:
            ClockLostMessage() : MessageBase(kClockLost)
            {
            }
            
            static MessageType type() { return kClockLost; }
            
            
        };

        
        
        class MessageBusPrivate;
        
        class MessageBus {
        public:
            

            
            void post(const ErrorMessage &message);
            void post(const WarningMessage &message);
            void post(const TraceMessage &message);
            void post(const DurationMessage &message);
            void post(const ProgressMessage &message);
            void post(const EndOfStreamMessage &message);
            void post(const ClockLostMessage &message);
            
            

        private:
            STARGAZER_DISALLOW_COPY_AND_ASSIGN(MessageBus);
            
            MessageBusPrivate *d_ptr;
            STARGAZER_DECLARE_PRIVATE(MessageBus);
        };
        
        
        
        
        
        
        
        
    }
}

#endif