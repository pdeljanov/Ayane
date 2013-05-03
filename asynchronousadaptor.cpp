/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-07.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */

#include "asynchronousadaptor.h"
#include "locklessrecyclingqueue.h"
#include "audiobuffer.h"
#include "audiobufferformat.h"
#include "atomicoperations.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/barrier.hpp>

#include "invalidstateexception.h"
#include "nosourceexception.h"

#include "logger.h"

#define ever ;;

namespace Ayane
{

    class AsynchronousAdaptorPrivate
    {
        public:

            /** The default number of output buffers. **/
            static const unsigned int NumberOfBuffers = 4;

            /** Ayane state model states **/
            enum InternalState
            {
                /** Source is not initialized. **/
                Uninitialized = 0,
                /** Source is initialized. **/
                Initialized,
                /** Source has resolved its format. **/
                Resolved,
                /** Source is running. **/
                Running
            };

            /** States the processing thread could be in. **/
            enum ProcessingState
            {
                /** The processing thread is not running (no thread exists). **/
                Stopped = 0,
                /** The processing thread is running its normal processing routine. **/
                Processing,
                /** The processing thread is locked (aka paused). **/
                Locked
            };

            class TemporaryAudioBuffer
            {

                public:

                    AudioBuffer *buffer;
                    AsynchronousAdapter::Next::Flag flag;

                public:

                    TemporaryAudioBuffer ( AudioBuffer *buffer ) :
                        buffer ( buffer ),
                        flag ( AsynchronousAdapter::Next::Ok )
                    {
                    }

                    ~TemporaryAudioBuffer()
                    {
                        delete buffer;
                    }

            };

            LocklessRecyclingQueue<TemporaryAudioBuffer, NumberOfBuffers> buffers;
            TemporaryAudioBuffer *activeBuffer;
            TemporaryAudioBuffer *processingBuffer;

            SupportedAudioBufferFormats supportedFormats;

            boost::shared_ptr<AbstractAudioSource> source;

            boost::thread *processThread;
            boost::condition_variable processCond;
            boost::barrier processBarrier;

            /** Standard Ayane state model state. Only read and write this from the user thread. **/
            InternalState state;

            /** Processing thread state. State values should be from ProcessingState enumeration. Only write from the user thread using
              * a tryCompareExchange.  Reads from either thread is safe.
            **/
            volatile unsigned int processingState;

            AudioBufferFormat format;

            Duration latency;

            AsynchronousAdaptorPrivate() :
                activeBuffer ( NULL ),
                processingBuffer ( NULL ),
                supportedFormats ( true ),
                processThread ( NULL ),
                processBarrier ( 2 ),
                state ( Initialized ),
                processingState ( Stopped ),
                latency ( 0.0f )
            {
            }

            void processBuffers ( TemporaryAudioBuffer **destBuffer )
            {
                boost::mutex mutex;
                AsynchronousAdapter::Next next;

                next.flag = AsynchronousAdapter::Next::Ok;

                for ( ever )
                {
                    // If the thread is to be terminated, it may be done here
                    boost::this_thread::interruption_point();

                    // Wait till we are notified of a possible new buffer.
                    while ( *destBuffer || !buffers.obtain ( destBuffer ) )
                    {
                        // If there is a valid buffer, but the flag is okay, don't wait.
                        if ( ( next.flag == AsynchronousAdapter::Next::Ok ) && *destBuffer )
                            break;

                        // Wait until an item is released (when nextBuffer is called) or till interrupted
                        boost::unique_lock<boost::mutex> lock ( mutex );
                        processCond.wait ( lock );
                    }

                    // IMPORTANT: There cannot be any interruption points past here.

                    // Acquire a new buffer
                    next = source->next();

                    // Send the buffer if it's ok, or an EOS buffer
                    if ( ( next.flag == AsynchronousAdapter::Next::Ok ) ||
                            ( next.flag == AsynchronousAdapter::Next::EndOfStream ) )
                    {
                        // Copy the source buffer into the destination buffer
                        ( *destBuffer )->buffer->copy ( *next.buffer );

                        ( *destBuffer )->flag = next.flag;

                        // Queue the destination buffer
                        buffers.enqueue ( *destBuffer );

                        *destBuffer = NULL;
                    }

                    // Loop again!
                }
            }

            bool handleInterruption()
            {
                // This thread must synchronize with the user thread. Wait on a barrier here.
                processBarrier.wait();

                // At this point, the user thread and procressing thread have synchronized. A read from the processingState variable should be safe now.
                volatile unsigned int pState = processingState;

                switch ( pState )
                {
                    case Locked:
                        {
                            LOG_INFO ( Logger::logger() ) << "Processing thead (id=" << boost::this_thread::get_id() << ") entering locked state." << std::endl;
                            processBarrier.wait();

                            LOG_INFO ( Logger::logger() ) << "Processing thead (id=" << boost::this_thread::get_id() << ") exiting locked state." << std::endl;
                            return false;
                        }
                    case Stopped:
                        {
                            LOG_INFO ( Logger::logger() ) << "Stopped processing thread (id=" << boost::this_thread::get_id() << "). Reason=\'Aborted\'" << std::endl;
                            return true;
                        }
                    default:
                        {
                            // This is a terrible place to be and should never happen. Shutdown the procressing thread before something awful happens.
                            LOG_WARN ( Logger::logger() ) << "Invalid procressing state set. Something went *horribly* wrong. Report this!" << std::endl;
                            return true;
                        }
                }

            }

            void processingThread()
            {

                for ( ever )
                {
                    try
                    {
                        processBuffers ( &processingBuffer );
                    }
                    catch ( boost::thread_interrupted &e )
                    {
                        // IMPORTANT: Disable interruptions so we don't have interrupt-ception. We *DO NOT* need to go deeper!
                        boost::this_thread::disable_interruption di;
                        if ( handleInterruption() ) return;
                    }
                }

            }

    };

    AsynchronousAdapter::AsynchronousAdapter() : d ( new AsynchronousAdaptorPrivate )
    {
    }

    AsynchronousAdapter::~AsynchronousAdapter()
    {
        delete d;
    }

    const Duration &AsynchronousAdapter::latency() const
    {
        if ( d->state == AsynchronousAdaptorPrivate::Running )
        {
            return d->latency;
        }
        else
        {
            throw InvalidStateException();
        }
    }

    AbstractAudioSource::Resolve AsynchronousAdapter::resolve()
    {
        if ( d->state == AsynchronousAdaptorPrivate::Initialized )
        {
            // Error if: there is no source.
            if ( d->source == NULL )
                throw NoSourceException();

            Resolve res = d->source->resolve();

            if ( res.isResolved )
            {
                Resolve ret;
                ret.isResolved = true;
                ret.inputFormat = d->source->format();

                LOG_INFO ( Logger::logger() ) << "Resolved output format." << std::endl;

                d->state = AsynchronousAdaptorPrivate::Resolved;

                return ret;
            }
            else
            {
                return res;
            }

        }
        else
        {
            throw InvalidStateException();
        }
    }

    Result AsynchronousAdapter::start ( const AudioBufferLength &length )
    {
        if ( d->state == AsynchronousAdaptorPrivate::Resolved )
        {

            // Setup the source
            Result ret = d->source->start ( length );

            if ( ret.isError() )
                return ret;

            // Get the source's format
            d->format = d->source->format();

            // Initialize the output buffers
            for ( unsigned int i = 0; i < AsynchronousAdaptorPrivate::NumberOfBuffers; ++i )
            {
                AudioBuffer *audioBuffer = new AudioBuffer ( d->format, length );

                AsynchronousAdaptorPrivate::TemporaryAudioBuffer *tempBuffer =
                    new AsynchronousAdaptorPrivate::TemporaryAudioBuffer ( audioBuffer );

                d->buffers.release ( tempBuffer );
            }

            // Calculate the latency
            d->latency = Duration ( length.duration ( d->format.sampleRate() ) * AsynchronousAdaptorPrivate::NumberOfBuffers );

            d->state = AsynchronousAdaptorPrivate::Running;

            return Result();

        }
        else
        {
            throw InvalidStateException();
        }
    }

    Result AsynchronousAdapter::stop()
    {
        if ( ( d->state == AsynchronousAdaptorPrivate::Running ) &&
                ( d->processingState == AsynchronousAdaptorPrivate::Stopped )
           )
        {
            // Shutdown the source
            Result ret = d->source->stop();

            if ( ret.isError() )
                return ret;

            if ( d->activeBuffer != NULL )
            {
                delete d->activeBuffer;
                d->activeBuffer = NULL;
            }

            if ( d->processingBuffer != NULL )
            {
                delete d->processingBuffer;
                d->processingBuffer = NULL;
            }

            d->buffers.freeAll();

            d->latency = 0.0f;

            d->format = AudioBufferFormat();

            d->state = AsynchronousAdaptorPrivate::Resolved;

            return Result();

        }
        else
        {
            throw InvalidStateException();
        }
    }

    void AsynchronousAdapter::startProcessing()
    {
        if ( ( d->state == AsynchronousAdaptorPrivate::Running ) &&
                ( d->processingState == AsynchronousAdaptorPrivate::Stopped )
           )
        {

            // Set state to processing.
            d->processingState = AsynchronousAdaptorPrivate::Processing;

            // Spawn the thread!
            d->processThread = new boost::thread ( &AsynchronousAdaptorPrivate::processingThread, d );
            LOG_INFO ( Logger::logger() ) << "New processing thread (id=" << d->processThread->get_id() << ") started." << std::endl;

        }
        else
        {
            throw InvalidStateException();
        }
    }

    void AsynchronousAdapter::stopProcessing ()
    {
        if ( ( d->state == AsynchronousAdaptorPrivate::Running ) &&
                ( d->processingState == AsynchronousAdaptorPrivate::Processing )
           )
        {

            // The processing thread may have killed itself. Test with a timed_join.
            // TODO: Find a better way to test for a self-killed processing thread.
            bool selfKilled = d->processThread->timed_join ( boost::posix_time::microseconds ( 0 ) );

            // If the thread was still running, it'll need to be interrupted, and then terminated properly
            if ( !selfKilled && d->processThread->joinable() )
            {
                // Interrupt and put processing thread into it's waiting state
                d->processThread->interrupt();

                // Set the new processing thread state
                AtomicOperations::tryCompareExchange ( &d->processingState, AsynchronousAdaptorPrivate::Processing, AsynchronousAdaptorPrivate::Stopped );

                // Signal and synchronize with the procressing thread
                d->processBarrier.wait();

                // Clean up thread
                d->processThread->join();
            }

            delete d->processThread;
            d->processThread = NULL;

            // Reset the thread state, no need for atomicity, the thread is dead
            d->processingState = AsynchronousAdaptorPrivate::Stopped;

        }
        else
        {
            throw InvalidStateException();
        }

    }

    bool AsynchronousAdapter::lock()
    {
        if ( ( d->state == AsynchronousAdaptorPrivate::Running ) &&
                ( d->processingState == AsynchronousAdaptorPrivate::Processing )
           )
        {

            // The processing thread may have killed itself. Test with a timed_join.
            // TODO: Find a better way to test for a self-killed processing thread.
            bool selfKilled = d->processThread->timed_join ( boost::posix_time::microseconds ( 0 ) );

            if ( !selfKilled )
            {

                // Interrupt thread and put it in it's waiting state
                d->processThread->interrupt();

                // Set the new processing thread state
                AtomicOperations::tryCompareExchange ( &d->processingState, AsynchronousAdaptorPrivate::Processing, AsynchronousAdaptorPrivate::Locked );

                // Signal and synchronize with the processing thread
                d->processBarrier.wait();

                return true;
            }
            else
            {
                LOG_TRACE ( Logger::logger() ) << "Processing thread finished processing. Locking is not possible until processing has been restarted." << std::endl;
                return false;
            }

        }
        else
        {
            throw InvalidStateException();
        }
    }

    void AsynchronousAdapter::unlock()
    {
        if ( ( d->state == AsynchronousAdaptorPrivate::Running ) &&
                ( d->processingState == AsynchronousAdaptorPrivate::Locked )
           )
        {

            // Signal the processing thread to exit it block
            d->processBarrier.wait();

            // Set the new processing thread state
            AtomicOperations::tryCompareExchange ( &d->processingState, AsynchronousAdaptorPrivate::Locked, AsynchronousAdaptorPrivate::Processing );

        }
        else
        {
            throw InvalidStateException();
        }

    }

    const SupportedAudioBufferFormats& AsynchronousAdapter::supportedFormats() const
    {
        return d->supportedFormats;
    }

    bool AsynchronousAdapter::attachSource ( const boost::shared_ptr<AbstractAudioSource> &source )
    {
        if ( d->state == AsynchronousAdaptorPrivate::Initialized )
        {

            if ( source.get() == NULL )
                return false;

            d->source = source;
            return true;

        }
        else
        {
            throw InvalidStateException();
        }
    }

    const AudioBufferFormat &AsynchronousAdapter::format() const
    {
        return d->format;
    }

    AbstractAudioSource::Next AsynchronousAdapter::next ()
    {
        Next ret;
        ret.buffer = NULL;
        

        if ( d->state == AsynchronousAdaptorPrivate::Running )
        {
            // If the last buffer was not NULL, release it
            if ( d->activeBuffer != NULL )
            {
                d->buffers.release ( d->activeBuffer );
                d->activeBuffer = NULL;
                d->processCond.notify_one();
            }

            // Attempt to dequeue a buffer from the queue and save it as the active buffer
            bool status = d->buffers.dequeue ( &d->activeBuffer );

            // If a buffer was dequeued, return it
            if ( status )
            {
                // TODO: pass along given flag
                ret.flag = d->activeBuffer->flag;
                ret.buffer = d->activeBuffer->buffer;
            }
            else
            {
                ret.flag = Next::Underrun;
            }

        }
        else
        {
            ret.flag = Next::InvalidState;
        }

        return ret;
    }


}



