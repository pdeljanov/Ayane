/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "buffer.h"
#include "rawbuffer.h"

#include <core/alignedmemory.h>

#include <algorithm>

using namespace Stargazer::Audio;

Buffer::Buffer ( const BufferFormat &format, const BufferLength &length ) :
    mFormat(format),
    mLength(length),
    mTimestamp(0),
    mFlags(kNone),
    mWriteIndex(0),
    mReadIndex(0)
{
}

Buffer::~Buffer()
{
}

Duration Buffer::duration() const
{
    return Duration ( mLength.duration ( mFormat.mSampleRate ) );
}

const Duration& Buffer::timestamp() const
{
    return mTimestamp;
}

void Buffer::setTimestamp ( const Duration& timestamp )
{
    mTimestamp = timestamp;
}

const BufferFormat& Buffer::format() const
{
    return mFormat;
}

unsigned int Buffer::frames() const
{
    return mLength.frames ( mFormat.mSampleRate );
}

unsigned int Buffer::available() const {
    // Frames that can be read!
    return mWriteIndex - mReadIndex;
}

unsigned int Buffer::space() const {
    // Frames that can be written!
    return frames() - mWriteIndex;
}

void Buffer::reset() {
    mWriteIndex = 0;
    mReadIndex = 0;
    mFlags = kNone;
    mTimestamp = 0;
}

template<typename T>
TypedBuffer<T>::TypedBuffer( const BufferFormat &format, const BufferLength &length ) :
    Buffer( format, length )
{
    // Calculate the number of frames in the buffer.
    unsigned int frames = length.frames( format.sampleRate() );
    
    // Calculate the number of actual samples the buffer must store.
    unsigned int samples = frames * format.channelCount();
    
    // Allocate the buffer with 16 byte alignment.
    T *buffer = AlignedMemory::allocate16<T>(samples);
    
    // Build the channel map.
    buildChannelMap(mChannels, format.channels(), buffer, frames);
}

template<typename T>
TypedBuffer<T>::~TypedBuffer()
{
    // Deallocate the buffer.
    AlignedMemory::deallocate(mChannels[0]);
}

template<typename T>
void TypedBuffer<T>::buildChannelMap( ChannelMap map, Channels channels, T* base, unsigned int stride )
{
    /* m_bufs is an array of pointers. Each pointer points to an address within base. These addresses
     * form the start addresses of the channel buffers. Channel pointers are stored in the canonical channel
     * ordering. If a channel is not used, m_bufs for that channel index is null. The first pointer in
     * m_bufs *must* be base as ChannelMap takes ownership of the memory.
     */
    
    channels &= kChannelMask;
    
    // map[0] must *always* be a pointer to the start of the buffer.
    map[0] = base;
    
    int i = 0;  // Channel we're testing.
    int j = 0;  // Previous buffer index set to an address other than null.
    
    while( channels )
    {
        if( channels & CanonicalChannels::get(i) )
        {
            map[i] = (i > 0) ? (map[j] + stride) : base;
            channels ^= CanonicalChannels::get(i);
            j = i;
        }
        else
        {
            map[i] = nullptr;
        }
        
        ++i;
    }
}

/* Buffer traits to support template type -> enum mapping. */
template< typename T >
struct TypedBufferTraits
{ static SampleFormat sampleFormat(); };

template <>
struct TypedBufferTraits<SampleInt16>
{ static SampleFormat sampleFormat(){ return kInt16; } };

template <>
struct TypedBufferTraits<SampleInt32>
{ static SampleFormat sampleFormat(){ return kInt32; } };

template <>
struct TypedBufferTraits<SampleFloat32>
{ static SampleFormat sampleFormat(){ return kFloat32; } };

template <>
struct TypedBufferTraits<SampleFloat64>
{ static SampleFormat sampleFormat(){ return kFloat64; } };

template<typename T>
SampleFormat TypedBuffer<T>::sampleFormat() const
{ return TypedBufferTraits<T>::sampleFormat(); }


/* Read(...) Functions */

template<typename T>
template<typename OutSampleType>
force_inline void TypedBuffer<T>::readChannel( Channel ch, T &is, OutSampleType &os )
{
    if( mFormat.channels() & ch )
        os = SampleFormats::convertSample<T, OutSampleType>(is);
}

template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( Mono<OutSampleType> &i )
{
    i.FC = SampleFormats::convertSample<T, OutSampleType>(mChannels[0][mReadIndex]);
    ++mReadIndex;
}

template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( Stereo<OutSampleType> &i )
{
    i.FL = SampleFormats::convertSample<OutSampleType, T>(mChannels[0][mReadIndex]);
    i.FR = SampleFormats::convertSample<OutSampleType, T>(mChannels[1][mReadIndex]);
    ++mReadIndex;
}

template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( Stereo21<OutSampleType> &i )
{
    i.FL  = SampleFormats::convertSample<OutSampleType, T>(mChannels[0][mReadIndex]);
    i.FR  = SampleFormats::convertSample<OutSampleType, T>(mChannels[1][mReadIndex]);
    i.LFE = SampleFormats::convertSample<OutSampleType, T>(mChannels[3][mReadIndex]);
    ++mReadIndex;
}



template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( MultiChannel3<OutSampleType> &i )
{
    i.FL = SampleFormats::convertSample<OutSampleType, T>(mChannels[0][mReadIndex]);
    i.FR = SampleFormats::convertSample<OutSampleType, T>(mChannels[1][mReadIndex]);
    
    readChannel(kFrontCenter,     mChannels[2][mReadIndex], i.FC);
    readChannel(kLowFrequencyOne, mChannels[3][mReadIndex], i.LFE);
    readChannel(kBackCenter,      mChannels[8][mReadIndex], i.BC);
    
    ++mReadIndex;
}

template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( MultiChannel4<OutSampleType> &i )
{
    i.FL = SampleFormats::convertSample<OutSampleType, T>(mChannels[0][mReadIndex]);
    i.FR = SampleFormats::convertSample<OutSampleType, T>(mChannels[1][mReadIndex]);
    
    readChannel(kFrontCenter,     mChannels[2][mReadIndex], i.FC);
    readChannel(kLowFrequencyOne, mChannels[3][mReadIndex], i.LFE);
    readChannel(kBackCenter,      mChannels[8][mReadIndex], i.BC);
    readChannel(kBackLeft,        mChannels[4][mReadIndex], i.BL);
    readChannel(kBackRight,       mChannels[5][mReadIndex], i.BR);
    
    ++mReadIndex;
}

template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( MultiChannel5<OutSampleType> &i )
{
    i.FL = SampleFormats::convertSample<OutSampleType, T>(mChannels[0][mReadIndex]);
    i.FR = SampleFormats::convertSample<OutSampleType, T>(mChannels[1][mReadIndex]);
    
    readChannel(kFrontCenter,     mChannels[2][mReadIndex],  i.FC);
    readChannel(kLowFrequencyOne, mChannels[3][mReadIndex], i.LFE);
    readChannel(kBackLeft,        mChannels[4][mReadIndex],  i.BL);
    readChannel(kBackRight,       mChannels[5][mReadIndex],  i.BR);
    readChannel(kSideLeft,        mChannels[9][mReadIndex],  i.SL);
    readChannel(kSideRight,       mChannels[10][mReadIndex], i.SR);
    
    ++mReadIndex;
}

template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( MultiChannel6<OutSampleType> &i )
{
    i.FL = SampleFormats::convertSample<OutSampleType, T>(mChannels[0][mReadIndex]);
    i.FR = SampleFormats::convertSample<OutSampleType, T>(mChannels[1][mReadIndex]);
    
    readChannel(kFrontCenter,     mChannels[2][mReadIndex],  i.FC);
    readChannel(kLowFrequencyOne, mChannels[3][mReadIndex],  i.LFE);
    readChannel(kBackLeft,        mChannels[4][mReadIndex],  i.BL);
    readChannel(kBackRight,       mChannels[5][mReadIndex],  i.BR);
    readChannel(kBackCenter,      mChannels[8][mReadIndex],  i.BC);
    readChannel(kSideLeft,        mChannels[9][mReadIndex],  i.SL);
    readChannel(kSideRight,       mChannels[10][mReadIndex], i.SR);
    
    ++mReadIndex;
}

template< typename T >
template< typename OutSampleType >
void TypedBuffer<T>::read( MultiChannel7<OutSampleType> &i )
{
    i.FL = SampleFormats::convertSample<OutSampleType, T>(mChannels[0][mReadIndex]);
    i.FR = SampleFormats::convertSample<OutSampleType, T>(mChannels[1][mReadIndex]);
    
    readChannel(kFrontCenter,        mChannels[2][mReadIndex],  i.FC);
    readChannel(kLowFrequencyOne,    mChannels[3][mReadIndex],  i.LFE);
    readChannel(kBackLeft,           mChannels[4][mReadIndex],  i.BL);
    readChannel(kBackRight,          mChannels[5][mReadIndex],  i.BR);
    readChannel(kFrontLeftOfCenter,  mChannels[6][mReadIndex],  i.FLc);
    readChannel(kFrontRightOfCenter, mChannels[7][mReadIndex],  i.FRc);
    readChannel(kSideLeft,           mChannels[9][mReadIndex],  i.SL);
    readChannel(kSideRight,          mChannels[10][mReadIndex], i.SR);
    
    ++mReadIndex;
}

template< typename T >
template< typename OutSampleType >
void TypedBuffer<T>::read(TypedBuffer<OutSampleType> &buffer)
{
    // Compatability check first. Buffers must be equal length in frames, and sample
    // rate. Required, or else resampling will need to be performed.
    if((buffer.mFormat.sampleRate() != mFormat.sampleRate()) ||
       (buffer.frames() != frames())
       )
    {
        // TODO: Raise an exception?
        return;
    }
    
    // Determine the channels that need to be copied (union of the channel layouts).
    // Apply the channel mask to prevent crash-causing inputs.
    Channels channels = (buffer.mFormat.channels() & mFormat.channels()) & kChannelMask;
    
    // Number of frames to copy.
    unsigned int length = frames();
    
    // Loop over each possible channel. As channels are converted and written,
    // unset that channel's bit. Loop will exit as soon as all channels present
    // have been processed.
    int i = 0;
    while(channels)
    {
        if( channels & CanonicalChannels::get(i) )
        {
            SampleFormats::convertMany<T, OutSampleType>(mChannels[i], buffer.mChannels[i], length);
            channels ^= CanonicalChannels::get(i);
        }
        ++i;
    }
    
}

template< typename T >
void TypedBuffer<T>::read(RawBuffer &buffer) {
    
    unsigned int length = std::min(buffer.space(), mWriteIndex - mReadIndex);
    
    // Loop through each channel available in the raw buffer.
    for( uint32_t i = 0; i < buffer.mChannelCount; ++i ){
        
        // Skip the channel if the buffer doesn't support it.
        if( !(mFormat.channels() & buffer.mBuffers[i].mChannel) ) {
            continue;
        }
        
        T *in = mChannels[CanonicalChannels::indexOf(buffer.mBuffers[i].mChannel)] + mReadIndex;
        
        switch (buffer.mFormat) {
            case kInt16: {
                SampleFormats::convertMany<T, SampleInt16>(in,
                                                           buffer.writeAs<SampleInt16>(i),
                                                           buffer.mStride,
                                                           length);
                continue;
            }
            case kInt32: {
                SampleFormats::convertMany<T, SampleInt32>(in,
                                                           buffer.writeAs<SampleInt32>(i),
                                                           buffer.mStride,
                                                           length);
                continue;
            }
            case kFloat32: {
                SampleFormats::convertMany<T, SampleFloat32>(in,
                                                             buffer.writeAs<SampleFloat32>(i),
                                                             buffer.mStride,
                                                             length);
                continue;
            }
            case kFloat64: {
                SampleFormats::convertMany<T, SampleFloat64>(in,
                                                             buffer.writeAs<SampleFloat64>(i),
                                                             buffer.mStride,
                                                             length);
                continue;
            }
            default:
                continue;
        }
    }
    
    buffer.mWriteIndex += length;
    mReadIndex += length;
}

/* Write(...) Functions */

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::writeChannel(Channel ch, T &os, InSampleType is )
{
    if( mFormat.channels() & ch )
        os = SampleFormats::convertSample<InSampleType, T>(is);
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const Mono<InSampleType> &i )
{
    mChannels[0][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FC);
    ++mWriteIndex;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const Stereo<InSampleType> &i )
{
    mChannels[0][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    mChannels[1][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    ++mWriteIndex;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const Stereo21<InSampleType> &i )
{
    mChannels[0][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    mChannels[1][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    mChannels[3][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.LFE);
    ++mWriteIndex;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const MultiChannel3<InSampleType> &i )
{
    mChannels[0][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    mChannels[1][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter,     mChannels[2][mWriteIndex], i.FC);
    writeChannel(kLowFrequencyOne, mChannels[3][mWriteIndex], i.LFE);
    writeChannel(kBackCenter,      mChannels[8][mWriteIndex], i.BC);

    ++mWriteIndex;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const MultiChannel4<InSampleType> &i )
{
    mChannels[0][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    mChannels[1][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter,     mChannels[2][mWriteIndex], i.FC);
    writeChannel(kLowFrequencyOne, mChannels[3][mWriteIndex], i.LFE);
    writeChannel(kBackCenter,      mChannels[8][mWriteIndex], i.BC);
    writeChannel(kBackLeft,        mChannels[4][mWriteIndex], i.BL);
    writeChannel(kBackRight,       mChannels[5][mWriteIndex], i.BR);
    
    ++mWriteIndex;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const MultiChannel5<InSampleType> &i )
{
    mChannels[0][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    mChannels[1][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter,     mChannels[2][mWriteIndex],  i.FC);
    writeChannel(kLowFrequencyOne, mChannels[3][mWriteIndex], i.LFE);
    writeChannel(kBackLeft,        mChannels[4][mWriteIndex],  i.BL);
    writeChannel(kBackRight,       mChannels[5][mWriteIndex],  i.BR);
    writeChannel(kSideLeft,        mChannels[9][mWriteIndex],  i.SL);
    writeChannel(kSideRight,       mChannels[10][mWriteIndex], i.SR);

    ++mWriteIndex;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const MultiChannel6<InSampleType> &i )
{
    mChannels[0][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    mChannels[1][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter,     mChannels[2][mWriteIndex],  i.FC);
    writeChannel(kLowFrequencyOne, mChannels[3][mWriteIndex],  i.LFE);
    writeChannel(kBackLeft,        mChannels[4][mWriteIndex],  i.BL);
    writeChannel(kBackRight,       mChannels[5][mWriteIndex],  i.BR);
    writeChannel(kBackCenter,      mChannels[8][mWriteIndex],  i.BC);
    writeChannel(kSideLeft,        mChannels[9][mWriteIndex],  i.SL);
    writeChannel(kSideRight,       mChannels[10][mWriteIndex], i.SR);
    
    ++mWriteIndex;
}

template< typename T >
template< typename InSampleType >
void TypedBuffer<T>::write( const MultiChannel7<InSampleType> &i )
{
    mChannels[0][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    mChannels[1][mWriteIndex] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter,        mChannels[2][mWriteIndex],  i.FC);
    writeChannel(kLowFrequencyOne,    mChannels[3][mWriteIndex],  i.LFE);
    writeChannel(kBackLeft,           mChannels[4][mWriteIndex],  i.BL);
    writeChannel(kBackRight,          mChannels[5][mWriteIndex],  i.BR);
    writeChannel(kFrontLeftOfCenter,  mChannels[6][mWriteIndex],  i.FLc);
    writeChannel(kFrontRightOfCenter, mChannels[7][mWriteIndex],  i.FRc);
    writeChannel(kSideLeft,           mChannels[9][mWriteIndex],  i.SL);
    writeChannel(kSideRight,          mChannels[10][mWriteIndex], i.SR);

    ++mWriteIndex;
}

template< typename T >
template< typename InSampleType >
void TypedBuffer<T>::write(const TypedBuffer<InSampleType> &buffer)
{
    // Compatability check first. Buffers must be equal length in frames, and sample
    // rate. Required, or else resampling will need to be performed.
    if((buffer.mFormat.sampleRate() != mFormat.sampleRate()) ||
       (buffer.frames() != frames())
       )
    {
        // TODO: Raise an exception?
        return;
    }
    
    // Determine the channels that need to be copied (union of the channel layouts).
    // Apply the channel mask to prevent crash-causing inputs.
    Channels channels = (buffer.mFormat.channels() & mFormat.channels()) & kChannelMask;
    
    // Number of frames to copy.
    unsigned int length = frames();
    
    // Loop over each possible channel. As channels are converted and written,
    // unset that channel's bit. Loop will exit as soon as all channels present
    // have been processed.
    int i = 0;
    while(channels)
    {
        if( channels & CanonicalChannels::get(i) )
        {
            SampleFormats::convertMany<InSampleType, T>(buffer.mChannels[i], mChannels[i], length);
            channels ^= CanonicalChannels::get(i);
        }
        ++i;
    }
    
}

template< typename T >
void TypedBuffer<T>::write( RawBuffer &buffer ) {
    
    unsigned int length = std::min(buffer.available(), frames() - mWriteIndex);

    // Loop through each channel available in the raw buffer.
    for( uint32_t i = 0; i < buffer.mChannelCount; ++i ){
        
        // Skip the channel if the buffer doesn't support it.
        if( !(mFormat.channels() & buffer.mBuffers[i].mChannel) ) {
            continue;
        }
        
        T *out = mChannels[CanonicalChannels::indexOf(buffer.mBuffers[i].mChannel)] + mWriteIndex;
        
        switch (buffer.mFormat) {
            case kInt16: {
                SampleFormats::convertMany<SampleInt16, T>(buffer.readAs<SampleInt16>(i),
                                                           buffer.mStride,
                                                           out, length);
                continue;
            }
            case kInt32: {
                SampleFormats::convertMany<SampleInt32, T>(buffer.readAs<SampleInt32>(i),
                                                           buffer.mStride,
                                                           out, length);
                continue;
            }
            case kFloat32: {
                SampleFormats::convertMany<SampleFloat32, T>(buffer.readAs<SampleFloat32>(i),
                                                             buffer.mStride,
                                                             out, length);
                continue;
            }
            case kFloat64: {
                SampleFormats::convertMany<SampleFloat64, T>(buffer.readAs<SampleFloat64>(i),
                                                             buffer.mStride,
                                                             out, length);
                continue;
            }
            default:
                continue;
        }
    }
    
    buffer.mReadIndex += length;
    mWriteIndex += length;
}



/* Shift-Operators */


template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<<(RawBuffer &buffer) {
    write(buffer);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>>(RawBuffer &buffer) {
    read(buffer);
    return (*this);
}

// A wild buffer appears!
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< (const Buffer& buffer )
{
    switch(buffer.sampleFormat())
    {
        case kInt16:
            write(static_cast<const Int16Buffer&>(buffer));
            break;
        case kInt32:
            write(static_cast<const Int32Buffer&>(buffer));
            break;
        case kFloat32:
            write(static_cast<const Float32Buffer&>(buffer));
            break;
        case kFloat64:
            write(static_cast<const Float64Buffer&>(buffer));
            break;
        default:
            // This should never happen.
            break;
    };
    return (*this);
}


template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( Buffer& buffer )
{
    switch(buffer.sampleFormat())
    {
        case kInt16:
            read(static_cast<Int16Buffer&>(buffer));
            break;
        case kInt32:
            read(static_cast<Int32Buffer&>(buffer));
            break;
        case kFloat32:
            read(static_cast<Float32Buffer&>(buffer));
            break;
        case kFloat64:
            read(static_cast<Float64Buffer&>(buffer));
            break;
        default:
            // This should never happen.
            break;
    };
    return (*this);
}


// Mono
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< (const Mono<SampleInt16>& f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< (const Mono<SampleInt32>& f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< (const Mono<SampleFloat32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< (const Mono<SampleFloat64>&f ){
    write(f);
    return (*this);
}


template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( Mono<SampleInt16>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( Mono<SampleInt32>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( Mono<SampleFloat32>&f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( Mono<SampleFloat64>&f ){
    read(f);
    return (*this);
}

// Stereo
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const Stereo<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const Stereo<SampleInt32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const Stereo<SampleFloat64>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const Stereo<SampleFloat32>&f ){
    write(f);
    return (*this);
}


template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( Stereo<SampleInt16>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( Stereo<SampleInt32>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( Stereo<SampleFloat32>&f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( Stereo<SampleFloat64>&f ){
    read(f);
    return (*this);
}

// Stereo21
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const Stereo21<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const Stereo21<SampleInt32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const Stereo21<SampleFloat32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const Stereo21<SampleFloat64>&f ){
    write(f);
    return (*this);
}


template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( Stereo21<SampleInt16>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( Stereo21<SampleInt32>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( Stereo21<SampleFloat32>&f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( Stereo21<SampleFloat64>&f ){
    read(f);
    return (*this);
}

// MultiChannel3
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel3<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel3<SampleInt32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel3<SampleFloat32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel3<SampleFloat64>&f ){
    write(f);
    return (*this);
}


template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel3<SampleInt16>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel3<SampleInt32>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel3<SampleFloat32>&f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel3<SampleFloat64>&f ){
    read(f);
    return (*this);
}

// MultiChannel4
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel4<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel4<SampleInt32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel4<SampleFloat32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel4<SampleFloat64>&f ){
    write(f);
    return (*this);
}


template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel4<SampleInt16>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel4<SampleInt32>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel4<SampleFloat32>&f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel4<SampleFloat64>&f ){
    read(f);
    return (*this);
}

// MultiChannel5
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel5<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel5<SampleInt32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel5<SampleFloat32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel5<SampleFloat64>&f ){
    write(f);
    return (*this);
}


template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel5<SampleInt16>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel5<SampleInt32>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel5<SampleFloat32>&f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel5<SampleFloat64>&f ){
    read(f);
    return (*this);
}

// MultiChannel6
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel6<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel6<SampleInt32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel6<SampleFloat32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel6<SampleFloat64>&f ){
    write(f);
    return (*this);
}


template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel6<SampleInt16>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel6<SampleInt32>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel6<SampleFloat32>&f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel6<SampleFloat64>&f ){
    read(f);
    return (*this);
}

// MultiChannel7
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel7<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel7<SampleInt32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel7<SampleFloat32>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel7<SampleFloat64>&f ){
    write(f);
    return (*this);
}


template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel7<SampleInt16>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel7<SampleInt32>& f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel7<SampleFloat32>&f ){
    read(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator>> ( MultiChannel7<SampleFloat64>&f ){
    read(f);
    return (*this);
}


namespace Stargazer {
    namespace Audio
    {

        template class TypedBuffer<SampleInt16>;
        template class TypedBuffer<SampleInt32>;
        template class TypedBuffer<SampleFloat32>;
        template class TypedBuffer<SampleFloat64>;
        
    }
}

