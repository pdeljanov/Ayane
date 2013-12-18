/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_MESSAGEBUS_H_
#define AYANE_MESSAGEBUS_H_

#include <string>
#include <functional>

#include "Ayane/Macros.h"
#include "Ayane/DPointer.h"
#include "Ayane/Duration.h"

namespace Ayane {
    
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
        friend class MessageBusPrivate;
        
    public:
        
        virtual ~MessageBase(){
        }
        
        /**
         *  Gets the type of the message.
         */
        MessageType typeOf() const {
            return mType;
        }
        
    protected:
        
        MessageBase(MessageType type) : mType(type), mNext(nullptr)
        {
        }
        
        MessageType mType;
        
    private:
        std::atomic<MessageBase*> mNext;
        
    };
    
    class ErrorMessage : public MessageBase {
        
    public:
        ErrorMessage(const std::string &message) :
        MessageBase(kWarning), mMessage(message)
        {
        }
        
        static MessageType type() { return kError; }
        
    private:
        std::string mMessage;
    };
    
    class WarningMessage : public MessageBase {
    public:
        WarningMessage(const std::string &message) :
        MessageBase(kWarning), mMessage(message)
        {
        }
        
        static MessageType type() { return kWarning; }
        
    private:
        std::string mMessage;
    };
    
    class TraceMessage : public MessageBase {
    public:
        TraceMessage(const std::string &message) :
        MessageBase(kTrace), mMessage(message)
        {
        }
        
        static MessageType type() { return kTrace; }
        
    private:
        std::string mMessage;
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
        ProgressMessage(const Duration &duration) :
        MessageBase(kProgress),
        mDuration(duration)
        {
        }
        
        static MessageType type() { return kProgress; }
        
        Duration mDuration;
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
    
    
    /**
     *  Message subscriber callback type.
     */
    typedef std::function<void(const MessageBase&)> MessageHandler;
    
    
    class MessageBusPrivate;
    
    /**
     *  A MessageBus is a multi-publisher message queue that provides a
     *  lockless message posting interface.
     */
    class MessageBus {
    public:
        
        MessageBus();
        ~MessageBus();
        
        void start();
        void stop();
        
        bool isRunning() const;
        
        void publish(MessageBase *message);
        
        void subscribe(MessageType type, MessageHandler &handler);
        void unsubscribe(MessageType type, MessageHandler &handler);
        
    private:
        AYANE_DISALLOW_COPY_AND_ASSIGN(MessageBus);
        
        MessageBusPrivate *d_ptr;
        AYANE_DECLARE_PRIVATE(MessageBus);
    };
    
    
}

#endif