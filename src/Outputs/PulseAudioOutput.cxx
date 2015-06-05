/*
 *
 * Copyright (c) 2015 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "Ayane/Outputs/PulseAudioOutput.h"

#include <vector>
#include <memory>

#include "Ayane/MessageBus.h"
#include "Ayane/Trace.h"
#include "Ayane/RawBuffer.h"

namespace Ayane {

  class PulseAudioOutputPrivate {
  public:

    /// Channel layout for the AU graph.
    std::unique_ptr<AudioChannelLayout>	mAUChannelLayout;

    /// Number of audio frames per buffer for CoreAudio
    UInt32 mMaxFramesPerSlice;

    /// The time in nanoseconds from the last clock tick
    UInt64 mLastClockTickHostTime;

    /// The clock provider
    ClockProvider mClockProvider;

    /// Pulled buffers queue
    BufferQueue mBuffers;

    /// RawBuffer wrapper to wrap CoreAudio's AudioBufferList structure
    std::unique_ptr<RawBuffer> mAudioBufferListWrapper;

    /// The current buffer being used
    ManagedBuffer mCurrentBuffer;
  };
  
}

using namespace Ayane;

PulseAudioOutput::PulseAudioOutput() : Stage(), d_ptr(new PulseAudioOutputPrivate) {
  // Add output sink to stage.
  addSink("input");
}

PulseAudioOutput::~PulseAudioOutput() {
  delete d_ptr;
}

bool PulseAudioOutput::beginPlayback() {
}

bool PulseAudioOutput::stoppedPlayback() {
  
}

void PulseAudioOutput::process(ProcessIOFlags *ioFlags) {
  
}

bool PulseAudioOutput::reconfigureIO() {
  
}

bool PulseAudioOutput::reconfigureInputFormat(const Sink &sink, const BufferFormat &format ) {

}