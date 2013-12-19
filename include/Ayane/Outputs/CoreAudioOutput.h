/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_OUTPUTS_COREAUDIOOUTPUT_H_
#define AYANE_OUTPUTS_COREAUDIOOUTPUT_H_

#include "Ayane/DPointer.h"
#include "Ayane/ClockProvider.h"
#include "Ayane/Stage.h"

namespace Ayane {
    
    typedef enum {
        
        /** The audio device is shared among many applications. */
        kShared = 0,
        
        /** The audio device is exclusive to the current application. */
        kExclusive
        
    } SharingMode;
    
    
    class CoreAudioOutputPrivate;
    
    /**
     *  A CoreAudioOutput uses Apple's CoreAudio to playback audio.
     */
    class CoreAudioOutput : public Stage {
        
    public:
        
        CoreAudioOutput();
        ~CoreAudioOutput();
        
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
        bool setSharingMode(SharingMode mode);
        
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
        AYANE_DISALLOW_COPY_AND_ASSIGN(CoreAudioOutput);
        
        CoreAudioOutputPrivate *d_ptr;
        AYANE_DECLARE_PRIVATE(CoreAudioOutput);
    };
    
}

#endif