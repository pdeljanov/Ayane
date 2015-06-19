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
#include "Ayane/SampleFormats.h"

#include "pulse/pulseaudio.h"
#include "pulse/thread-mainloop.h"

const std::string kPulseAudioContextName = "Ayane";
const std::string kPulseAudioStreamName = "";

namespace Ayane {

  class PulseAudioOutputPrivate {
  public:

    PulseAudioOutputPrivate();
    ~PulseAudioOutputPrivate();

    bool openOutput();
    void closeOutput();

    bool startOutput(const BufferFormat& format);
    void stopOutput();

    inline void writeOutput(ManagedBuffer buffer){
      mBuffers.push(buffer);
    }

    inline bool outputBufferFull(){
      return mBuffers.full();
    }

    bool setupPulseAudioContext();
    void destroyPulseAudioContext();
    bool ensurePulseAudioContext();

    bool connectToPulseAudioServer();
    void disconnectFromPulseAudioServer();
    bool waitForPulseAudioConnection();
    bool ensurePulseAudioConnection();

    bool createPulseAudioStream(const BufferFormat& format);
    void destroyPulseAudioStream();
    bool waitForPulseAudioStream();

    void handlePulseAudioContextStateChange(pa_context_state_t state);
    void handlePulseAudioSubscriptionChanged(pa_subscription_event_type_t eventType, uint32_t index);

    void handlePulseAudioStreamStateChanged(pa_stream_state state);
    void handlePulseAudioStreamWriteRequest(size_t byteLength);
    void handlePulseAudioStreamSuspended();
    void handlePulseAudioStreamUnderflow();
    void handlePulseAudioStreamOperationSuccess();

    inline void waitForPulseAudioStateChange() {
      pa_threaded_mainloop_wait(mPAMainLoop);
    }

    inline void signalPulseAudioStateChange() {
      pa_threaded_mainloop_signal(mPAMainLoop, 0);
    }

    static bool waitForOperationToComplete(pa_operation* operation, pa_threaded_mainloop* mainloop);

    /// The clock provider
    ClockProvider mClockProvider;

    /// Queue of pulled buffers from the input sink.
    BufferQueue mBuffers;

    /// RawBuffer wrapper to wrap PulseAudio's raw output stream buffer.
    std::unique_ptr<RawBuffer> mBufferWrapper;

    /// The current buffer being written out of Ayane to PulseAudio.
    ManagedBuffer mCurrentBuffer;

  private:

    struct pa_threaded_mainloop *mPAMainLoop;
    struct pa_context *mPAContext;
    struct pa_stream *mPAStream;

  };

  static void pulseAudioOutputPrivate_stateCallback(pa_context* context, void* user){
    PulseAudioOutputPrivate* instance = static_cast<PulseAudioOutputPrivate*>(user);
    instance->handlePulseAudioContextStateChange(pa_context_get_state(context));
  };

  static void pulseAudioOutputPrivate_subscribeCallback(pa_context *context, pa_subscription_event_type_t eventType, uint32_t index, void *user){
    PulseAudioOutputPrivate* instance = static_cast<PulseAudioOutputPrivate*>(user);
    instance->handlePulseAudioSubscriptionChanged(eventType, index);
  };

  static void pulseAudioOutputPrivate_streamStateCallback(pa_stream* stream, void* user){
    PulseAudioOutputPrivate* instance = static_cast<PulseAudioOutputPrivate*>(user);
    instance->handlePulseAudioStreamStateChanged(pa_stream_get_state(stream));
  }

  static void pulseAudioOutputPrivate_streamWriteCallback(pa_stream* stream, size_t byteLength, void* user){
    PulseAudioOutputPrivate* instance = static_cast<PulseAudioOutputPrivate*>(user);
    instance->handlePulseAudioStreamWriteRequest(byteLength);
  }

  static void pulseAudioOutputPrivate_streamSuspendedCallback(pa_stream* stream, void* user){
    PulseAudioOutputPrivate* instance = static_cast<PulseAudioOutputPrivate*>(user);
    instance->handlePulseAudioStreamSuspended();
  }

  static void pulseAudioOutputPrivate_streamUnderflowCallback(pa_stream* stream, void* user){
    PulseAudioOutputPrivate* instance = static_cast<PulseAudioOutputPrivate*>(user);
    instance->handlePulseAudioStreamUnderflow();
  }

  static void pulseAudioOutputPrivate_streamOperationSuccessCallback(pa_stream* stream, int success, void* user){
    PulseAudioOutputPrivate* instance = static_cast<PulseAudioOutputPrivate*>(user);
    instance->handlePulseAudioStreamOperationSuccess();
  }

  bool PulseAudioOutputPrivate::waitForOperationToComplete(pa_operation* operation, pa_threaded_mainloop* mainloop)
  {
    pa_operation_state_t state;

    while (PA_OPERATION_RUNNING == (state = pa_operation_get_state(operation))){
      pa_threaded_mainloop_wait(mainloop);
    }

    pa_operation_unref(operation);

    return (PA_OPERATION_DONE == state);
  }


  PulseAudioOutputPrivate::PulseAudioOutputPrivate() : 
    mClockProvider(ClockCapabilities(0, 1000000000), 100000000),
    mBuffers(2),
    mPAMainLoop(nullptr), 
    mPAContext(nullptr), 
    mPAStream(nullptr) {

  }

  PulseAudioOutputPrivate::~PulseAudioOutputPrivate()
  {
    closeOutput();
  }

  void PulseAudioOutputPrivate::handlePulseAudioContextStateChange(pa_context_state_t state)
  {
    signalPulseAudioStateChange();
  }

  void PulseAudioOutputPrivate::handlePulseAudioSubscriptionChanged(pa_subscription_event_type_t eventType, uint32_t index)
  {

  }

  void PulseAudioOutputPrivate::handlePulseAudioStreamStateChanged(pa_stream_state_t state)
  {
    signalPulseAudioStateChange();
  }

  void PulseAudioOutputPrivate::handlePulseAudioStreamWriteRequest(size_t byteLength){
    void* streamBuffer = nullptr;
    size_t streamBufferLength = byteLength;

    // Get a writable memory location from PulseAudio.
    pa_stream_begin_write(mPAStream, &streamBuffer, &streamBufferLength);

    // Since we are just given a byteLength, get the frame size so we can calculate the number of
    // frames that will fit in the write pointer.
    size_t frameSize = pa_frame_size(pa_stream_get_sample_spec(mPAStream));

    // Update the raw buffer wrapper to point to the new writeable stream buffer.
    mBufferWrapper->mBuffers[0].mBuffer = streamBuffer;
    mBufferWrapper->mFrames = streamBufferLength / frameSize;
    mBufferWrapper->reset();  // Beware of .reset(), this kills the unique_ptr!

    // Copy as many frames as possible from the current audio buffer into the PulseAudio buffer.
    if(mCurrentBuffer && (mCurrentBuffer->available() > 0)) {
      *mCurrentBuffer >> *mBufferWrapper;
    }

    // Attempt to continue filling the PulseAudio buffer.
    while(mBufferWrapper->space() > 0) {

      // Release the current buffer if it hasn't been released.
      if(mCurrentBuffer) {
	mCurrentBuffer.reset();
      }

      // Try to get a new buffer.
      if(mBuffers.pop(&mCurrentBuffer)) {
	(*mCurrentBuffer) >> (*mBufferWrapper);
      }
      else {
	
	// If absolutely nothing was written, cancel the write and exit the request.
	if(mBufferWrapper->mWriteIndex == 0) {
	  pa_stream_cancel_write(mPAStream);
	  return;
	}
      }

    }

    // Write to the stream.
    pa_stream_write(mPAStream, streamBuffer, mBufferWrapper->mWriteIndex * frameSize, nullptr, 0, PA_SEEK_RELATIVE);
  }

  void PulseAudioOutputPrivate::handlePulseAudioStreamSuspended()
  {

  }

  void PulseAudioOutputPrivate::handlePulseAudioStreamUnderflow()
  {
    WARNING_THIS("PulseAudioOutputPrivate::handlePulseAudioStreamUnderflow") << "Underflow." << std::endl;
  }
  
  void PulseAudioOutputPrivate::handlePulseAudioStreamOperationSuccess(){
    signalPulseAudioStateChange();
  }


  
  
  bool PulseAudioOutputPrivate::openOutput()
  {
    // Exit early if the PulseAudio mainloop is already initialized.
    if(mPAMainLoop){
      return true;
    }

    // Instantiate the threaded mainloop.
    mPAMainLoop = pa_threaded_mainloop_new();
    if (nullptr == mPAMainLoop) {
      ERROR_THIS("PulseAudioOutputPrivate::initialize") << "Failed to instantiate PulseAudio mainloop." << std::endl;
      return false;
    }

    // Start he mainloop thread.
    pa_threaded_mainloop_lock(mPAMainLoop);

    if (pa_threaded_mainloop_start(mPAMainLoop) < 0) {

      // Failed to start the mainloop, cleanup.
      pa_threaded_mainloop_unlock(mPAMainLoop);
      pa_threaded_mainloop_free(mPAMainLoop);
      mPAMainLoop = nullptr;

      ERROR_THIS("PulseAudioOutputPrivate::initialize") << "Failed to start PulseAudio mainloop." << std::endl;
      return false;
    }

    // Setup the PulseAudio context.
    if (!setupPulseAudioContext()) {

      pa_threaded_mainloop_unlock(mPAMainLoop);
      pa_threaded_mainloop_stop(mPAMainLoop);
      pa_threaded_mainloop_free(mPAMainLoop);
      mPAMainLoop = nullptr;

      return false;
    }

    pa_threaded_mainloop_unlock(mPAMainLoop);

    return true;
  }
  
  void PulseAudioOutputPrivate::closeOutput()
  {
    // Stop the mainloop.
    pa_threaded_mainloop_stop(mPAMainLoop);

    // Destroy the PulseAudio context.
    destroyPulseAudioContext();

    pa_threaded_mainloop_free(mPAMainLoop);
    mPAMainLoop = nullptr;
  }

  bool PulseAudioOutputPrivate::setupPulseAudioContext()
  {
    if(nullptr != mPAContext){
      return true;
    }

    // TODO: Allow configuration of context name.
    mPAContext = pa_context_new(pa_threaded_mainloop_get_api(mPAMainLoop), kPulseAudioContextName.c_str());

    if (mPAContext == nullptr) {
      ERROR_THIS("PulseAudioOutputPrivate::setupPulseAudioContext") << "Failed to instantiate PulseAudio context." << std::endl;
      return false;
    }

    // Register callbacks.
    pa_context_set_subscribe_callback(mPAContext, pulseAudioOutputPrivate_subscribeCallback, this);
    pa_context_set_state_callback(mPAContext, pulseAudioOutputPrivate_stateCallback, this);

    // Attempt to connect (asynchronously) to the PulseAudio server.
    if(!connectToPulseAudioServer()){
      destroyPulseAudioContext();
      return false;
    }

    return true;
  }

  void PulseAudioOutputPrivate::destroyPulseAudioContext()
  {
    if(nullptr == mPAContext){
      return;
    }

    pa_context_set_state_callback(mPAContext, nullptr, nullptr);
    pa_context_set_subscribe_callback(mPAContext, nullptr, nullptr);

    pa_context_disconnect(mPAContext);
    pa_context_unref(mPAContext);
    mPAContext = nullptr;
  }
  
  bool PulseAudioOutputPrivate::ensurePulseAudioContext()
  {
    // If there is no context don't attempt to create it.
    if(nullptr == mPAContext){
      return false;
    }
    
    // Ensure the context is in a working state.
    switch (pa_context_get_state(mPAContext)) {
      case PA_CONTEXT_UNCONNECTED:
      case PA_CONTEXT_TERMINATED:
      case PA_CONTEXT_FAILED: 
	
	// Restart the context.
	destroyPulseAudioContext();
	if(!setupPulseAudioContext()){
	  return false;
	}
	break;
	
      case PA_CONTEXT_READY:
      case PA_CONTEXT_CONNECTING:
      case PA_CONTEXT_AUTHORIZING:
      case PA_CONTEXT_SETTING_NAME:
	break;
    }
    
    return true;
  }

  

  bool PulseAudioOutputPrivate::connectToPulseAudioServer()
  {
    if(nullptr == mPAContext){
      ERROR_THIS("PulseAudioOutputPrivate::connectToPulseAudioServer") << "No PulseAudio context." << std::endl;
      return false;
    }

    // TODO: Allow connection to non-default server.
    return (pa_context_connect(mPAContext, nullptr, static_cast<pa_context_flags_t>(0), nullptr) < 0);
  }

  void PulseAudioOutputPrivate::disconnectFromPulseAudioServer()
  {
    pa_context_disconnect(mPAContext);
  }

  bool PulseAudioOutputPrivate::waitForPulseAudioConnection()
  {
    // No context, can't do anything.
    if(nullptr == mPAContext){
      return false;
    }

    // Wait for a connection, or bail out on error.
    while (true) {
      switch (pa_context_get_state(mPAContext)) {
	case PA_CONTEXT_UNCONNECTED:
	case PA_CONTEXT_TERMINATED:
	case PA_CONTEXT_FAILED:
	  ERROR_THIS("PulseAudioOutputPrivate::waitForPulseAudioConnection") << "Failed to connect to PulseAudio server." << std::endl;
	  return false;
	case PA_CONTEXT_CONNECTING:
	case PA_CONTEXT_AUTHORIZING:
	case PA_CONTEXT_SETTING_NAME:
	  waitForPulseAudioStateChange();
	  break;
	case PA_CONTEXT_READY:
	  return true;
      }
    }

  }


  bool PulseAudioOutputPrivate::ensurePulseAudioConnection(){

    // Ensure the PulseAudio context is in a good working order.
    if(!ensurePulseAudioContext()){
      return false;
    }

    // Wait for the PulseAudio connection.
    if(!waitForPulseAudioConnection()){
      destroyPulseAudioContext();
      return false;
    }

    // Working connection!
    return true;
  }


  bool PulseAudioOutputPrivate::createPulseAudioStream(const BufferFormat& format){

    // Create a sample spec that will match the input buffer format.
    pa_sample_spec sampleSpec;

    // TODO: Use sample format of the actual buffer. For now, use Float32 as that is Ayane's default.
    sampleSpec.format = PA_SAMPLE_FLOAT32LE;
    sampleSpec.rate = format.sampleRate();
    sampleSpec.channels = format.channelCount();

    // Create a PulseAudio channel map from the channels we have.
    pa_channel_map channelMap;
    pa_channel_map_init(&channelMap);

    channelMap.channels = 0;
    
    // While creating the PulseAudio channel map, fill out the buffer wrapper.
    std::unique_ptr<RawBuffer> wrapper(new RawBuffer(0, sampleSpec.channels, kFloat32, false));

    if(format.channels() & kFrontLeft){
      channelMap.map[channelMap.channels++] = PA_CHANNEL_POSITION_FRONT_LEFT;
      wrapper->mBuffers[channelMap.channels].mChannel = kFrontLeft;
    }
    if(format.channels() & kFrontRight){
      channelMap.map[channelMap.channels++] = PA_CHANNEL_POSITION_FRONT_RIGHT;
      wrapper->mBuffers[channelMap.channels].mChannel = kFrontRight;
    }
    if(format.channels() & kFrontCenter){
      channelMap.map[channelMap.channels++] = PA_CHANNEL_POSITION_FRONT_CENTER;
      wrapper->mBuffers[channelMap.channels].mChannel = kFrontCenter;
    }
    if(format.channels() & kLowFrequencyOne){
      channelMap.map[channelMap.channels++] = PA_CHANNEL_POSITION_LFE;
      wrapper->mBuffers[channelMap.channels].mChannel = kLowFrequencyOne;
    }
    if(format.channels() & kBackLeft){
      channelMap.map[channelMap.channels++] = PA_CHANNEL_POSITION_REAR_LEFT;
      wrapper->mBuffers[channelMap.channels].mChannel = kBackLeft;
    }
    if(format.channels() & kBackRight){
      channelMap.map[channelMap.channels++] = PA_CHANNEL_POSITION_REAR_RIGHT;
      wrapper->mBuffers[channelMap.channels].mChannel = kBackRight;
    }
    if(format.channels() & kFrontLeftOfCenter){
      channelMap.map[channelMap.channels++] = PA_CHANNEL_POSITION_FRONT_LEFT_OF_CENTER;
      wrapper->mBuffers[channelMap.channels].mChannel = kFrontLeftOfCenter;
    }
    if(format.channels() & kFrontRightOfCenter){
      channelMap.map[channelMap.channels++] = PA_CHANNEL_POSITION_FRONT_RIGHT_OF_CENTER;
      wrapper->mBuffers[channelMap.channels].mChannel = kFrontRightOfCenter;
    }
    if(format.channels() & kBackCenter){
      channelMap.map[channelMap.channels++] = PA_CHANNEL_POSITION_REAR_CENTER;
      wrapper->mBuffers[channelMap.channels].mChannel = kBackCenter;
    }
    if(format.channels() & kSideLeft){
      channelMap.map[channelMap.channels++] = PA_CHANNEL_POSITION_SIDE_LEFT;
      wrapper->mBuffers[channelMap.channels].mChannel = kSideLeft;
    }
    if(format.channels() & kSideRight){
      channelMap.map[channelMap.channels++] = PA_CHANNEL_POSITION_SIDE_RIGHT;
      wrapper->mBuffers[channelMap.channels].mChannel = kSideRight;
    }

    mPAStream = pa_stream_new(mPAContext, kPulseAudioStreamName.c_str(), &sampleSpec, &channelMap);

    if (nullptr == mPAStream){
      ERROR_THIS("PulseAudioOutputPrivate::createPulseAudioStream") << "Failed to instantiate PulseAudio stream." << std::endl;
      return false;
    }

    mBufferWrapper = std::move(wrapper);

    // Register stream callbacks.
    pa_stream_set_state_callback(mPAStream, pulseAudioOutputPrivate_streamStateCallback, this);
    pa_stream_set_write_callback(mPAStream, pulseAudioOutputPrivate_streamWriteCallback, this);
    pa_stream_set_suspended_callback(mPAStream, pulseAudioOutputPrivate_streamSuspendedCallback, this);
    pa_stream_set_underflow_callback(mPAStream, pulseAudioOutputPrivate_streamUnderflowCallback, this);

    // Connect the stream to the sink (asynchronously).
    if (pa_stream_connect_playback(mPAStream, nullptr, nullptr, pa_stream_flags_t(0), nullptr, nullptr) < 0) {
      ERROR_THIS("PulseAudioOutputPrivate::createPulseAudioStream") << "Failed to connect PulseAudio stream to sink." << std::endl;
      destroyPulseAudioStream();
      return false;
    }

    return true;
  }

  void PulseAudioOutputPrivate::destroyPulseAudioStream(){

    // Do nothing if there is no stream.
    if(nullptr == mPAStream){
      return;
    }

    // Deregister callbacks.
    pa_stream_set_state_callback(mPAStream, nullptr, this);
    pa_stream_set_write_callback(mPAStream, nullptr, this);
    pa_stream_set_suspended_callback(mPAStream, nullptr, this);
    pa_stream_set_underflow_callback(mPAStream, nullptr, this);

    // Disconnect and destroy the stream.
    pa_stream_disconnect(mPAStream);
    pa_stream_unref(mPAStream);
    mPAStream = nullptr;
  }
  
  bool PulseAudioOutputPrivate::waitForPulseAudioStream()
  {
    // No stream, can't do anything.
    if(nullptr == mPAStream){
      return false;
    }

    while (true) {
      switch (pa_stream_get_state(mPAStream)) {
	case PA_STREAM_FAILED:
	case PA_STREAM_TERMINATED:
	case PA_STREAM_UNCONNECTED:
	  ERROR_THIS("PulseAudioOutputPrivate::waitForPulseAudioStream") << "Failed to connect PulseAudio stream." << std::endl;
	  return false;
	case PA_STREAM_CREATING:
	  waitForPulseAudioStateChange();
	  break;
	case PA_STREAM_READY:
	  return true;
      }
    }
  }


  bool PulseAudioOutputPrivate::startOutput(const BufferFormat& format)
  {
    pa_threaded_mainloop_lock(mPAMainLoop);

    // Ensure a connection to the PulseAudio sound server exists. This will block
    // until a connection or a failure.
    if(!ensurePulseAudioConnection()){
      pa_threaded_mainloop_unlock(mPAMainLoop);
      return false;
    }

    // Create the PulseAudio stream.
    if(!createPulseAudioStream(format)){
      pa_threaded_mainloop_unlock(mPAMainLoop);
      return false;
    }

    // Wait for the stream to be connected.
    if(!waitForPulseAudioStream()){
      pa_threaded_mainloop_unlock(mPAMainLoop);
      return false;
    }

    pa_threaded_mainloop_unlock(mPAMainLoop);

    // Ready to go!
    return true;
  }

  void PulseAudioOutputPrivate::stopOutput()
  {
    pa_threaded_mainloop_lock(mPAMainLoop);

    // Drain the output (synchronously)
    if (PA_STREAM_READY == pa_stream_get_state(mPAStream)) {
      pa_operation* operation = pa_stream_drain(mPAStream, pulseAudioOutputPrivate_streamOperationSuccessCallback, this);
      if (nullptr == operation) {
	WARNING_THIS("PulseAudioOutput::stopOutput") << "Failed to drain PulseAudio stream." << std::endl;
      } 
      else {
	PulseAudioOutputPrivate::waitForOperationToComplete(operation, mPAMainLoop);
      }
    }

    // Destroy the PulseAudio stream.
    destroyPulseAudioStream();

    pa_threaded_mainloop_unlock(mPAMainLoop);
  }



}

using namespace Ayane;

PulseAudioOutput::PulseAudioOutput() : Stage(), d_ptr(new PulseAudioOutputPrivate) {
  // Add output sink to stage.
  addSink("input");
}

PulseAudioOutput::~PulseAudioOutput() {
  delete d_ptr;
}

ClockProvider& PulseAudioOutput::clockProvider()
{
  A_D(PulseAudioOutput);
  return d->mClockProvider;
}

Stage::Sink* PulseAudioOutput::input()
{
  return mSinks["input"].get();
}


bool PulseAudioOutput::beginPlayback() {

  // Open the PulseAudio device.
  A_D(PulseAudioOutput);
  if(!d->openOutput()){
    return false;
  }
  
  // Don't actually start the device till a buffer is received.
  double clockPeriodInSeconds = ((double)d->mClockProvider.clockPeriod() / 1000000000.0);
  d->mClockProvider.publish(clockPeriodInSeconds);

  return true;
}

bool PulseAudioOutput::stoppedPlayback() {
  A_D(PulseAudioOutput);

  d->closeOutput();

  // Clear all processed buffers.
  d->mBuffers.clear();
  d->mBufferWrapper.reset();
  d->mCurrentBuffer.reset();

  resetPort(input());

  return true;
}

void PulseAudioOutput::process(ProcessIOFlags *ioFlags) {
  A_D(PulseAudioOutput);

  // Do nothing if the input sink is not linked.
  if(!input()->isLinked()) {
    WARNING_THIS("PulseAudioOutput::process") << "No source linked to input." << std::endl;
    return;
  }

  ManagedBuffer buffer;

  // Pull from the input sink.
  PullResult result = pull(input(), &buffer);

  if(result == kSuccess) {

    d->writeOutput(std::move(buffer));

    // Hint that the PulseAudioOutput could process more buffers since the internal ring buffer is not full.
    if(!d->outputBufferFull()) {
      (*ioFlags) |= kProcessMoreHint;
    }

  }
  else {
    ERROR_THIS("PulseAudioOutput::process") << "Pull error, PullResult=" << result << "." << std::endl;
  }
}

bool PulseAudioOutput::reconfigureIO() {
  return true;
}

bool PulseAudioOutput::reconfigureInputFormat(const Sink &sink, const BufferFormat &format ) {
#pragma unused(sink)

  A_D(PulseAudioOutput);
  d->stopOutput();
  return d->startOutput(format);
}