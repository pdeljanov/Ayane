#pragma once

/*  Ayane
 *
 *  Created by Philip Deljanov on 12-01-06.
 *  Copyright 2012 Philip Deljanov. All rights reserved.
 *
 */

/** @file asynchronousadaptor.h
  * \brief Implements asynchronous processing on the source side. The source interface is thread-safe.
**/

#include "abstractaudiodsp.h"

namespace Ayane
{
  class AsynchronousAdaptorPrivate;
  
  /** Implements asynchronous processing on the source side of the DSP.
    *
    * The asynchronous adapter is DSP wrapper that causes DSPs on the source side of the 
    * interface to be processed on a new thread.  This allows DSPs to run concurrently with
    * each other with no extra effort required by the programmer.
    *
    * \warning The public interface of this class is *NOT* thread-safe.
    * 
    * \warning Accessing DSPs while the processing thread is running is discouraged unless a DSP is programmed
    *          to be thread-safe. Most Ayane DSPs are (and should be) thread-unsafe. Therefore, to safely access 
    *          a DSP while the processing thread is running, simply call the lock() function on the adapter closest
    *          to the right of that DSP. When done accessing the DSP, simply call unlock() on the same adapter that 
    *          lock() was called on.
    * 
    * \warning Interally, a set of buffers is used to couple the two interfaces. This will lead to added
    *          latency in the audio pipleine. To reduce this, consider choosing a shorter buffer length, or
    *          reducing the number of internal buffers.
  **/
  class AsynchronousAdapter : public AbstractAudioDSP
  {

    public:

      // AsynchronousAdapter interface
      
      AsynchronousAdapter();
      virtual ~AsynchronousAdapter();

      /** Locks the AsynchronousAdapter.
        *
        * Since the AsynchronousAdapter processes audio on a separate thread, it must be
        * locked (halting the processing thread) before DSPs on the source may be changed. Locking
        * will not flush the internal buffers so nextBuffer will continue to provide
        * audio data which was processed prior to the chain being locked.
        *
        * Locks should be held for as short a time as possible so that playback won't be interrupted.
        * The maximum time a lock can be held for before an interruption can be found by calling latency()
        * 
        * \note This function blocks until the processing thread becomes locked.
        * \warning If this function returns false, it means the processing thread has exited.
        * 
        * \return True if the adaptor could be locked. False otherwise.
      **/
      bool lock();

      /** Unlocks the AsynchronousAdapter.
        *
        * Processing will resume right after the chain is unlocked.
	* 
	* \note This function blocks until the processing thread becomes unlocked.
      **/
      void unlock();

      /** Calculates the amount of latency the AsynchronousAdapter causes.
        *
        * \return The latency in milliseconds.
      **/
      const Duration &latency() const;

      /** Starts the processing thread. 
        *
	* Until this function is called, no buffers will be processed from the source.
      **/
      void startProcessing();
      
      /** Stops the processing thread. **/
      void stopProcessing();
      
      // Sink interface 
      
      virtual const SupportedAudioBufferFormats& supportedFormats() const;
      
      virtual bool attachSource ( const boost::shared_ptr<AbstractAudioSource> &source );

      // Source interface
      
      virtual Resolve resolve();
      
      virtual const AudioBufferFormat& format() const;

      virtual Result start ( const AudioBufferLength &length );

      virtual Next next();

      virtual Result stop();
      
    private:

      AsynchronousAdaptorPrivate *d;
      
  };
}
