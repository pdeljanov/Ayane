/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_AUDIO_HOST_MAC_COREAUDIOENDPOINT_H_
#define AYANE_AUDIO_HOST_MAC_COREAUDIOENDPOINT_H_

#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioToolbox.h>

#include "clockprovider.h"
#include "rawbuffer.h"
#include "stage.h"

#include <vector>
#include <memory>

namespace Ayane {
    
    typedef enum {
        
        kShared = 0,
        kExclusive
        
    } SharingMode;
    
    
    class CoreAudioEndpoint : public Stage {
        
    public:
        
        CoreAudioEndpoint();
        ~CoreAudioEndpoint();
        
        /**
         *  Gets the output sharing mode.
         */
        SharingMode sharingMode() const;
        
        /**
         *  Attempts to set the audio device sharing mode.
         *
         *  \returns True if setting the sharing mode was succesful, false
         *           otherwise.
         */
        bool setSharingMode( SharingMode mode );
        
        
        Sink *input();
        
        ClockProvider &clockProvider();
        
    private:
        AYANE_DISALLOW_COPY_AND_ASSIGN(CoreAudioEndpoint);
        
        class BufferingCriteria;
        
        virtual bool beginPlayback();
        virtual bool stoppedPlayback();
        virtual void process(ProcessIOFlags *ioFlags);
        virtual bool reconfigureIO();
        virtual bool reconfigureInputFormat(const Sink &sink,
                                            const BufferFormat &format );
        
        
        
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
        
        bool isOutputRunning() const;
        
        bool resetOutput();
        
        
        
        
        bool saveGraphInteractions( std::vector<AUNodeInteraction> &interactions );
        bool restoreGraphInteractions( std::vector<AUNodeInteraction> &interactions);
        
        bool setPropertyOnAUGraphNodes(AudioUnitPropertyID propertyID,
                                       const void *propertyData,
                                       UInt32 propertyDataSize);
        
        bool setAUGraphSampleRateAndChannelLayout(Float64 sampleRate,
                                                  UInt32 channelsPerFrame);
        
        bool setAUOutputChannelLayout(AudioChannelLayout *channelLayout);
        
        AudioBufferList *allocateABL(UInt32 channelsPerFrame,
                                     UInt32 bytesPerSample,
                                     bool interleaved,
                                     UInt32 capacityFrames);
        
        
        // Audio Unit graph
        AUGraph mAUGraph;
        
        // Output node.
        AUNode mAUOutput;
        
        // Audio format for the AU graph.
        AudioStreamBasicDescription			mAUFormat;
        
        // Channel layout for the AU graph.
        std::unique_ptr<AudioChannelLayout>	mAUChannelLayout;
        
        UInt32 mMaxFramesPerSlice;
        
        double mPeriodPerFrame;
        double mClockPeriod;
        double mNextClockTick;
        double mCurrentClockTick;
        ClockProvider mClockProvider;
        
        BufferQueue mBuffers;
        
        std::unique_ptr<RawBuffer> mAudioBufferListWrapper;
        ManagedBuffer mCurrentBuffer;
        
    public:
        
        // AUNode render callback.
        OSStatus render(AudioUnitRenderActionFlags		*ioActionFlags,
                        const AudioTimeStamp			    *inTimeStamp,
                        UInt32							 inBusNumber,
                        UInt32							 inNumberFrames,
                        AudioBufferList					*ioData);
        
        // AUGraph render notification.
        OSStatus renderNotify(AudioUnitRenderActionFlags *ioActionFlags,
                              const AudioTimeStamp		 *inTimeStamp,
                              UInt32                      inBusNumber,
                              UInt32						  inNumberFrames,
                              AudioBufferList            *ioData);
        
    };
    
}

#endif