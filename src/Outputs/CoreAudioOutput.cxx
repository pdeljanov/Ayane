/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Portions based on the SFBAudioEngine.
 * SFBAudioEngine Copyright (c) Stephen F. Booth <me@sbooth.org>
 *
 */

#include "Ayane/Outputs/CoreAudioOutput.h"

#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioToolbox.h>
#include <vector>
#include <memory>

#include "Ayane/MessageBus.h"
#include "Ayane/Trace.h"
#include "Ayane/RawBuffer.h"

namespace Ayane {
    
    class CoreAudioOutputPrivate {
    public:
        
        CoreAudioOutputPrivate();
        ~CoreAudioOutputPrivate();
        
        bool createOutputDeviceUID(CFStringRef& deviceUID) const;
        bool setOutputDeviceUID(CFStringRef deviceUID);
        
        bool outputDeviceID(AudioDeviceID& deviceID) const;
        bool setOutputDeviceID(AudioDeviceID deviceID);
        
        bool outputDeviceSampleRate(Float64& sampleRate) const;
        bool setOutputDeviceSampleRate(Float64 sampleRate);
        
        bool openOutput();
        bool closeOutput();
        
        bool startOutput();
        bool stopOutput();
        
        bool resetOutput();
        
        bool isOutputRunning() const;
        
        
        bool saveGraphInteractions(std::vector<AUNodeInteraction> &interactions);
        bool restoreGraphInteractions(std::vector<AUNodeInteraction> &interactions);
        
        bool setPropertyOnAUGraphNodes(AudioUnitPropertyID propertyID,
                                       const void *propertyData,
                                       UInt32 propertyDataSize);
        
        bool setAUGraphSampleRateAndChannelLayout(Float64 sampleRate,
                                                  UInt32 channelsPerFrame);
        
        bool setAUOutputChannelLayout(AudioChannelLayout *channelLayout,
                                      SInt32 **outChannelMap,
                                      UInt32 *outChannelMapCount);
        
        AudioBufferList *allocateABL(UInt32 channelsPerFrame,
                                     UInt32 bytesPerSample,
                                     bool interleaved,
                                     UInt32 capacityFrames);
        
        
        /// AUNode render callback.
        OSStatus render(AudioUnitRenderActionFlags		*ioActionFlags,
                        const AudioTimeStamp			    *inTimeStamp,
                        UInt32							 inBusNumber,
                        UInt32							 inNumberFrames,
                        AudioBufferList					*ioData);
        
        /// AUGraph render notification.
        OSStatus renderNotify(AudioUnitRenderActionFlags *ioActionFlags,
                              const AudioTimeStamp		 *inTimeStamp,
                              UInt32                      inBusNumber,
                              UInt32						  inNumberFrames,
                              AudioBufferList            *ioData);
        
        
        /// Audio Unit graph
        AUGraph mAUGraph;
        
        /// Output node.
        AUNode mAUOutput;
        
        /// Audio format for the AU graph.
        AudioStreamBasicDescription	mAUFormat;
        
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


namespace {
    
	// AUGraph input callback
	OSStatus auRenderCallback(void							*inRefCon,
                              AudioUnitRenderActionFlags    *ioActionFlags,
                              const AudioTimeStamp			*inTimeStamp,
                              UInt32                        inBusNumber,
                              UInt32						    inNumberFrames,
                              AudioBufferList				*ioData)
	{
        
        CoreAudioOutputPrivate *endpoint = static_cast<CoreAudioOutputPrivate*>(inRefCon);
        
		return endpoint->render(ioActionFlags, inTimeStamp, inBusNumber,
                                inNumberFrames, ioData);
	}
    
	// AUGraph render notify callback
	OSStatus auGraphRenderNotify(void                       *inRefCon,
								 AudioUnitRenderActionFlags	*ioActionFlags,
								 const AudioTimeStamp		*inTimeStamp,
								 UInt32						inBusNumber,
								 UInt32						inNumberFrames,
								 AudioBufferList            *ioData)
	{
        
        CoreAudioOutputPrivate *endpoint = static_cast<CoreAudioOutputPrivate*>(inRefCon);
        
		return endpoint->renderNotify(ioActionFlags, inTimeStamp, inBusNumber,
                                      inNumberFrames, ioData);
	}
    
}




CoreAudioOutputPrivate::CoreAudioOutputPrivate() :
mAUGraph(nullptr),
mAUOutput(-1),
mAUChannelLayout(nullptr),
mMaxFramesPerSlice(0),
mLastClockTickHostTime(0),
mClockProvider(ClockCapabilities(0, 1000000000), 100000000),
mBuffers(2)
{
    /*
     * The AU graph will always use the canonical Core Audio format since the
     * buffers flowing through the audio engine are in an agnostic format.
     *
     * NOTE: Since we didn't zero the structure via a memset, don't delete any
     *       "uncessary" property setters.
     */
    
    mAUFormat.mFormatID			= kAudioFormatLinearPCM;
	mAUFormat.mFormatFlags		= kAudioFormatFlagsAudioUnitCanonical;
    
    // Sample rate and channels per frame can only be set once a buffer is
    // received.
	mAUFormat.mSampleRate		= 0;
	mAUFormat.mChannelsPerFrame	= 0;
	mAUFormat.mBitsPerChannel	= 8 * sizeof(AudioUnitSampleType);
	
	mAUFormat.mBytesPerPacket	= (mAUFormat.mBitsPerChannel / 8);
	mAUFormat.mFramesPerPacket	= 1;
	mAUFormat.mBytesPerFrame		= mAUFormat.mBytesPerPacket * mAUFormat.mFramesPerPacket;
	
	mAUFormat.mReserved			= 0;
}

CoreAudioOutputPrivate::~CoreAudioOutputPrivate() {
    
}

bool CoreAudioOutputPrivate::createOutputDeviceUID(CFStringRef& deviceUID) const
{
	AudioDeviceID deviceID;
    
	if(!outputDeviceID(deviceID)) {
		return false;
    }
    
	AudioObjectPropertyAddress propAddr;
    
    propAddr.mSelector	= kAudioDevicePropertyDeviceUID;
    propAddr.mScope		= kAudioObjectPropertyScopeGlobal;
    propAddr.mElement	= kAudioObjectPropertyElementMaster;
    
	UInt32 dataSize = sizeof(deviceUID);
	OSStatus result = AudioObjectGetPropertyData(deviceID,
                                                 &propAddr,
                                                 0,
                                                 nullptr,
                                                 &dataSize,
                                                 &deviceUID);
    
	if(kAudioHardwareNoError != result) {
        ERROR_THIS("CoreAudioOutputPrivate::createOutputDeviceUID")
        << "ErrorCode=" << result << std::endl;
		return nullptr;
	}
    
	return true;
}

bool CoreAudioOutputPrivate::setOutputDeviceUID(CFStringRef deviceUID)
{
	AudioDeviceID deviceID = kAudioDeviceUnknown;
    
	// If nullptr was passed as the device UID, use the default output device
	if(nullptr == deviceUID) {
        
		AudioObjectPropertyAddress propAddr;
        
        propAddr.mSelector	= kAudioHardwarePropertyDefaultOutputDevice;
        propAddr.mScope		= kAudioObjectPropertyScopeGlobal;
        propAddr.mElement	= kAudioObjectPropertyElementMaster;
        
		UInt32 specifierSize = sizeof(deviceID);
        
		OSStatus result = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                                     &propAddr,
                                                     0,
                                                     nullptr,
                                                     &specifierSize,
                                                     &deviceID);
        
		if(kAudioHardwareNoError != result) {
            ERROR_THIS("CoreAudioOutputPrivate::setOutputDeviceUID")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
        
	}
	else {
        
		AudioObjectPropertyAddress propAddr;
        
        propAddr.mSelector	= kAudioHardwarePropertyDeviceForUID;
        propAddr.mScope		= kAudioObjectPropertyScopeGlobal;
        propAddr.mElement	= kAudioObjectPropertyElementMaster;
        
		AudioValueTranslation translation = {
			&deviceUID, sizeof(deviceUID),
			&deviceID, sizeof(deviceID)
		};
        
		UInt32 specifierSize = sizeof(translation);
        
		OSStatus result = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                                     &propAddr,
                                                     0,
                                                     nullptr,
                                                     &specifierSize,
                                                     &translation);
        
		if(kAudioHardwareNoError != result) {
            ERROR_THIS("CoreAudioOutputPrivate::setOutputDeviceUID")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
	}
    
	if(kAudioDeviceUnknown == deviceID) {
        ERROR_THIS("CoreAudioOutputPrivate::setOutputDeviceUID")
        << "Unknown device ID." << std::endl;
		return false;
    }
    
	return setOutputDeviceID(deviceID);
}

bool CoreAudioOutputPrivate::outputDeviceID(AudioDeviceID& deviceID) const
{
	AudioUnit au = nullptr;
	OSStatus result = AUGraphNodeInfo(mAUGraph, mAUOutput, nullptr, &au);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::outputDeviceID")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	UInt32 dataSize = sizeof(deviceID);
    
	result = AudioUnitGetProperty(au,
                                  kAudioOutputUnitProperty_CurrentDevice,
                                  kAudioUnitScope_Global,
                                  0,
                                  &deviceID,
                                  &dataSize);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::outputDeviceID")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	return true;
}

bool CoreAudioOutputPrivate::setOutputDeviceID(AudioDeviceID deviceID)
{
	if(kAudioDeviceUnknown == deviceID)
		return false;
    
	AudioUnit au = nullptr;
	OSStatus result = AUGraphNodeInfo(mAUGraph, mAUOutput, nullptr, &au);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::setOutputDeviceID")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	// Update our output AU to use the specified device
	result = AudioUnitSetProperty(au,
                                  kAudioOutputUnitProperty_CurrentDevice,
                                  kAudioUnitScope_Global,
                                  0,
                                  &deviceID,
                                  (UInt32)sizeof(deviceID));
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::setOutputDeviceID")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	return true;
}

bool CoreAudioOutputPrivate::outputDeviceSampleRate(Float64& sampleRate) const
{
	AudioDeviceID deviceID;
    
	if(!outputDeviceID(deviceID)) {
		return false;
    }
    
	AudioObjectPropertyAddress propAddr;
    
    propAddr.mSelector	= kAudioDevicePropertyNominalSampleRate,
    propAddr.mScope		= kAudioObjectPropertyScopeGlobal,
    propAddr.mElement	= kAudioObjectPropertyElementMaster;
    
	UInt32 dataSize = sizeof(sampleRate);
    
	OSStatus result = AudioObjectGetPropertyData(deviceID,
                                                 &propAddr,
                                                 0,
                                                 nullptr,
                                                 &dataSize,
                                                 &sampleRate);
    
	if(kAudioHardwareNoError != result) {
        ERROR_THIS("CoreAudioOutputPrivate::outputDeviceSampleRate")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	return true;
}

bool CoreAudioOutputPrivate::setOutputDeviceSampleRate(Float64 sampleRate)
{
	AudioDeviceID deviceID;
    
	if(!outputDeviceID(deviceID)) {
		return false;
    }
    
	AudioObjectPropertyAddress propAddr;
    
    propAddr.mSelector	= kAudioDevicePropertyNominalSampleRate;
    propAddr.mScope		= kAudioObjectPropertyScopeGlobal;
    propAddr.mElement	= kAudioObjectPropertyElementMaster;
    
    // Get the current sample rate.
	Float64 currentSampleRate;
    
	UInt32 dataSize = sizeof(currentSampleRate);
	
	OSStatus result = AudioObjectGetPropertyData(deviceID,
                                                 &propAddr,
                                                 0,
                                                 nullptr,
                                                 &dataSize,
                                                 &currentSampleRate);
	if(kAudioHardwareNoError != result) {
        ERROR_THIS("CoreAudioOutputPrivate::setOutputDeviceSampleRate")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
    // Attempting to set the same sample rate.
	if(currentSampleRate == sampleRate) {
		return true;
    }
    
    // Set the new sample rate.
	dataSize = sizeof(sampleRate);
    
	result = AudioObjectSetPropertyData(deviceID,
                                        &propAddr,
                                        0,
                                        nullptr,
                                        sizeof(sampleRate),
                                        &sampleRate);
    
	if(kAudioHardwareNoError != result) {
        ERROR_THIS("CoreAudioOutputPrivate::setOutputDeviceSampleRate")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	return true;
}

bool CoreAudioOutputPrivate::openOutput() {
    
    OSStatus result = NewAUGraph(&mAUGraph);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::openOutput")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
    // Output component description
    AudioComponentDescription desc;
    
    // Set up the output node
	desc.componentType			= kAudioUnitType_Output;
	desc.componentSubType		= kAudioUnitSubType_HALOutput;
	desc.componentManufacturer	= kAudioUnitManufacturer_Apple;
	desc.componentFlags			= kAudioComponentFlag_SandboxSafe;
	desc.componentFlagsMask		= 0;
    
	result = AUGraphAddNode(mAUGraph, &desc, &mAUOutput);
    
	if(result != noErr) {
        
        ERROR_THIS("CoreAudioOutputPrivate::openOutput")
        << "ErrorCode=" << result << std::endl;
        
		result = DisposeAUGraph(mAUGraph);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::openOutput")
            << "ErrorCode=" << result << std::endl;
        }
        
		mAUGraph = nullptr;
		return false;
	}
    
    
	// Install the input callback
	/*
     
     
     if(result != noErr) {
     
     // TODO: Log.
     
     if(DisposeAUGraph(mAUGraph) != noErr) {
     // TODO: Log.
     }
     
     mAUGraph = nullptr;
     return false;
     }
     */
    
	// Open the graph
	result = AUGraphOpen(mAUGraph);
    
	if(result != noErr) {
        
        ERROR_THIS("CoreAudioOutputPrivate::openOutput")
        << "ErrorCode=" << result << std::endl;
        
		if(DisposeAUGraph(mAUGraph) != noErr){
            ERROR_THIS("CoreAudioOutputPrivate::openOutput")
            << "ErrorCode=" << result << std::endl;
        }
        
		mAUGraph = nullptr;
		return false;
	}
    
    
    // Get the audio output unit.
    AudioUnit au = nullptr;
	result = AUGraphNodeInfo(mAUGraph, mAUOutput, nullptr, &au);
    
    if(result != noErr) {
        
        ERROR_THIS("CoreAudioOutputPrivate::openOutput")
        << "ErrorCode=" << result << std::endl;
        
		if(DisposeAUGraph(mAUGraph) != noErr){
            ERROR_THIS("CoreAudioOutputPrivate::openOutput")
            << "ErrorCode=" << result << std::endl;
        }
        
		mAUGraph = nullptr;
		return false;
	}
    
    // Install the output render callback (do this manually!).
    AURenderCallbackStruct cbs = { auRenderCallback, this };
    
    AudioUnitSetProperty(au,
                         kAudioUnitProperty_SetRenderCallback,
                         kAudioUnitScope_Global,
                         0,
                         &cbs,
                         sizeof(cbs));
    
    if(result != noErr) {
        
        ERROR_THIS("CoreAudioOutputPrivate::openOutput")
        << "ErrorCode=" << result << std::endl;
        
		if(DisposeAUGraph(mAUGraph) != noErr){
            ERROR_THIS("CoreAudioOutputPrivate::openOutput")
            << "ErrorCode=" << result << std::endl;
        }
        
		mAUGraph = nullptr;
		return false;
	}
    
    // Install the graph render notification used to drive the clock.
	result = AUGraphAddRenderNotify(mAUGraph, auGraphRenderNotify, this);
    
	if(result != noErr) {
        
        ERROR_THIS("CoreAudioOutputPrivate::openOutput")
        << "ErrorCode=" << result << std::endl;
        
		if(DisposeAUGraph(mAUGraph) != noErr){
            ERROR_THIS("CoreAudioOutputPrivate::openOutput")
            << "ErrorCode=" << result << std::endl;
        }
        
		mAUGraph = nullptr;
		return false;
	}
    
    // Save the default maximum number of frames per slice.
	UInt32 dataSize = sizeof(mMaxFramesPerSlice);
	result = AudioUnitGetProperty(au, kAudioUnitProperty_MaximumFramesPerSlice,
                                  kAudioUnitScope_Global,
                                  0,
                                  &mMaxFramesPerSlice,
                                  &dataSize);
	if(result != noErr) {
        
        ERROR_THIS("CoreAudioOutputPrivate::openOutput")
        << "ErrorCode=" << result << std::endl;
        
		if(DisposeAUGraph(mAUGraph) != noErr){
            ERROR_THIS("CoreAudioOutputPrivate::openOutput")
            << "ErrorCode=" << result << std::endl;
        }
        
		mAUGraph = nullptr;
		return false;
	}
    
    
    // Initialize the graph
	result = AUGraphInitialize(mAUGraph);
    
	if(result != noErr) {
        
        ERROR_THIS("CoreAudioOutputPrivate::openOutput")
        << "ErrorCode=" << result << std::endl;
        
		if(DisposeAUGraph(mAUGraph) != noErr){
            ERROR_THIS("CoreAudioOutputPrivate::openOutput")
            << "ErrorCode=" << result << std::endl;
        }
        
		mAUGraph = nullptr;
		return false;
	}
    
    return true;
}

bool CoreAudioOutputPrivate::closeOutput()
{
	Boolean graphIsRunning = false;
	OSStatus result = AUGraphIsRunning(mAUGraph, &graphIsRunning);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::closeOutput")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	if(graphIsRunning) {
		result = AUGraphStop(mAUGraph);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::closeOutput")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
	}
    
	Boolean graphIsInitialized = false;
	result = AUGraphIsInitialized(mAUGraph, &graphIsInitialized);
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::closeOutput")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	if(graphIsInitialized) {
		result = AUGraphUninitialize(mAUGraph);
		
        if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::closeOutput")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
	}
    
	result = AUGraphClose(mAUGraph);
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::closeOutput")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	result = DisposeAUGraph(mAUGraph);
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::closeOutput")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	mAUGraph = nullptr;
	mAUOutput = -1;
    
	return true;
}

bool CoreAudioOutputPrivate::startOutput()
{
	// We don't want to start output in the middle of a buffer modification
	/*std::unique_lock<std::mutex> lock(mMutex, std::try_to_lock);
     if(!lock)
     return false;
     */
    
	OSStatus result = AUGraphStart(mAUGraph);
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::startOutput")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
	
	return true;
}

bool CoreAudioOutputPrivate::stopOutput()
{
	OSStatus result = AUGraphStop(mAUGraph);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::stopOutput")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	return true;
}

bool CoreAudioOutputPrivate::isOutputRunning() const
{
	Boolean isRunning = false;
	OSStatus result = AUGraphIsRunning(mAUGraph, &isRunning);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::isOutputRunning")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	return isRunning;
}

bool CoreAudioOutputPrivate::resetOutput()
{
    
	UInt32 nodeCount = 0;
	OSStatus result = AUGraphGetNodeCount(mAUGraph, &nodeCount);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::resetOutput")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	for(UInt32 i = 0; i < nodeCount; ++i) {
        
		AUNode node = 0;
        
		result = AUGraphGetIndNode(mAUGraph, i, &node);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::resetOutput")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
        
		AudioUnit au = nullptr;
        
		result = AUGraphNodeInfo(mAUGraph, node, nullptr, &au);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::resetOutput")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
        
		result = AudioUnitReset(au, kAudioUnitScope_Global, 0);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::resetOutput")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
	}
    
	return true;
}

bool CoreAudioOutputPrivate::saveGraphInteractions(std::vector<AUNodeInteraction> &interactions ) {
    
    UInt32 interactionCount = 0;
    
	OSStatus result = AUGraphGetNumberOfInteractions(mAUGraph, &interactionCount);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::saveGraphInteractions")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
    interactions.reserve(interactionCount);
    
    for(UInt32 i = 0; i < interactionCount; ++i) {
        
		result = AUGraphGetInteractionInfo(mAUGraph, i, &interactions[i]);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::saveGraphInteractions")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
        
	}
    return true;
}

bool CoreAudioOutputPrivate::restoreGraphInteractions(std::vector<AUNodeInteraction> &interactions) {
    
    UInt32 interactionCount = interactions.size();
    
    OSStatus result;
    
    for(UInt32 i = 0; i < interactionCount; ++i) {
        
		switch(interactions[i].nodeInteractionType) {
                
                // Reestablish the connection
			case kAUNodeInteraction_Connection:
			{
				result = AUGraphConnectNodeInput(mAUGraph,
												 interactions[i].nodeInteraction.connection.sourceNode,
												 interactions[i].nodeInteraction.connection.sourceOutputNumber,
												 interactions[i].nodeInteraction.connection.destNode,
												 interactions[i].nodeInteraction.connection.destInputNumber);
				
				if(result != noErr) {
                    ERROR_THIS("CoreAudioOutputPrivate::restoreGraphInteractions")
                    << "ErrorCode=" << result << std::endl;
					return false;
				}
                
				break;
			}
                // Reestablish the input callback
			case kAUNodeInteraction_InputCallback:
			{
				result = AUGraphSetNodeInputCallback(mAUGraph,
													 interactions[i].nodeInteraction.inputCallback.destNode,
													 interactions[i].nodeInteraction.inputCallback.destInputNumber,
													 &interactions[i].nodeInteraction.inputCallback.cback);
                
				if(result != noErr) {
                    ERROR_THIS("CoreAudioOutputPrivate::restoreGraphInteractions")
                    << "ErrorCode=" << result << std::endl;
					return false;
				}
                
				break;
			}
		}
	}
    
    return true;
    
}

bool CoreAudioOutputPrivate::setPropertyOnAUGraphNodes(AudioUnitPropertyID propertyID,
                                                       const void *propertyData,
                                                       UInt32 propertyDataSize)
{
    
	if(nullptr == propertyData || 0 >= propertyDataSize)
		return  false;
    
	UInt32 nodeCount = 0;
    
	OSStatus result = AUGraphGetNodeCount(mAUGraph, &nodeCount);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::setPropertyOnAUGraphNodes")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	// Iterate through the nodes and attempt to set the property
	for(UInt32 i = 0; i < nodeCount; ++i) {
        
		AUNode node;
        
		result = AUGraphGetIndNode(mAUGraph, i, &node);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::setPropertyOnAUGraphNodes")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
        
		AudioUnit au = nullptr;
        
		result = AUGraphNodeInfo(mAUGraph, node, nullptr, &au);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::setPropertyOnAUGraphNodes")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
        
		if(mAUOutput == node) {
            
			// For AUHAL as the output node, you can't set the device side, so just set the client side
			result = AudioUnitSetProperty(au,
                                          propertyID,
                                          kAudioUnitScope_Input,
                                          0,
                                          propertyData,
                                          propertyDataSize);
            
			if(result != noErr) {
                ERROR_THIS("CoreAudioOutputPrivate::setPropertyOnAUGraphNodes")
                << "ErrorCode=" << result << std::endl;
				return false;
			}
            
		}
		else {
            
			UInt32 elementCount = 0;
			UInt32 dataSize = sizeof(elementCount);
            
			result = AudioUnitGetProperty(au,
                                          kAudioUnitProperty_ElementCount,
                                          kAudioUnitScope_Input,
                                          0,
                                          &elementCount,
                                          &dataSize);
            
			if(result != noErr) {
                ERROR_THIS("CoreAudioOutputPrivate::setPropertyOnAUGraphNodes")
                << "ErrorCode=" << result << std::endl;
				return false;
			}
            
			for(UInt32 j = 0; j < elementCount; ++j) {
                
				result = AudioUnitSetProperty(au,
                                              propertyID,
                                              kAudioUnitScope_Input,
                                              j,
                                              propertyData,
                                              propertyDataSize);
				if(result != noErr) {
                    ERROR_THIS("CoreAudioOutputPrivate::setPropertyOnAUGraphNodes")
                    << "ErrorCode=" << result << std::endl;
					return false;
				}
			}
            
			elementCount = 0;
			dataSize = sizeof(elementCount);
            
			result = AudioUnitGetProperty(au,
                                          kAudioUnitProperty_ElementCount,
                                          kAudioUnitScope_Output,
                                          0,
                                          &elementCount,
                                          &dataSize);
            
			if(result != noErr) {
                ERROR_THIS("CoreAudioOutputPrivate::setPropertyOnAUGraphNodes")
                << "ErrorCode=" << result << std::endl;
				return false;
			}
            
			for(UInt32 j = 0; j < elementCount; ++j) {
                
				result = AudioUnitSetProperty(au,
                                              propertyID,
                                              kAudioUnitScope_Output,
                                              j,
                                              propertyData,
                                              propertyDataSize);
                
				if(result != noErr) {
                    ERROR_THIS("CoreAudioOutputPrivate::setPropertyOnAUGraphNodes")
                    << "ErrorCode=" << result << std::endl;
					return false;
				}
			}
		}
	}
    
	return true;
}


bool CoreAudioOutputPrivate::setAUGraphSampleRateAndChannelLayout(Float64 sampleRate,
                                                                  UInt32 channelsPerFrame)
{
    
	OSStatus result;
    
	// ========================================
	// Save the interaction information and then clear all the connections
    
    std::vector<AUNodeInteraction> interactions;
    if(!saveGraphInteractions(interactions)) {
        return false;
    }
    
	result = AUGraphClearConnections(mAUGraph);
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::setAUGraphSampleRateAndChannelLayout")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	// ========================================
	// Attempt to set the new stream format
    
    AudioStreamBasicDescription format = mAUFormat;
	
	format.mChannelsPerFrame    = channelsPerFrame;
	format.mSampleRate          = sampleRate;
    
	if(!setPropertyOnAUGraphNodes(kAudioUnitProperty_StreamFormat,
                                  &format,
                                  sizeof(format)))
    {
        
		// Restore the previous format if the new format did not take.
		if(!setPropertyOnAUGraphNodes(kAudioUnitProperty_StreamFormat,
                                      &mAUFormat,
                                      sizeof(mAUFormat)))
        {
            ERROR_THIS("CoreAudioOutputPrivate::setAUGraphSampleRateAndChannelLayout")
            << "Could not restore previous configuration." << std::endl;
		}
        
	}
	else {
		mAUFormat = format;
    }
    
	// ========================================
	// Restore the graph's connections and input callbacks
    if(!restoreGraphInteractions(interactions)) {
        return false;
    }
    
	// ========================================
	// Adjust maximum frames per slice if there is sample rate conversion.
    
	AudioUnit au = nullptr;
	result = AUGraphNodeInfo(mAUGraph, mAUOutput, nullptr, &au);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::setAUGraphSampleRateAndChannelLayout")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	Float64 inputSampleRate = 0;
	UInt32 dataSize = sizeof(inputSampleRate);
    
	result = AudioUnitGetProperty(au,
                                  kAudioUnitProperty_SampleRate,
                                  kAudioUnitScope_Input,
                                  0,
                                  &inputSampleRate,
                                  &dataSize);
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::setAUGraphSampleRateAndChannelLayout")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	Float64 outputSampleRate = 0;
	dataSize = sizeof(outputSampleRate);
    
	result = AudioUnitGetProperty(au,
                                  kAudioUnitProperty_SampleRate,
                                  kAudioUnitScope_Output,
                                  0,
                                  &outputSampleRate,
                                  &dataSize);
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::setAUGraphSampleRateAndChannelLayout")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	UInt32 newMaxFrames = mMaxFramesPerSlice;
    
	// If the output unit's input and output sample rates don't match,
    // calculate a working maximum number of frames per slice
	if(inputSampleRate != outputSampleRate) {
		
		Float64 ratio = inputSampleRate / outputSampleRate;
		Float64 multiplier = std::max(1.0, ratio);
        
		// Round up to the nearest 16 frames
		newMaxFrames = (UInt32)ceil(mMaxFramesPerSlice * multiplier);
		newMaxFrames += 16;
		newMaxFrames &= 0xFFFFFFF0;
	}
    
	UInt32 currentMaxFrames = 0;
	dataSize = sizeof(currentMaxFrames);
    
	result = AudioUnitGetProperty(au,
                                  kAudioUnitProperty_MaximumFramesPerSlice,
                                  kAudioUnitScope_Global,
                                  0,
                                  &currentMaxFrames,
                                  &dataSize);
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::setAUGraphSampleRateAndChannelLayout")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	// Adjust the maximum frames per slice if necessary
	if(newMaxFrames != currentMaxFrames) {
        
        // Restore the previous maximum number of frames per slice if the new
        // value did not take.
		if(!setPropertyOnAUGraphNodes(kAudioUnitProperty_MaximumFramesPerSlice,
                                      &newMaxFrames,
                                      sizeof(newMaxFrames)))
        {
			return false;
		}
	}
    
	return true;
    
}

bool CoreAudioOutputPrivate::setAUOutputChannelLayout(AudioChannelLayout *channelLayout,
                                                      SInt32 **outChannelMap,
                                                      UInt32 *outChannelMapCount)
{
    
    // Do nothing if channelLayout is null.
    if(channelLayout == nullptr) {
		return true;
    }
    
	AudioUnit outputUnit = nullptr;
    
	OSStatus result = AUGraphNodeInfo(mAUGraph, mAUOutput, nullptr, &outputUnit);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::setAUOutputChannelLayout")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	// Clear the existing channel map
	result = AudioUnitSetProperty(outputUnit,
                                  kAudioOutputUnitProperty_ChannelMap,
                                  kAudioUnitScope_Input,
                                  0,
                                  nullptr,
                                  0);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutputPrivate::setAUOutputChannelLayout")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
    
    // Test if the output channel layout is Stereo.
	AudioChannelLayout stereoChannelLayout;
    memset(&stereoChannelLayout, 0, sizeof(AudioChannelLayout));
    stereoChannelLayout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
    
	AudioChannelLayout* layouts[] = {
		channelLayout,
		&stereoChannelLayout
	};
    
	UInt32 channelLayoutIsStereo = false;
	UInt32 propertySize = sizeof(channelLayoutIsStereo);
    
	result = AudioFormatGetProperty(kAudioFormatProperty_AreChannelLayoutsEquivalent,
                                    sizeof(layouts),
                                    (void *)layouts,
                                    &propertySize,
                                    &channelLayoutIsStereo);
    
	if(result != noErr){
        ERROR_THIS("CoreAudioOutputPrivate::setAUOutputChannelLayout")
        << "ErrorCode=" << result << std::endl;
    }
    
    
	// Stereo
	if(channelLayoutIsStereo) {
        
        // Get the preferred Stereo channels.
		UInt32 preferredChannelsForStereo [2];
		UInt32 preferredChannelsForStereoSize = sizeof(preferredChannelsForStereo);
        
		result = AudioUnitGetProperty(outputUnit,
                                      kAudioDevicePropertyPreferredChannelsForStereo,
                                      kAudioUnitScope_Output,
                                      0,
                                      preferredChannelsForStereo,
                                      &preferredChannelsForStereoSize);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::setAUOutputChannelLayout")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
        
		// Build a channel map using the preferred stereo channels
		AudioStreamBasicDescription outputFormat;
		propertySize = sizeof(outputFormat);
        
		result = AudioUnitGetProperty(outputUnit,
                                      kAudioUnitProperty_StreamFormat,
                                      kAudioUnitScope_Output,
                                      0,
                                      &outputFormat,
                                      &propertySize);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::setAUOutputChannelLayout")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
        
        SInt32 *channelMap = new SInt32[outputFormat.mChannelsPerFrame];
        
		for(UInt32 i = 0; i <  outputFormat.mChannelsPerFrame; ++i) {
			channelMap[i] = -1;
        }
        
		channelMap[preferredChannelsForStereo[0] - 1] = 0;
		channelMap[preferredChannelsForStereo[1] - 1] = 1;
        
		// Set the channel map
		result = AudioUnitSetProperty(outputUnit,
                                      kAudioOutputUnitProperty_ChannelMap,
                                      kAudioUnitScope_Input,
                                      0,
                                      channelMap,
                                      (UInt32)sizeof(channelMap));
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::setAUOutputChannelLayout")
            << "ErrorCode=" << result << std::endl;
            delete channelMap;
			return false;
		}
        
        *outChannelMap = channelMap;
        *outChannelMapCount = outputFormat.mChannelsPerFrame;
	}
	// Multichannel
	else {
        
		// Get the device's preferred channel layout array size.
		UInt32 devicePreferredChannelLayoutSize = 0;
        
		result = AudioUnitGetPropertyInfo(outputUnit,
                                          kAudioDevicePropertyPreferredChannelLayout,
                                          kAudioUnitScope_Output,
                                          0,
                                          &devicePreferredChannelLayoutSize,
                                          nullptr);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::setAUOutputChannelLayout")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
        
        // Allocate an array for the device's preferred channel list.
        std::unique_ptr<AudioChannelLayout> devicePreferredChannelLayout(static_cast<AudioChannelLayout*>(malloc(devicePreferredChannelLayoutSize)));
        
		result = AudioUnitGetProperty(outputUnit,
                                      kAudioDevicePropertyPreferredChannelLayout,
                                      kAudioUnitScope_Output,
                                      0,
                                      devicePreferredChannelLayout.get(),
                                      &devicePreferredChannelLayoutSize);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::setAUOutputChannelLayout")
            << "ErrorCode=" << result << std::endl;
			return false;
		}
        
        // Get the device's preferred channel list.
		UInt32 channelCount = 0;
		UInt32 dataSize = sizeof(channelCount);
        
		result = AudioFormatGetProperty(kAudioFormatProperty_NumberOfChannelsForLayout,
                                        devicePreferredChannelLayoutSize,
                                        devicePreferredChannelLayout.get(),
                                        &dataSize,
                                        &channelCount);
        
        if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::setAUOutputChannelLayout")
            << "GetProperty=kAudioFormatProperty_NumberOfChannelsForLayout, ErrorCode=" << result << std::endl;
			return false;
		}
        
		AudioChannelLayout* channelLayouts[] = {
            channelLayout,
            devicePreferredChannelLayout.get()
		};

        std::unique_ptr<SInt32> channelMap(new SInt32[channelCount]);
		dataSize = (UInt32)(sizeof(SInt32) * channelCount);
        
		result = AudioFormatGetProperty(kAudioFormatProperty_ChannelMap,
                                        sizeof(channelLayouts),
                                        channelLayouts,
                                        &dataSize,
                                        channelMap.get());
        
        if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::setAUOutputChannelLayout")
            << "GetProperty=kAudioFormatProperty_ChannelMap"
            << ", ErrorCode=" << result << std::endl;
			return false;
		}
        
		// Set the channel map
		result = AudioUnitSetProperty(outputUnit,
                                      kAudioOutputUnitProperty_ChannelMap,
                                      kAudioUnitScope_Input,
                                      0,
                                      channelMap.get(),
                                      dataSize);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutputPrivate::setAUOutputChannelLayout")
            << "SetProperty=kAudioOutputUnitProperty_ChannelMap"
            << ", ErrorCode=" << result << std::endl;
			return false;
		}
        
        *outChannelMap = channelMap.release();
        *outChannelMapCount = channelCount;
	}
    
    return true;
}

AudioBufferList * CoreAudioOutputPrivate::allocateABL(UInt32 channelsPerFrame,
                                                      UInt32 bytesPerSample,
                                                      bool interleaved,
                                                      UInt32 capacityFrames)
{
    UInt32 bytesPerFrame = bytesPerSample * channelsPerFrame;
    
    AudioBufferList *abl = NULL;
    
    UInt32 numBuffers = interleaved ? 1 : channelsPerFrame;
    UInt32 channelsPerBuffer = interleaved ? channelsPerFrame : 1;
    
    abl = static_cast<AudioBufferList *>(calloc(1, offsetof(AudioBufferList, mBuffers) + (sizeof(AudioBuffer) * numBuffers)));
    
    abl->mNumberBuffers = numBuffers;
    
    for(UInt32 bufferIndex = 0; bufferIndex < abl->mNumberBuffers; ++bufferIndex) {
        abl->mBuffers[bufferIndex].mData = static_cast<void *>(calloc(capacityFrames, bytesPerFrame));
        abl->mBuffers[bufferIndex].mDataByteSize = capacityFrames * bytesPerFrame;
        abl->mBuffers[bufferIndex].mNumberChannels = channelsPerBuffer;
    }
    
    return abl;
}









CoreAudioOutput::CoreAudioOutput() : Stage(), d_ptr(new CoreAudioOutputPrivate) {
    // Add output sink to stage.
    addSink("input");
}

CoreAudioOutput::~CoreAudioOutput() {
    delete d_ptr;
}


SharingMode CoreAudioOutput::sharingMode() const {
    A_D(const CoreAudioOutput);
    
	AudioObjectPropertyAddress propertyAddress;
    
    propertyAddress.mSelector	= kAudioDevicePropertyHogMode;
    propertyAddress.mScope		= kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement    = kAudioObjectPropertyElementMaster;
    
	pid_t hogPID = static_cast<pid_t>(-1);
	UInt32 dataSize = sizeof(hogPID);
    
	AudioDeviceID deviceID;
	if(!d->outputDeviceID(deviceID)) {
		return kShared;
    }
    
	OSStatus result = AudioObjectGetPropertyData(deviceID,
                                                 &propertyAddress,
                                                 0,
                                                 nullptr,
                                                 &dataSize,
                                                 &hogPID);
    
	if(kAudioHardwareNoError != result) {
		return kShared;
	}
    
	return (hogPID == getpid() ? kExclusive : kShared);
}

bool CoreAudioOutput::setSharingMode(SharingMode mode)
{
    A_D(CoreAudioOutput);
    
    AudioDeviceID deviceID;
	if(!d->outputDeviceID(deviceID)) {
		return false;
    }
    
	AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector   = kAudioDevicePropertyHogMode;
    propertyAddress.mScope      = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement    = kAudioObjectPropertyElementMaster;
    
	pid_t pid = static_cast<pid_t>(-1);
	UInt32 dataSize = sizeof(pid);
    
	OSStatus result = AudioObjectGetPropertyData(deviceID,
                                                 &propertyAddress,
                                                 0,
                                                 nullptr,
                                                 &dataSize,
                                                 &pid);

	if(kAudioHardwareNoError != result) {
        ERROR_THIS("CoreAudioOutput::setSharingMode")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
    // Get the application's PID.
    pid_t appPID = getpid();
    
    if( mode == kShared ) {
        
        // Device is not being used exclusively.
        if( pid == static_cast<pid_t>(-1) ) {
            return true;
        }
        // Check if the device is exclusive to another application. Can't do
        // anything.
        else if( pid != appPID ) {
            WARNING_THIS("CoreAudioOutput::setSharingMode") <<
            "Another PID is hogging the audio device." << std::endl;
            return false;
        }

        // Set pid to be shared.
        pid = static_cast<pid_t>(-1);
    }
    else {
        
        // Device is being used exclusively.
        if( pid == appPID ) {
            return true;
        }
        // Check if the device is exclusive to another application. Can't do
        // anything.
        else if( pid != appPID ) {
            WARNING_THIS("CoreAudioOutput::setSharingMode") <<
            "Another PID is hogging the audio device." << std::endl;
            return false;
        }
        
        // Set pid to us.
        pid = appPID;
    }
    
	bool restartIO = d->isOutputRunning();
	if(restartIO) {
		d->stopOutput();
    }
    
	result = AudioObjectSetPropertyData(deviceID,
                                        &propertyAddress,
                                        0,
                                        nullptr,
                                        sizeof(pid),
                                        &pid);
    
	if(kAudioHardwareNoError != result) {
        ERROR_THIS("CoreAudioOutput::setSharingMode")
        << "ErrorCode=" << result << std::endl;
		return false;
	}
    
	if(restartIO && !d->isOutputRunning()) {
		d->startOutput();
    }
    
	return true;
}


Stage::Sink *CoreAudioOutput::input()
{
    return mSinks["input"].get();
}

bool CoreAudioOutput::beginPlayback() {
    A_D(CoreAudioOutput);
    
    // Open output.
    if( !d->openOutput() ) {
        ERROR_THIS("CoreAudioOutput::beginPlayback") << "Could not open output."
        << std::endl;
        return false;
    }
    
    // Set default output device.
    CFStringRef device = nullptr;
    if( !d->setOutputDeviceUID(device) ) {
        ERROR_THIS("CoreAudioOutput::beginPlayback") << "Failed to set default "
        "output device." << std::endl;
        return false;
    }
    
    // Don't actually start the device till a buffer is received.
    

    double clockPeriodInSeconds = ((double)d->mClockProvider.clockPeriod() / 1000000000.0);
    d->mClockProvider.publish(clockPeriodInSeconds);

    return true;
}

bool CoreAudioOutput::stoppedPlayback() {
    A_D(CoreAudioOutput);
    
    d->stopOutput();
    d->closeOutput();
    
    // Clear all processed buffers.
    d->mBuffers.clear();
    d->mAudioBufferListWrapper.reset();
    d->mCurrentBuffer.reset();
    
    resetPort(input());

    return true;
}





void CoreAudioOutput::process( ProcessIOFlags *ioFlags ){
    A_D(CoreAudioOutput);
    
    ManagedBuffer buffer;

    if( !input()->isLinked() ) {
        WARNING_THIS("CoreAudioOutput::process") << "No source linked to input."
        << std::endl;
        return;
    }

    PullResult result = pull(input(), &buffer);
    
    // May kick off a sink reconfiguration.
    if( result == kSuccess ) {
        
        // Pass ownership of buffer to the queue.
        d->mBuffers.push(buffer);
        
        // Hint that the CoreAudioOutput could process more buffers since its
        // internal ring buffer is not full.
        if(!d->mBuffers.full()) {
            (*ioFlags) |= kProcessMoreHint;
        }
    }
    else {
        ERROR_THIS("CoreAudioOutput::process") << "Pull error, PullResult="
        << result << "." << std::endl;
    }
    
}

bool CoreAudioOutput::reconfigureIO() {
    return true;
}

bool CoreAudioOutput::reconfigureInputFormat(const Sink &sink,
                                             const BufferFormat &format)
{
#pragma unused(sink)
    
    A_D(CoreAudioOutput);

    
    TRACE_THIS("CoreAudioOutput::reconfigureSink")
    << "Attempting reconfiguration." << std::endl;
    
    /* Stop and unitialiaze the AUGraph. */
	Boolean graphIsRunning = FALSE;
	OSStatus result = AUGraphIsRunning(d->mAUGraph, &graphIsRunning);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutput::reconfigureSink")
        << "Couldn't get graph running state." << std::endl;
        
		return false;
	}
	
	if(graphIsRunning) {
        
		result = AUGraphStop(d->mAUGraph);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutput::reconfigureSink")
            << "Couldn't stop the graph." << std::endl;
			return false;
		}
	}

	Boolean graphIsInitialized = FALSE;
	result = AUGraphIsInitialized(d->mAUGraph, &graphIsInitialized);
    
	if(result != noErr) {
        ERROR_THIS("CoreAudioOutput::reconfigureSink")
        << "Couldn't get graph initialization state." << std::endl;
        
		return false;
	}
	
	if(graphIsInitialized) {
        
		result = AUGraphUninitialize(d->mAUGraph);
        
		if(result != noErr) {
            ERROR_THIS("CoreAudioOutput::reconfigureSink")
            << "Couldn't uninitialize the graph." << std::endl;
			return false;
		}
	}
    
    /* Reconfigure the graph. */
    if(!d->setAUGraphSampleRateAndChannelLayout(format.sampleRate(),
                                                format.channelCount())) {
        
        ERROR_THIS("CoreAudioOutput::reconfigureSink")
        << "Setting the sample rate and number of channels failed." << std::endl;
        
        return false;
    }
    
    
    
    // Create a CoreAudio channel layout with 0 channel descriptions and try to
    // configure it.
    size_t layoutSize = offsetof(AudioChannelLayout, mChannelDescriptions) +
        (0 * sizeof(AudioChannelDescription));
    
    // Allocate a channel layout.
    std::unique_ptr<AudioChannelLayout> channelLayout((AudioChannelLayout*)malloc(layoutSize));
    
    // Zero channel layout
    memset(channelLayout.get(), 0, layoutSize);

    // Use the channel bitmap because CoreAudio and Ayane channel bitmaps map
    // the same way.
    channelLayout->mNumberChannelDescriptions = 0;
    channelLayout->mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelBitmap;
    channelLayout->mChannelBitmap = format.channels();
    
    UInt32 channelCount = 0;
    SInt32 *channelMap = nullptr;
    
    // Set the channel layout and get the input channel to output channel map.
    if(!d->setAUOutputChannelLayout(channelLayout.get(),
                                    &channelMap,
                                    &channelCount) )
    {
        ERROR_THIS("CoreAudioOutput::reconfigureSink")
        << "Setting the channel layout failed. Do you have more input channels"
        " than output channels?" << std::endl;
        return false;
    }
    
    /* Update the raw buffer using the channel IO map. */
    d->mAudioBufferListWrapper.reset(new RawBuffer(d->mMaxFramesPerSlice,
                                                   channelCount,
                                                   kFloat32,
                                                   true));
    
    TRACE_THIS("CoreAudioOutput::reconfigureInputFormat")
    << "Using channel map:" << std::endl;
    
    for(UInt32 i = 0; i < channelCount; ++i) {
        TRACE_THIS("CoreAudioOutput::reconfigureInputFormat")
        << "  " << i << " -> " << channelMap[i] << std::endl;
        
        // Again, CoreAudio's channel mapping is the same as Ayane's so we can
        // directly determine the channel map.  In this case, i is the CoreAudio ABL
        // list index.
        d->mAudioBufferListWrapper->mBuffers[i].mChannel = static_cast<Channel>(1<<channelMap[i]);
    }

    delete channelMap;

    /* Clear the buffer queue. */
    d->mCurrentBuffer.reset();
    d->mBuffers.clear();

    /* Restart the graph. */
    result = AUGraphInitialize(d->mAUGraph);
    
    if(result != noErr) {
        ERROR_THIS("CoreAudioOutput::reconfigureSink")
        << "Couldn't intialize the graph." << std::endl;
        return false;
    }
    
    result = AUGraphStart(d->mAUGraph);

    if(result != noErr) {
        ERROR_THIS("CoreAudioOutput::reconfigureSink")
        << "Couldn't start the graph." << std::endl;
        return false;
    }

    INFO_THIS("CoreAudioOutput::reconfigureSink")
    << "Reconfiguration successful. "
    << format.sampleRate() << "Hz, Channels="
    << format.channelCount() << ", ChannelLayout="
    << std::hex << std::showbase << format.channels()
    << std::noshowbase << std::dec << std::endl;
    
    return true;
}




ClockProvider &CoreAudioOutput::clockProvider() {
    A_D(CoreAudioOutput);
    return d->mClockProvider;
}


OSStatus CoreAudioOutputPrivate::renderNotify(AudioUnitRenderActionFlags *ioActionFlags,
                                         const AudioTimeStamp *inTimeStamp,
                                         UInt32 inBusNumber,
                                         UInt32 inNumberFrames,
                                         AudioBufferList *ioData)
{
#pragma unused(inNumberFrames)
#pragma unused(inBusNumber)
#pragma unused(ioData)
    
    if( *ioActionFlags & kAudioUnitRenderAction_PreRender) {
        
        //
        // mHostTime:   Host timestamp in nanoseconds
        // mSampleTime: The sample count upto this point
        //
        // d(mHostTime)/10^9 == (mSampleTime/sampleRate)
        //

        // Our configured nominal clock period is mClockPeriod. Calculate the
        // delta between the mLastClockTickHostTime and mHostTime. If the delta
        // is larger than mClockPeriod, publish the delta to the clock provider
        // and update mLastClockTickHostTime.
        
        UInt64 delta = inTimeStamp->mHostTime - mLastClockTickHostTime;
        
        if (delta > mClockProvider.clockPeriod()) {
            
            mLastClockTickHostTime = inTimeStamp->mHostTime;
            double deltaInSeconds = ((double)delta / 1000000000.0);

            mClockProvider.publish(deltaInSeconds);
            
            /*
            if(mCurrentBuffer) {
                A_Q(CoreAudio);
                q->messageBus().publish(new ProgressMessage(mCurrentBuffer->timestamp()));
            }
             */
        }

	}
	else if( *ioActionFlags & kAudioUnitRenderAction_PostRender ) {
    }
    
    return noErr;
}



OSStatus CoreAudioOutputPrivate::render(AudioUnitRenderActionFlags *ioActionFlags,
                                   const AudioTimeStamp	 *inTimeStamp,
                                   UInt32 inBusNumber,
                                   UInt32 inNumberFrames,
                                   AudioBufferList *ioData)
{
#pragma unused(inTimeStamp)
#pragma unused(inBusNumber)
    
    // Update the buffer pointers for the RawBuffer wrapper. CoreAudio does not
    // guarantee the ioData buffer pointers will remain the same.
    for(UInt32 i = 0; i < ioData->mNumberBuffers; ++i) {
        mAudioBufferListWrapper->mBuffers[i].mBuffer = ioData->mBuffers[i].mData;
    }
    
    mAudioBufferListWrapper->mFrames = inNumberFrames;
    mAudioBufferListWrapper->reset();  // Beware of .reset(), this kills the unique_ptr!
        
    // Copy any left over data from the current buffer.
    if( mCurrentBuffer && (mCurrentBuffer->available() > 0) ) {
        *mCurrentBuffer >> *mAudioBufferListWrapper;
    }

    while( mAudioBufferListWrapper->space() > 0 ) {
        
        // Release the current buffer if it hasn't been released.
        if( mCurrentBuffer ) {
            mCurrentBuffer.reset();
        }
        
        // Try to get a new buffer.
        if( mBuffers.pop(&mCurrentBuffer) ) {
            (*mCurrentBuffer) >> (*mAudioBufferListWrapper);
        }
        else {
            
            // Going to want to throttle this...
            DEBUG_ONLY(WARNING_THIS("CoreAudioOutputPrivate::render") << "Underrun."
                       << std::endl);
            
            // If absolutely nothing was written, signal we're outputting silence.
            if( mAudioBufferListWrapper->mWriteIndex == 0 ) {
                *ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;
            }

            // Fill the remaining space in each buffer with 0.
            size_t byteCountToZero = mAudioBufferListWrapper->space() * sizeof(AudioUnitSampleType);
            
            for(UInt32 bufferIndex = 0;
                bufferIndex < ioData->mNumberBuffers;
                ++bufferIndex) {
                
                AudioUnitSampleType *bufferStart = (AudioUnitSampleType*)ioData->mBuffers[bufferIndex].mData;
                memset(bufferStart + mAudioBufferListWrapper->mWriteIndex, 0, byteCountToZero);
            }
            
            // Break here so we don't have to update mWriteIndex.
            break;
        }
    }
    
    return noErr;
}


