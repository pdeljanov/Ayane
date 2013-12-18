/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "Ayane/MessageBus.h"
#include "Ayane/Trace.h"

#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>

using namespace Ayane;

namespace Ayane {
                
    class MessageBusPrivate {
    public:
        MessageBusPrivate();
        ~MessageBusPrivate();
        
        
        void post(MessageBase *message);
        MessageBase *flush();
        void clear();
        
        void dispatchThread();

        
        std::thread mDispatchThread;
        std::mutex mDispatchMutex;
        std::condition_variable mDispatchNotification;
        
        std::map<MessageType, MessageHandler> mSubscribers;
        
        MessageBase *mQueueHead;
        
        std::atomic_bool mStopping;
    };
    
}


MessageBusPrivate::MessageBusPrivate() :
    mQueueHead(new MessageBase(kNil)), mStopping(false)
{
    
}

MessageBusPrivate::~MessageBusPrivate(){
    
    if( mQueueHead ) {
        delete mQueueHead;
    }
    
    clear();
}

void MessageBusPrivate::post(MessageBase *message){
    
    MessageBase *expectedHeadNext = nullptr;
    
    do {
        expectedHeadNext = mQueueHead->mNext.load(std::memory_order_relaxed);
        message->mNext = expectedHeadNext;
    }
    while(!std::atomic_compare_exchange_weak(&mQueueHead->mNext,
                                             &expectedHeadNext,
                                             message));
    
    // Send notification to consumer thread.
    mDispatchNotification.notify_one();
}

MessageBase *MessageBusPrivate::flush() {
    
    MessageBase *expectedHeadNext = nullptr;
    
    do {
        expectedHeadNext = mQueueHead->mNext.load(std::memory_order_relaxed);
    } while(!std::atomic_compare_exchange_weak(&mQueueHead->mNext,
                                                &expectedHeadNext,
                                                (MessageBase*)nullptr));
    
    return expectedHeadNext;
}

void MessageBusPrivate::clear() {
    MessageBase *currentQueueItem = flush();
    
    while( currentQueueItem != nullptr ){
        MessageBase *item = currentQueueItem;
        currentQueueItem = item->mNext;
        delete currentQueueItem;
    }
}

void MessageBusPrivate::dispatchThread() {
    INFO_THIS("MessageBusPrivate::dispatchThread") << "Started message bus "
    "dispatch thread " << std::this_thread::get_id() << "." << std::endl;
    
    MessageBase *message = nullptr;
    
    while (!mStopping) {
        
        // Obtain the dispatch lock to protect the subscriber map.
        std::unique_lock<std::mutex> lock(mDispatchMutex);
        
        while(!mStopping && (message = flush()) == nullptr){
            mDispatchNotification.wait(lock);
        }
        
        // Process the messages.
        while(message != nullptr){
            
            std::map<MessageType, MessageHandler>::const_iterator handler;
            handler = mSubscribers.find(message->mType);
            
            if(handler != mSubscribers.end()) {
                handler->second(*message);
            }
            
            MessageBase *lastMessage = message;
            message = message->mNext;
            delete lastMessage;
        }
        
        message = nullptr;
    }
    
    
    INFO_THIS("MessageBusPrivate::dispatchThread") << "Message bus thread "
    << std::this_thread::get_id() << " exiting." << std::endl;
}



MessageBus::MessageBus() : d_ptr(new MessageBusPrivate){
}

MessageBus::~MessageBus() {
    stop();
    delete d_ptr;
}

void MessageBus::start(){
    A_D(MessageBus);
    
    if(!d->mDispatchThread.joinable()){
        d->mStopping = false;
        d->mDispatchThread = std::thread(&MessageBusPrivate::dispatchThread, d);
    }
}

void MessageBus::stop(){
    A_D(MessageBus);
    
    if(d->mDispatchThread.joinable()){
        d->mStopping = true;
        d->mDispatchNotification.notify_one();
        d->mDispatchThread.join();
        
        // Clear any remaining messages.
        d->clear();
    }
}

bool MessageBus::isRunning() const{
    A_D(const MessageBus);
    return d->mDispatchThread.joinable();
}

void MessageBus::publish(MessageBase *message) {
    A_D(MessageBus);
    d->post(message);
}

void MessageBus::subscribe(MessageType type, MessageHandler &handler){
    A_D(MessageBus);
    
    std::lock_guard<std::mutex> lock(d->mDispatchMutex);
    
    d->mSubscribers.insert(std::make_pair(type, handler));
}

void MessageBus::unsubscribe(MessageType type, MessageHandler &handler){
    A_D(MessageBus);
    
    std::lock_guard<std::mutex> lock(d->mDispatchMutex);
    
    std::map<MessageType,MessageHandler>::iterator handlerIter;
    handlerIter = d->mSubscribers.find(type);
    
    if(handlerIter != d->mSubscribers.end()){
        d->mSubscribers.erase(handlerIter);
    }
}




