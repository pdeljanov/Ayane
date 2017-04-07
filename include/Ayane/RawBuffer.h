#include <assert.h>
/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_RAWBUFFER_H_
#define AYANE_RAWBUFFER_H_

#include <array>
#include <algorithm>
#include <type_traits>

#include "Ayane/Macros.h"
#include "Ayane/SampleFormats.h"
#include "Ayane/Channels.h"

namespace Ayane {
    
    class Buffer;

    /**
     * RawBufferFormat describes the contents of a plain C-style buffer.
     */
    class RawBufferFormat {

    public:
        RawBufferFormat(SampleFormat format) :
            mSampleFormat(format),
            mChannelCount(0)
        {
            mChannels.fill(kInvalid);
        }

        inline RawBufferFormat& withChannel(Channel channel) {
            assert(mChannelCount < mChannels.size());

            mChannels[mChannelCount++] = channel;
            return *this;
        }

        inline Channel channel(uint32_t index){
            assert(index < mChannelCount);

            return mChannels[index];
        }

        inline uint32_t channels() const {
            return mChannelCount;
        }

        inline SampleFormat sampleFormat() const {
            return mSampleFormat;
        }

    private:
        /** The sample format of the raw buffer. */
        SampleFormat mSampleFormat;

        /** The number of channels. */
        uint32_t mChannelCount;

        /** Buffer descriptors. */
        std::array<Channel, kMaximumChannels> mChannels;
    };

    class RawBuffer
    {
        template<typename T>
        friend class TypedBuffer;
        
    public:

        RawBuffer(SampleFormat format, uint32_t frames) :
            mFormat(format),
            mFrames(frames),
            mReadIndex(0),
            mWriteIndex(0),
            mIsPlanar(false)
        {
            mBuffers.fill(nullptr);
        }

        RawBuffer(const RawBufferFormat& format, uint32_t frames, void* buffer) :
            mFormat(format),
            mFrames(frames),
            mReadIndex(0),
            mWriteIndex(frames),
            mIsPlanar(false)
        {
            mBuffers.fill(nullptr);
            mBuffers[0] = buffer;
        }

        RawBuffer(const RawBufferFormat& format, uint32_t frames, void** buffers, uint32_t count) :
            mFormat(format),
            mFrames(frames),
            mReadIndex(0),
            mWriteIndex(frames),
            mIsPlanar(true)
        {
            assert(count == format.channels());
            uint32_t i = 0;
            while(i < count) {
                mBuffers[i] = buffers[i];
                i++;
            }
        }

        template<class InputIt>
        RawBuffer(const RawBufferFormat& format, uint32_t frames, InputIt begin, InputIt end) :
            mFormat(format),
            mFrames(frames),
            mReadIndex(0),
            mWriteIndex(frames),
            mIsPlanar(true)
        {
            static_assert(std::is_same<typename std::iterator_traits<InputIt>::value_type, void*>::value,
               "Iterator must be to a C-Style (void*) buffer.");

            uint32_t i = 0;
            while(begin != end && i < mBuffers.size()) {
                mBuffers[i++] = *begin++;
            }

            assert(i == format.channels());
        }

        RawBuffer& planar() {
            mIsPlanar = true;
            return *this;
        }

        RawBuffer& interleaved(void* buffer) {
            assert(nullptr != buffer);

            mIsPlanar = false;
            mBuffers[0] = buffer;
            return *this;
        }

        RawBuffer& withChannel(Channel channel) {
            assert(false == mIsPlanar);

            mFormat.channel(channel);

            return *this;
        }

        RawBuffer& withChannel(Channel channel, void* buffer) {
            assert(nullptr != buffer);
            assert(true == mIsPlanar);

            mBuffers[mFormat.channels()] = buffer;
            mFormat.channel(channel);

            return *this;
        }

        RawBuffer& populate(uint32_t used) {
            mWriteIndex = used;
            return *this;
        }

        inline uint32_t channels() const {
            return mFormat.channels();
        }

        inline Channel channel(uint32_t index) {
            return mFormat.channel(index);
        }

        // Buffer Layout:
        //
        // [   read#   |      readable#      |       writeable#      ]
        //             r                     w
        //

        /** Gets the total number of frames the buffer can store. */
        inline uint32_t frames() const {
            return mFrames;
        }

        /** Gets the number of frames read from the buffer. */
        inline uint32_t framesRead() const {
            return mReadIndex;
        }

        /** Gets the number of frames written to the buffer. */
        inline uint32_t framesWritten() const {
            return mWriteIndex;
        }

        /** Gets the number of frames that still may be read from the buffer. */
        inline uint32_t readable() const {
            return mWriteIndex - mReadIndex;
        }

        /** Gets the number of frames that still can be written to the buffer. */
        inline uint32_t writeable() const {
            return mFrames - mWriteIndex;
        }

        inline void consume(uint32_t consumed) {
            assert(consumed <= readable());
            mReadIndex += consumed;
        }

        inline void fill(uint32_t filled) {
            assert(filled <= writeable());
            mWriteIndex += filled;
        }

        /**
         * Resets the read and write positions to clear the buffer such that the full capacity
         * is available for writing, and nothing is available for reading.
         */
        void rewind() {
            mReadIndex = 0;
            mWriteIndex = 0;
        }
        
        /** Gets the data stride of one frame. */
        inline std::size_t stride() const {
            return mIsPlanar ? 1 : mFormat.channels();
        }

        /** Gets the sample format. */
        inline SampleFormat format() const {
            return mFormat.sampleFormat();
        }

    private:

        /**
         *  Returns a pointer to be used for reading from the raw buffer.
         */
        template<typename OutSampleType>
        OutSampleType *readAs( unsigned int channel ){
            assert(channel < mFormat.channels());

            OutSampleType *base = nullptr;
            
            if(mIsPlanar) {
                base = reinterpret_cast<OutSampleType*>(mBuffers[channel]);
                base += mReadIndex;
            }
            else {
                base = reinterpret_cast<OutSampleType*>(mBuffers[0]) + channel;
                base += (mReadIndex * stride());
            }
            
            return base;
        }
        
        /**
         *  Returns a pointer to be used for writing from the raw buffer.
         */
        template<typename OutSampleType>
        OutSampleType *writeAs( unsigned int channel ){
            assert(channel < mFormat.channels());

            OutSampleType *base = nullptr;
            
            if(mIsPlanar) {
                base = reinterpret_cast<OutSampleType*>(mBuffers[channel]);
                base += mWriteIndex;
            }
            else {
                base = reinterpret_cast<OutSampleType*>(mBuffers[0]) + channel;
                base += (mWriteIndex * stride());
            }
            
            return base;
        }
        
        RawBuffer& operator>> (Buffer& rhs);
        
        RawBuffer& operator<< (Buffer& rhs);
        
        
    private:
        AYANE_DISALLOW_DEFAULT_CTOR_COPY_AND_ASSIGN(RawBuffer);

        /** Format descriptor. */
        RawBufferFormat mFormat;

        /** The number of frames the raw buffer can store. */
        uint32_t mFrames;
        
        /** The number of frames read. */
        uint32_t mReadIndex;
        
        /** The number of frames written. */
        uint32_t mWriteIndex;

        /** True if the data layout is planar, false otherwise. */
        bool mIsPlanar;

        /** Raw buffers. */
        std::array<void*, kMaximumChannels> mBuffers;
    };
    
}

#endif
