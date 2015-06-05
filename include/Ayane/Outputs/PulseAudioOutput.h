/*
 * 
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_OUTPUTS_PULSEAUDIOOUTPUT_H_
#define AYANE_OUTPUTS_PULSEAUDIOOUTPUT_H_

#include "Ayane/DPointer.h"
#include "Ayane/ClockProvider.h"
#include "Ayane/Stage.h"

namespace Ayane {

  class PulseAudioOutputPrivate;

  /**
   *  A PulseAudio output.
   */
  class PulseAudioOutput : public Stage {

  public:

    PulseAudioOutput();
    ~PulseAudioOutput();

    /**
     *  Gets the input sink.
     */
    Sink *input();

    /**
     *  Gets the clock provider.
     */
    ClockProvider &clockProvider();

  protected:
    virtual bool beginPlayback();
    virtual bool stoppedPlayback();
    virtual void process(ProcessIOFlags *ioFlags);
    virtual bool reconfigureIO();
    virtual bool reconfigureInputFormat(const Sink &sink,
					const BufferFormat &format );

  private:
    AYANE_DISALLOW_COPY_AND_ASSIGN(PulseAudioOutput);

    PulseAudioOutputPrivate *d_ptr;
    AYANE_DECLARE_PRIVATE(PulseAudioOutput);
  };

}

#endif