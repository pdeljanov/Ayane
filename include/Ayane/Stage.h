/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_STAGE_H_
#define AYANE_STAGE_H_

// No one knows the names of the trampled flowers.
// Fallen birds await the next wind before they try again.

#include "Ayane/BufferPool.h"
#include "Ayane/BufferQueue.h"
#include "Ayane/ClockProvider.h"
#include "Ayane/MessageBus.h"

#include <memory>
#include <mutex>
#include <thread>
#include <string>
#include <unordered_map>

namespace Ayane {
    
    class StagePrivate;
    
    /**
     *  A Stage is the basic building block in an audio pipeline. A Stage
     *  may produce, consume, or transform audio data that flows through it.
     */
    class Stage {
    public:
        
        class Source;
        class Sink;
        
        
        /** Collection type for sources. */
        typedef std::unordered_map<std::string, std::unique_ptr<Source>> SourceCollection;
        
        /** Collection type for sinks. */
        typedef std::unordered_map<std::string, std::unique_ptr<Sink>> SinkCollection;
        
        typedef SourceCollection::iterator SourceIterator;
        typedef SourceCollection::const_iterator ConstSourceIterator;
        
        typedef SinkCollection::iterator SinkIterator;
        typedef SinkCollection::const_iterator ConstSinkIterator;
        
        typedef std::pair<SourceIterator, SourceIterator> SourceIteratorPair;
        typedef std::pair<ConstSourceIterator, ConstSourceIterator> ConstSourceIteratorPair;
        
        typedef std::pair<SinkIterator, SinkIterator> SinkIteratorPair;
        typedef std::pair<ConstSinkIterator, ConstSinkIterator> ConstSinkIteratorPair;
        
        
        /**
         *  Enumeration of synchronicity modes for a Stage. Synchronicity modes
         *  control how a stage will be threaded.
         */
        typedef enum
        {
            
            /** Asynchronous operating mode. The Stage pushes buffers to its
             *  source ports in parallel with pull requests.
             */
            kAsynchronous,
            
            /** Synchronous operating mode. The Stage pushes buffers to its
             *  source ports only when a pull request is made.
             */
            kSynchronous
            
        }
        SynchronicityMode;
        
        
        /** Enumeration of Stage operating states. */
        typedef enum
        {
            
            /** The Stage is not initialized or capable of playing. */
            kDeactivated,
            
            /** The Stage is initialized, and capable of playing. */
            kActivated,
            
            /** The Stage is playing. */
            kPlaying
            
        } State;
        
        
        Stage();
        virtual ~Stage();
        
        
        
        /* Port API */
        
        
        
        /**
         *  Attempts to retreive a source port by name.
         */
        bool source( const std::string &name, Source *outSource );
        
        /**
         *  Attempts to retreive a sink by name.
         */
        bool sink( const std::string &name, Sink *outSink );
        
        /**
         *  Gets an iterator begin/end pair for sources.
         */
        ConstSourceIteratorPair sourceIterator(){
            return std::make_pair( mSources.begin(), mSources.end() );
        }
        
        /**
         *  Gets an iterator begin/end pair for sinks.
         */
        ConstSinkIteratorPair sinkIterator(){
            return std::make_pair( mSinks.begin(), mSinks.end() );
        }
        
        /**
         *  Gets the number of sources.
         */
        int sourceCount() const {
            return mSources.size();
        }
        
        /**
         *  Gets the number of sinks.
         */
        int sinkCount() const {
            return mSinks.size();
        }
        
        /**
         *  Unlinks the current source, from the sink, and replaces it with
         *  the next source.
         */
        static bool replace(Source *current, Source *next, Sink *sink);
        
        /**
         *  Links the specified source and sink together.
         */
        static bool link( Source *source, Sink *sink );
        
        /**
         *  Unlinks the specified source from the sink.
         */
        static void unlink( Source *source, Sink *sink );
        
        
        /* Stage (Public) API */
        
        /**
         *  Gets the stage's state. Thread-safe.
         */
        State state() const;
        
        /**
         *  Activates the stage to prepare it for playback. Thread-safe.
         */
        bool activate(MessageBus *messageBus = nullptr);
        
        /**
         *  Deactivates the stage. Thread-safe.
         */
        void deactivate();
        
        /**
         *  Starts playback. Once called, the stage will produce buffers
         *  clocked by the clock provider. Thread-safe.
         */
        void play(ClockProvider &clockProvider);
        
        /**
         *  Stops playback. Thread-safe.
         */
        void stop();
        

    protected:
                
        
        /**
         *  Enumeration of possible input/output flags for process.
         */
        typedef enum {
            
            /**
             *  Output: Hints that the process callback can be called
             *  atleast one more time. This flag is only useful for pure
             *  sink nodes that implement their own buffering scheme and
             *  would like an extra process run to occur to fill their
             *  internal buffer. This hint will be ignored if the stage is
             *  not run asynchronously, or sources are present on the stage.
             */
            kProcessMoreHint = 1<<0
            
        } ProcessIOFlag;
        
        /**
         *  Enumeration of possible results from pull operations.
         */
        typedef enum
        {
            /** A buffer was pulled successfully. */
            kSuccess = 0,
            
            /** The pull was cancelled. */
            kCancelled,
            
            /**
             *  The buffer received is in an unsupported format or the
             *  sink reconfiguration callback failed.
             */
            kUnsupportedFormat,
            
            /** The source has no queued buffers. */
            kBufferQueueEmpty,
            
            /**
             *  The requested operation is only valid on an asynchronous
             *  source.
             */
            kNotAsynchronous
            
        } PullResult;
        
        /**
         *  Process input/output flags set. Bits in this type can be tested
         *  against the flags in ProcessIOFlag.
         */
        typedef uint32_t ProcessIOFlags;

        
        
        /* Port API */

        
        
        /**
         *  Adds a sources with the given name to the source list.
         */
        void addSource( const std::string &name );
        
        /**
         *  Removes a source with the given name from the source list.
         */
        void removeSource( const std::string &name );
        
        /**
         *  Adds a sink with the given name to the sink list.
         */
        void addSink( const std::string &name );
        
        /**
         *  Removes a sink with the given name from the sink list.
         */
        void removeSink( const std::string &name );
        
        /**
         *  Requests a buffer from the linked source. This function will
         *  wait until the source services the request. The returned buffer
         *  may be a different format between successive calls, but the sink
         *  will issue a port-specific reconfigureSink() event.
         */
        PullResult pull(Sink *sink, ManagedBuffer *outBuffer);
        
        /**
         *  Attempts to pull a buffer from the linked source. This
         *  function will never block, but it may not always return a
         *  buffer.
         */
        PullResult tryPull(Sink *sink, ManagedBuffer *outBuffer);
        
        /**
         *  Cancels any waiting pulls.
         */
        void cancelPull(Sink *sink);
        
        /**
         *  Attempts to push buffer to the source. If the source buffer
         *  queue is full, this function will drop the buffer.
         */
        void push(Source *source, ManagedBuffer &buffer );
        
        /**
         *  Resets a source port.
         */
        void resetPort(Source *source);
        
        /**
         *  Resets a sink port.
         */
        void resetPort(Sink *sink);
        
        
        
        /* Stage (Private) API */
        
        
        
        /**
         *  Gets the stage clock.
         */
        const Clock *clock() const;
        
        /**
         *  Gets the parent message bus on which the Stage can post a
         *  message.
         */
        MessageBus *messageBus() const;
        
        /**
         *  Called by the Stage when the next set of buffers should be
         *  pushed to the Stage's sources.
         */
        virtual void process(ProcessIOFlags *ioFlags) = 0;
        
        /**
         *  Called by the Stage when source or sink port availability
         *  changes. Source or sink ports may be linked or unlinked between
         *  reconfigureIO calls, and therefore this is the correct function
         *  to probe the number of input and ouputs.
         *
         *  This function will only be called after last process() event
         *  finishes execution, and blocks any further processing events
         *  till completion. As with all Stage callbacks, no synchronization
         *  is required.
         */
        virtual bool reconfigureIO() = 0;
        
        /**
         *  Called by the Stage when the buffer format of sink changes.
         *
         *  This function is always called on the processing thread after
         *  a pull call is made on a sink, but before the buffer is returned
         *  to the caller.
         *
         */
        virtual bool reconfigureInputFormat(const Sink &sink,
                                            const BufferFormat &format) = 0;
        
        /**
         *  Called by the Stage when transitioning from Activated to
         *  Playing.
         *
         *  The state of the stage between invocations of beginPlayback is
         *  not guaranteed to be the same. Therefore, beginPlayback is the
         *  ideal callback to probe the stage to determine any necessary
         *  processing parameters.
         *
         *  As with all Stage callbacks, no synchronization
         *  is required.
         */
        virtual bool beginPlayback() = 0;
        
        /**
         *  Called by the Stage when transitioning from either Paused
         *  or Playing to Stopped.
         *
         *  As with all Stage callbacks, no synchronization
         *  is required.
         */
        virtual bool stoppedPlayback() = 0;

        /**
         *  Map of sources. Keyed by name.
         */
        SourceCollection mSources;
        
        /**
         *  Map of sinks. Keyed by name.
         */
        SinkCollection mSinks;
        
    private:
        class SourceSinkPrivate;

        AYANE_DISALLOW_COPY_AND_ASSIGN(Stage);
        
        StagePrivate *d_ptr;
        AYANE_DECLARE_PRIVATE(Stage);
    };
    
    /**
     *  A source provides the producing side of a one-to-one
     *  connection between stages. When linked to a sink, a source
     *  provides an interface for buffers to be pushed from the source
     *  stage, and pulled from the sink stage.
     */
    class Stage::Source {
        
        friend class Stage;
        friend class StagePrivate;
        
    public:
        
        /**
         *  Source destructor. Clears all pending buffers and cancels
         *  any waiting pulls.
         */
        ~Source();
        
        /**
         *  Returns true if the source is linked to a sink, false otherwise.
         */
        bool isLinked() const;
        
        /**
         *  Checks if the linked sink supports the specified buffer format.
         */
        bool checkFormatSupport( const BufferFormat &format ) const;
        
        /**
         *  Gets the synchonicity mode of the linked source and sink.
         *  Only valid after the stage is activated.
         */
        SynchronicityMode linkSynchronicity() const;
        
    private:
        AYANE_DISALLOW_COPY_AND_ASSIGN(Source);
        
        Source(StagePrivate *stage);
        
        StagePrivate *mStage;
        Sink  *mLinkedSink;
        
        std::unique_ptr<SourceSinkPrivate> mShared;
    };
    
    
    /**
     *  A sink provides the consuming side of a one-to-one connection
     *  between stages.  When linked to a source, a sink provides the
     *  interface to pull buffers from the linked source's stage.
     */
    class Stage::Sink {
        
        friend class Stage;
        friend class StagePrivate;
        
    public:
        
        /**
         *  Enumeration of scheduling modes for a Sink Port. Scheduling
         *  modes help determine the synchronicity mode the Stage should run
         *  with.
         */
        typedef enum
        {
            /**
             *  Default scheduling. No hints to the Stage scheduler.
             *  This mode is guaranteed to be safe for all valid
             *  pipeline use cases.
             */
            kDefault,
            /**
             *  Forces the upstream port's stage to operate in
             *  asynchronous mode. This mode can be helpful when
             *  interfacing to outputs running on their own thread
             *  (such as OS audio devices). This mode is safe, but may
             *  not be as performant since it will create excess
             *  processing threads. Therefore, only use when needed.
             */
            kForceAsynchronous
            
        }
        SchedulingMode;
        
        /**
         *  Sink destructor.
         */
        ~Sink();
        
        /**
         *  Returns true if the sink is linked to a source, false otherwise.
         */
        bool isLinked() const;
        
        /**
         *  Tests if the buffer format is compatible with the sink.
         */
        bool checkFormatSupport(const BufferFormat &format) const;
        
        /**
         *  Gets the scheduling mode.
         */
        SchedulingMode scheduling() const {
            return m_scheduling;
        }
        
        /**
         *  Sets the scheduling mode. Only valid before a port is linked.
         */
        void setScheduling(SchedulingMode mode) {
            m_scheduling = mode;
        }
        
        /**
         *  Gets the synchonicity mode of the linked source and sink.
         *  Only valid after the stage is activated.
         */
        SynchronicityMode linkSynchronicity() const;
        
        /**
         *  Gets the buffer format the sink is currently configured for.
         *  Note that the format may not be valid, test with
         *  BufferFormat::isValid.
         */
        const BufferFormat &configuredBufferFormat() const {
            return mBufferFormat;
        }
        
    private:
        AYANE_DISALLOW_COPY_AND_ASSIGN(Sink);
        
        Sink(StagePrivate *stage);
        
        StagePrivate  *mStage;
        Source *mLinkedSource;
        
        SchedulingMode m_scheduling;
        
        SourceSinkPrivate *mShared;
        
        BufferFormat mBufferFormat;
        
        volatile bool mPullCancelled;
    };
    
}

#endif
