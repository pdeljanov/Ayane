#pragma once

/*  Ayane
 *
 *  Created by Philip Deljanov on 12-01-06.
 *  Copyright 2012 Philip Deljanov. All rights reserved.
 *
 */

/** @file abstractaudiosink.h
  * \brief Base interface for a source-sink architecture audio sink.
**/

#include <boost/shared_ptr.hpp>
#include "supportedaudiobufferformats.h"
#include "abstractaudiosource.h"

namespace Ayane
{

  /** Base interface for a source-sink architecture audio sink. **/
  class AbstractAudioSink
  {

    public:

      /** Gets a list of the sinks supported audio buffer formats.
        *
	* This function gets a list of support audio buffer formats. It may be called once the 
	* sink is initialized.
	* 
	* \throws InvalidStateException If the sink is not initialized.
	* 
	* \returns A list of AudioBufferFormats.
      **/
      virtual const SupportedAudioBufferFormats& supportedFormats() const = 0;
      
      /** Attaches a source to the sink.
        * 
	* This function attaches a source to the sink. The sink will pull audio buffers from this source.
	* 
	* \throws InvalidStateException If the sink is not ready, or in a busy state where the source may not be
	*                               switched.
	* 
	* \returns True if the source could be attached. False otherwise.
      **/
      virtual bool attachSource ( const boost::shared_ptr<AbstractAudioSource> &source ) = 0;

  };
}
