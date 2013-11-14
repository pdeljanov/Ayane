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

#include "coreaudioendpoint.h"

#include "audio/rawbuffer.h"

#include <iostream>

using namespace Stargazer::Audio;


namespace {
    
	// AUGraph input callback
	OSStatus auRenderCallback(void							*inRefCon,
                              AudioUnitRenderActionFlags    *ioActionFlags,
                              const AudioTimeStamp			*inTimeStamp,
                              UInt32                        inBusNumber,
                              UInt32						    inNumberFrames,
                              AudioBufferList				*ioData)
	{
        
        CoreAudioEndpoint *endpoint = static_cast<CoreAudioEndpoint*>(inRefCon);
        
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

        CoreAudioEndpoint *endpoint = static_cast<CoreAudioEndpoint*>(inRefCon);
        
		return endpoint->renderNotify(ioActionFlags, inTimeStamp, inBusNumber,
                                    inNumberFrames, ioData);
	}
    
}




CoreAudioEndpoint::CoreAudioEndpoint() : Stage(),
    mAUGraph(nullptr), mAUOutput(-1), mAUChannelLayout(nullptr), mBuffers(2)
{
    
    /*
     * The AU graph will always use the canonical Core Audio format since the 
     * buffers flowing through the audio engine are in an agnostic format.
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
    
    /*
     *  Setup Stage.
     */
    
    // Add output sink to stage.
    addSink("input");
}

CoreAudioEndpoint::~CoreAudioEndpoint() {

}


SharingMode CoreAudioEndpoint::sharingMode() const {
    
	AudioObjectPropertyAddress propertyAddress;
    
    propertyAddress.mSelector	= kAudioDevicePropertyHogMode;
    propertyAddress.mScope		= kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement    = kAudioObjectPropertyElementMaster;
    
	pid_t hogPID = static_cast<pid_t>(-1);
	UInt32 dataSize = sizeof(hogPID);
    
	AudioDeviceID deviceID;
	if(!outputDeviceID(deviceID)) {
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
    
	return ( hogPID == getpid() ? kExclusive : kShared );
}

bool CoreAudioEndpoint::setSharingMode(SharingMode mode)
{
    AudioDeviceID deviceID;
	if(!outputDeviceID(deviceID)) {
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
            return false;
        }
        
        // Set pid to us.
        pid = appPID;
    }
    
	bool restartIO = isOutputRunning();
	if(restartIO) {
		stopOutput();
    }
    
	result = AudioObjectSetPropertyData(deviceID,
                                        &propertyAddress,
                                        0,
                                        nullptr,
                                        sizeof(pid),
                                        &pid);
    
	if(kAudioHardwareNoError != result) {
		return false;
	}
    
	if(restartIO && !isOutputRunning()) {
		startOutput();
    }
    
	return true;
}


bool CoreAudioEndpoint::createOutputDeviceUID(CFStringRef& deviceUID) const
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
		return nullptr;
	}

	return true;
}

bool CoreAudioEndpoint::setOutputDeviceUID(CFStringRef deviceUID)
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
			return false;
		}
        
	}
    
	if(kAudioDeviceUnknown == deviceID) {
		return false;
    }
    
	return setOutputDeviceID(deviceID);
}

bool CoreAudioEndpoint::outputDeviceID(AudioDeviceID& deviceID) const
{
	AudioUnit au = nullptr;
	OSStatus result = AUGraphNodeInfo(mAUGraph, mAUOutput, nullptr, &au);
    
	if(result != noErr) {
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
		return false;
	}
    
	return true;
}

bool CoreAudioEndpoint::setOutputDeviceID(AudioDeviceID deviceID)
{
	if(kAudioDeviceUnknown == deviceID)
		return false;
    
	AudioUnit au = nullptr;
	OSStatus result = AUGraphNodeInfo(mAUGraph, mAUOutput, nullptr, &au);
    
	if(result != noErr) {
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
		return false;
	}
    
	return true;
}

bool CoreAudioEndpoint::outputDeviceSampleRate(Float64& sampleRate) const
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
		return false;
	}
    
	return true;
}

bool CoreAudioEndpoint::setOutputDeviceSampleRate(Float64 sampleRate)
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
		return false;
	}
    
	return true;
}



bool CoreAudioEndpoint::openOutput() {
    
    OSStatus result = NewAUGraph(&mAUGraph);
    
	if(result != noErr) {
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
        
		result = DisposeAUGraph(mAUGraph);
        
		if(result != noErr) {
            // TODO: Log.
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
        
        // TODO: Log.
        
		if(DisposeAUGraph(mAUGraph) != noErr){
            // TODO: Log.
        }
        
		mAUGraph = nullptr;
		return false;
	}
    
    
    // Get the audio output unit.
    AudioUnit au = nullptr;
	result = AUGraphNodeInfo(mAUGraph, mAUOutput, nullptr, &au);
    
    if(result != noErr) {
        
        // TODO: Log.
        
		if(DisposeAUGraph(mAUGraph) != noErr){
            // TODO: Log.
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
        
        // TODO: Log.
        
		if(DisposeAUGraph(mAUGraph) != noErr){
            // TODO: Log.
        }
        
		mAUGraph = nullptr;
		return false;
	}
    
    // Install the graph render notification used to drive the clock.
	result = AUGraphAddRenderNotify(mAUGraph, auGraphRenderNotify, this);
    
	if(result != noErr) {
        
        // TODO: Log.
        
		if(DisposeAUGraph(mAUGraph) != noErr){
            // TODO: Log.
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
        
        // TODO: Log.
        
		if(DisposeAUGraph(mAUGraph) != noErr){
            // TODO: Log.
        }
        
		mAUGraph = nullptr;
		return false;
	}
    

    // Initialize the graph
	result = AUGraphInitialize(mAUGraph);
    
	if(result != noErr) {
        
        // TODO: Log.
        
		if(DisposeAUGraph(mAUGraph) != noErr){
            // TODO: Log.
        }
        
		mAUGraph = nullptr;
		return false;
	}

    return true;
}



bool CoreAudioEndpoint::closeOutput()
{
	Boolean graphIsRunning = false;
	OSStatus result = AUGraphIsRunning(mAUGraph, &graphIsRunning);
    
	if(result != noErr) {
		return false;
	}
    
	if(graphIsRunning) {
		result = AUGraphStop(mAUGraph);
		if(result != noErr) {
			return false;
		}
	}
    
	Boolean graphIsInitialized = false;
	result = AUGraphIsInitialized(mAUGraph, &graphIsInitialized);
	if(result != noErr) {
		return false;
	}
    
	if(graphIsInitialized) {
		result = AUGraphUninitialize(mAUGraph);
		if(result != noErr) {
			return false;
		}
	}
    
	result = AUGraphClose(mAUGraph);
	if(result != noErr) {
		return false;
	}
    
	result = DisposeAUGraph(mAUGraph);
	if(result != noErr) {
		return false;
	}
    
	mAUGraph = nullptr;
	mAUOutput = -1;

	return true;
}

bool CoreAudioEndpoint::startOutput()
{
	// We don't want to start output in the middle of a buffer modification
	/*std::unique_lock<std::mutex> lock(mMutex, std::try_to_lock);
	if(!lock)
		return false;
    */
    
	OSStatus result = AUGraphStart(mAUGraph);
	if(result != noErr) {
		return false;
	}
	
	return true;
}

bool CoreAudioEndpoint::stopOutput()
{
	OSStatus result = AUGraphStop(mAUGraph);
	if(result != noErr) {
		return false;
	}

	return true;
}

bool CoreAudioEndpoint::isOutputRunning() const
{
	Boolean isRunning = false;
	OSStatus result = AUGraphIsRunning(mAUGraph, &isRunning);
	if(result != noErr) {
		return false;
	}

	return isRunning;
}

bool CoreAudioEndpoint::resetOutput()
{
    
	UInt32 nodeCount = 0;
	OSStatus result = AUGraphGetNodeCount(mAUGraph, &nodeCount);
	if(result != noErr) {
		return false;
	}
    
	for(UInt32 i = 0; i < nodeCount; ++i) {
        
		AUNode node = 0;
        
		result = AUGraphGetIndNode(mAUGraph, i, &node);
        
		if(result != noErr) {
			return false;
		}
        
		AudioUnit au = nullptr;
        
		result = AUGraphNodeInfo(mAUGraph, node, nullptr, &au);
        
		if(result != noErr) {
			return false;
		}
        
		result = AudioUnitReset(au, kAudioUnitScope_Global, 0);
        
		if(result != noErr) {
			return false;
		}
	}
    
	return true;
}


bool CoreAudioEndpoint::saveGraphInteractions(std::vector<AUNodeInteraction> &interactions ) {
    
    UInt32 interactionCount = 0;
    
	OSStatus result = AUGraphGetNumberOfInteractions(mAUGraph, &interactionCount);
    
	if(result != noErr) {
		return false;
	}

    interactions.reserve(interactionCount);
    
    for(UInt32 i = 0; i < interactionCount; ++i) {

		result = AUGraphGetInteractionInfo(mAUGraph, i, &interactions[i]);
        
		if(result != noErr) {
			return false;
		}
        
	}
    return true;
}

bool CoreAudioEndpoint::restoreGraphInteractions(std::vector<AUNodeInteraction> &interactions) {
    
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
					return false;
				}
                
				break;
			}
		}
	}
    
    return true;
    
}


bool CoreAudioEndpoint::setPropertyOnAUGraphNodes(AudioUnitPropertyID propertyID,
                                                  const void *propertyData,
                                                  UInt32 propertyDataSize)
{
    
	if(nullptr == propertyData || 0 >= propertyDataSize)
		return  false;
    
	UInt32 nodeCount = 0;
    
	OSStatus result = AUGraphGetNodeCount(mAUGraph, &nodeCount);
    
	if(result != noErr) {
		return false;
	}
    
	// Iterate through the nodes and attempt to set the property
	for(UInt32 i = 0; i < nodeCount; ++i) {
        
		AUNode node;
        
		result = AUGraphGetIndNode(mAUGraph, i, &node);
        
		if(result != noErr) {
			return false;
		}
        
		AudioUnit au = nullptr;
        
		result = AUGraphNodeInfo(mAUGraph, node, nullptr, &au);
        
		if(result != noErr) {
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
					return false;
				}
			}
		}
	}
    
	return true;
}


bool CoreAudioEndpoint::setAUGraphSampleRateAndChannelLayout(Float64 sampleRate, UInt32 channelsPerFrame) {
    
	OSStatus result;
    
	// ========================================
	// Save the interaction information and then clear all the connections

    std::vector<AUNodeInteraction> interactions;
    if(!saveGraphInteractions(interactions)) {
        return false;
    }
    
	result = AUGraphClearConnections(mAUGraph);
	if(result != noErr) {
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
            // TODO: Log
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

bool CoreAudioEndpoint::setAUOutputChannelLayout(AudioChannelLayout *channelLayout) {
    
    // Do nothing if channelLayout is null.
    if(channelLayout == nullptr) {
		return true;
    }
    
	AudioUnit outputUnit = nullptr;
    
	OSStatus result = AUGraphNodeInfo(mAUGraph, mAUOutput, nullptr, &outputUnit);
    
	if(result != noErr) {
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
		return false;
	}
    

    // Test if the output channel layout is Stereo.
	AudioChannelLayout stereoChannelLayout;
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
        // TODO: Log.
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
			return false;
		}
        
        SInt32 *channelMap = new SInt32[outputFormat.mChannelsPerFrame];
        
		for(UInt32 i = 0; i <  outputFormat.mChannelsPerFrame; ++i) {
			channelMap[i] = -1;
        }
        
		channelMap[preferredChannelsForStereo[0] - 1] = 0;
		channelMap[preferredChannelsForStereo[1] - 1] = 1;
        
        /*
		LOGGER_DEBUG("org.sbooth.AudioEngine.Player", "Using  stereo channel map: ");
		for(UInt32 i = 0; i < outputFormat.mChannelsPerFrame; ++i) {
			LOGGER_DEBUG("org.sbooth.AudioEngine.Player", "  " << i << " -> " << channelMap[i]);
        }
        */
        
		// Set the channel map
		result = AudioUnitSetProperty(outputUnit,
                                      kAudioOutputUnitProperty_ChannelMap,
                                      kAudioUnitScope_Input,
                                      0,
                                      channelMap,
                                      (UInt32)sizeof(channelMap));
        
        delete channelMap;
        
		if(result != noErr) {
			return false;
		}
        
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
			return false;
		}
        
        // Allocate an array for the device's preferred channel list.
        std::unique_ptr<AudioChannelLayout> devicePreferredChannelLayout(
            static_cast<AudioChannelLayout*>(malloc(devicePreferredChannelLayoutSize)));
        
		result = AudioUnitGetProperty(outputUnit,
                                      kAudioDevicePropertyPreferredChannelLayout,
                                      kAudioUnitScope_Output,
                                      0,
                                      devicePreferredChannelLayout.get(),
                                      &devicePreferredChannelLayoutSize);
        
		if(result != noErr) {
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
			return false;
		}
        
		// Create the channel map.
        std::unique_ptr<SInt32> channelMap( new SInt32[channelCount] );
		dataSize = (UInt32)sizeof(channelMap);
        
		AudioChannelLayout *channelLayouts [] = {
			channelLayout,
			devicePreferredChannelLayout.get()
		};
        
		result = AudioFormatGetProperty(kAudioFormatProperty_ChannelMap,
                                        sizeof(channelLayouts),
                                        channelLayouts,
                                        &dataSize,
                                        channelMap.get());

		if(result != noErr) {
			return false;
		}
        
        /*
		LOGGER_DEBUG("org.sbooth.AudioEngine.Player", "Using multichannel channel map: ");
		for(UInt32 i = 0; i < channelCount; ++i) {
			LOGGER_DEBUG("org.sbooth.AudioEngine.Player", "  " << i << " -> " << channelMap[i]);
        }
        */
        
		// Set the channel map
		result = AudioUnitSetProperty(outputUnit,
                                      kAudioOutputUnitProperty_ChannelMap,
                                      kAudioUnitScope_Input,
                                      0,
                                      channelMap.get(),
                                      (UInt32)sizeof(channelMap));
        
		if(result != noErr) {
			return false;
		}
	}
    
    return true;
}















AudioBufferList * CoreAudioEndpoint::allocateABL(UInt32 channelsPerFrame,
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







Stage::Sink *CoreAudioEndpoint::input()
{
    return mSinks["input"];
}

bool CoreAudioEndpoint::beginPlayback() {
    std::cout << "CoreAudioEndpoint::beginPlayback" << std::endl;
    
    // Open output.
    if( !openOutput() ) {
        
        std::cout << "CoreAudioEndpoint::beginPlayback: Could not open output."
        << std::endl;
        
        return false;
    }
    
    // Set default output device.
    CFStringRef device = nullptr;
    if( !setOutputDeviceUID(device) ) {
        
        std::cout << "CoreAudioEndpoint::beginPlayback: Failed to set default "
            "output device." << std::endl;
        
        return false;
    }
    
    // Don't actually start the device till a buffer is received.
    
    // Kick off the clock.
    mClockPeriod = 0.100;
    mCurrentClockTick = 0.0;
    mClockProvider.publish(mClockPeriod);

    return true;
}

bool CoreAudioEndpoint::stoppedPlayback() {
    std::cout << "CoreAudioEndpoint::stoppedPlayback" << std::endl;
    
    stopOutput();
    closeOutput();
    
    // Clear all processed buffers.
    mBuffers.clear();
    mAudioBufferListWrapper.reset();
    mCurrentBuffer.reset();
    
    // Reset the sink.
    input()->reset();

    return true;
}





void CoreAudioEndpoint::process( ProcessIOFlags *ioFlags ){
    
    std::unique_ptr<Buffer> buffer;

    // May kick off a sink reconfiguration.
    if( input()->isLinked() && (input()->pull(&buffer) == Sink::kSuccess) ) {
        
        // Pass ownership of buffer to the queue.
        mBuffers.push(buffer);
        
        // Hint that the CoreAudioEndpoint could process more buffers since its
        // internal ring buffer is not full.
        if(!mBuffers.full()) {
            (*ioFlags) |= kProcessMoreHint;
        }
    }
    
}

bool CoreAudioEndpoint::reconfigureIO() {
    std::cout << "CoreAudioEndpoint::reconfigureIO" << std::endl;
    return true;
}

bool CoreAudioEndpoint::reconfigureInputFormat(const Sink &sink,
                                               const BufferFormat &format){
    
#pragma unused(sink)
    
    std::cout << "CoreAudioEndpoint::reconfigureSink: Attempting reconfiguration."
    << std::endl;
    
    /* Stop and unitialiaze the AUGraph. */
	Boolean graphIsRunning = FALSE;
	OSStatus result = AUGraphIsRunning(mAUGraph, &graphIsRunning);
    
	if(result != noErr) {
        std::cout << "CoreAudioEndpoint::reconfigureSink: Couldn't get graph "
        << "running state." << std::endl;
        
		return false;
	}
	
	if(graphIsRunning) {
        
		result = AUGraphStop(mAUGraph);
        
		if(result != noErr) {
            std::cout << "CoreAudioEndpoint::reconfigureSink: Couldn't stop the "
            << "graph." << std::endl;
			return false;
		}
	}

	Boolean graphIsInitialized = FALSE;
	result = AUGraphIsInitialized(mAUGraph, &graphIsInitialized);
    
	if(result != noErr) {
        std::cout << "CoreAudioEndpoint::reconfigureSink: Couldn't get graph "
        << "initialization state." << std::endl;
        
		return false;
	}
	
	if(graphIsInitialized) {
        
		result = AUGraphUninitialize(mAUGraph);
        
		if(result != noErr) {
            std::cout << "CoreAudioEndpoint::reconfigureSink: Couldn't "
            "unitialize the graph." << std::endl;
			return false;
		}
	}
    
    /* Reconfigure the graph. */
    if(!setAUGraphSampleRateAndChannelLayout(format.sampleRate(),
                                             format.channelCount())) {
        
        std::cout << "CoreAudioEndpoint::reconfigureSink: Setting the sample "
            "rate and number of channels failed." << std::endl;
        
        return false;
    }
    
    // TODO: Add more channel layouts.
    // Stereo channel layout.
    int numberChannelDescriptions = 0;
    
    size_t layoutSize = offsetof(AudioChannelLayout, mChannelDescriptions) +
        (numberChannelDescriptions * sizeof(AudioChannelDescription));
    
    AudioChannelLayout *channelLayout = (AudioChannelLayout *)malloc(layoutSize);
    memset(channelLayout, 0, layoutSize);
    
    channelLayout->mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
    
    if( !setAUOutputChannelLayout(channelLayout) ){
        
        std::cout << "CoreAudioEndpoint::reconfigureSink: Setting the channel "
            "layout failed." << std::endl;
        
        return false;
    }
    
    /* Update the raw buffer */
    mAudioBufferListWrapper.reset(new RawBuffer(mMaxFramesPerSlice,
                                                format.channelCount(),
                                                kFloat32,
                                                true));
    
    mAudioBufferListWrapper->mBuffers[0].mChannel = kFrontLeft;
    mAudioBufferListWrapper->mBuffers[1].mChannel = kFrontRight;
    
    
    /* Update the clock. */
    mPeriodPerFrame = (1.0 / format.sampleRate());
    
    
    /* Clear the buffer queue. */
    mCurrentBuffer.reset();
    mBuffers.clear();

    /* Restart the graph. */
    result = AUGraphInitialize(mAUGraph);
    
    if(result != noErr) {
        std::cout << "CoreAudioEndpoint::reconfigureSink: Couldn't intialize "
        "the graph." << std::endl;
        return false;
    }
    
    result = AUGraphStart(mAUGraph);

    if(result != noErr) {
        std::cout << "CoreAudioEndpoint::reconfigureSink: Couldn't start the "
        "graph." << std::endl;
        return false;
    }

    std::cout<<"CoreAudioEndpoint::reconfigureSink: Reconfiguration successful."
    " SampleRate=" << format.sampleRate() <<
    ", ChannelCount=" << format.channelCount() <<
    ", ChannelMap=" << format.channels() << std::endl;
    
    return true;
}




ClockProvider &CoreAudioEndpoint::clockProvider() {
    return mClockProvider;
}


OSStatus CoreAudioEndpoint::renderNotify(AudioUnitRenderActionFlags *ioActionFlags,
                                         const AudioTimeStamp *inTimeStamp,
                                         UInt32 inBusNumber,
                                         UInt32 inNumberFrames,
                                         AudioBufferList *ioData)
{
#pragma unused(inTimeStamp)
#pragma unused(inBusNumber)
#pragma unused(ioData)
    
    if( *ioActionFlags & kAudioUnitRenderAction_PreRender) {
        
        mCurrentClockTick += (inNumberFrames * mPeriodPerFrame);
        
        if( mCurrentClockTick >= mClockPeriod ){
            mClockProvider.publish(mCurrentClockTick);
            mCurrentClockTick = 0.0;
        }

        
	}
	else if( *ioActionFlags & kAudioUnitRenderAction_PostRender ) {
        
    }
    
    return noErr;
}



OSStatus CoreAudioEndpoint::render(AudioUnitRenderActionFlags *ioActionFlags,
                                   const AudioTimeStamp	 *inTimeStamp,
                                   UInt32 inBusNumber,
                                   UInt32 inNumberFrames,
                                   AudioBufferList *ioData)
{
#pragma unused(inTimeStamp)
#pragma unused(inBusNumber)
    
    // Update the RawBuffer wrapper to point to the new AudioBufferList buffers.
    mAudioBufferListWrapper->mBuffers[0].mBuffer = ioData->mBuffers[0].mData;
    mAudioBufferListWrapper->mBuffers[1].mBuffer = ioData->mBuffers[1].mData;
    mAudioBufferListWrapper->mFrames = inNumberFrames;
    mAudioBufferListWrapper->reset();
    
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
            
            // If absolutely nothing was written, signal we're outputting silence.
            if( mAudioBufferListWrapper->mReadIndex == 0 ) {
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
            
            // Break out here because space() won't be updated.
            break;
        }
    }
    
    return noErr;
}


