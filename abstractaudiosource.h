#pragma once

/*  Ayane
 *
 *  Created by Philip Deljanov on 12-01-06.
 *  Copyright 2012 Philip Deljanov. All rights reserved.
 *
 */

/** @file abstractaudiosource.h
  * \brief Base interface for a source-sink architecture audio source.
**/

#include "audiobuffer.h"
#include "result.h"

namespace Ayane
{

  /** Base interface for a source-sink architecture audio source. **/
  class AbstractAudioSource
  {

    public:

      /** Encapsulates the result of a resolve call. **/
      typedef struct
      {
	
        /** True if the source could resolve its format properly. False otherwise. **/
        bool isResolved;

        /** Contains the input format.  In the case where a source could not resolve,
          * this format can be used to determine what additional format transformations are
          * required. Otherwise it provides useful logging information. **/
        AudioBufferFormat inputFormat;
	
      } Resolve;

      /** Encapsulates the result of a next call. **/
      typedef struct
      {
	
        /** Contains the status of the next call. **/
        enum Flag
        {
          /** Nothing out of the ordinary occured. **/
          Ok,
          /** A buffer could not be returned. 'buffer' is NULL. **/
          Underrun,
          /** The returned buffer is the last one in the stream. Subsequent calls
            * will return a NULL buffer with an Underrun flag.
          **/
          EndOfStream,
          /** The source is in an invalid processing state. 'buffer' is NULL. **/
          InvalidState
        };

        /** Contains the pointer to the returned buffer.  NULL if there is an underrun or invalid state condition. **/
        AudioBuffer *buffer;

        /** Status flag for the call **/
        Flag flag;
	
      } Next;
      
    public:
     
      //virtual ~AbstractAudioSource();

      /** Resolves the internal format of the source.
        *
	* This function will determine the format the buffer of the source will produce.
	*
	* \throws InvalidStateException If called when the source is in an invalid state.
	*
	* \returns True if the source could resolve a format. False otherwise.  Additionally,
	*          the input format to the source will be returned.
      **/
      virtual Resolve resolve() = 0;

      /** Prepares the source to produce buffers.
        *
        * This function must be called before next.  This function should be overriden to
        * perpare your source to initialize any structures required in the next function.
        *
        * \warning Call the base class to ensure a proper state transition.
        *
        * \param length The length of the buffer that the source should produce.
	* 
        * \throws InvalidStateException If called when the source is in an invalid state.
	* 
	* \returns An error code result.
      **/
      virtual Result start ( const AudioBufferLength &length ) = 0;

      /** Processes the next audio buffer in the stream and then returns it.
        *
        * \note The buffer returned is only valid till the next call to nextBuffer.
	* 
        * \returns The buffer and status flags.
      **/
      virtual Next next() = 0;

      /** Shutsdown the source so that it can't produce buffers anymore.
        *
        * Every call to start should be matched with a call to stop at some
        * point or else there is a risk of a memory leak.  This function should be
	* overriden to destroy any resources created in state.
        *
	* \warning Call the base class to ensure a proper state transition.
	* 
        * \throws InvalidStateException If called when the source is in an invalid state.
	* 
        * \returns An error code result.
      **/
      virtual Result stop() = 0;

      /** Gets the format of the buffers received from next.
        * 
        * \warning This function can only be called when in the 'Resolved' or later state.
	* 
        * \throws InvalidStateException If called when the source is in an invalid state.
	* 
      **/
      virtual const AudioBufferFormat &format() const = 0;

  };
}



