/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-07.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */

#include "buffer.h"
#include "rawbuffer.h"

#include <core/alignedmemory.h>

using namespace Stargazer::Audio;

Buffer::Buffer ( const BufferFormat &format, const BufferLength &length ) :
    m_format(format),
    m_length(length),
    m_timestamp(0),
    m_flags(kNone),
    m_wr(0), m_rd(0)
{

}

Buffer::Buffer ( const Buffer &source ) : m_timestamp(0)
{
    /*
    // Copy the buffer attributes from the source buffer.
    m_format = source.m_format;
    m_length = source.m_length;
    m_samples = source.m_samples;
    
    // Allocate the buffer with 16 byte alignment.
    m_buffer = AlignedMemory::allocate16<unsigned char>(m_samples * m_stride);
    
    // Copy the source buffer's data into this buffer.
    memcpy ( m_buffer, source.m_buffer, m_stride * m_samples );
     */
}

Buffer::~Buffer( )
{
    // :)
}

Duration Buffer::duration() const
{
    return Duration ( m_length.duration ( m_format.m_sampleRate ) );
}

const Duration& Buffer::timestamp() const
{
    return m_timestamp;
}

void Buffer::setTimestamp ( const Duration& timestamp )
{
    m_timestamp = timestamp;
}

const BufferFormat& Buffer::format() const
{
    return m_format;
}

unsigned int Buffer::frames() const
{
    return m_length.frames ( m_format.m_sampleRate );
}

unsigned int Buffer::available() const {
    // Frames that can be read!
    return m_wr - m_rd;
}

unsigned int Buffer::space() const {
    // Frames that can be written!
    return frames() - m_wr;
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
    buildChannelMap(m_ch, format.channels(), buffer, frames);
}

template<typename T>
TypedBuffer<T>::~TypedBuffer()
{
    // Deallocate the buffer.
    AlignedMemory::deallocate(m_ch[0]);
    
    // Call base class destructor.
    Buffer::~Buffer();
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
        if( channels & kCanonicalChannels[i] )
        {
            map[i] = (i > 0) ? (map[j] + stride) : base;
            channels ^= kCanonicalChannels[i];
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
    if( m_format.channels() & ch )
        os = SampleFormats::convertSample<T, OutSampleType>(is);
}

template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( Mono<OutSampleType> &i )
{
    i.FC = SampleFormats::convertSample<T, OutSampleType>(m_ch[0][m_rd]);
    ++m_rd;
}

template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( Stereo<OutSampleType> &i )
{
    i.FL = SampleFormats::convertSample<OutSampleType, T>(m_ch[0][m_rd]);
    i.FR = SampleFormats::convertSample<OutSampleType, T>(m_ch[1][m_rd]);
    ++m_rd;
}

template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( Stereo21<OutSampleType> &i )
{
    i.FL  = SampleFormats::convertSample<OutSampleType, T>(m_ch[0][m_rd]);
    i.FR  = SampleFormats::convertSample<OutSampleType, T>(m_ch[1][m_rd]);
    i.LFE = SampleFormats::convertSample<OutSampleType, T>(m_ch[3][m_rd]);
    ++m_rd;
}



template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( MultiChannel3<OutSampleType> &i )
{
    i.FL = SampleFormats::convertSample<OutSampleType, T>(m_ch[0][m_rd]);
    i.FR = SampleFormats::convertSample<OutSampleType, T>(m_ch[1][m_rd]);
    
    readChannel(kFrontCenter,     m_ch[2][m_rd], i.FC);
    readChannel(kLowFrequencyOne, m_ch[3][m_rd], i.LFE);
    readChannel(kBackCenter,      m_ch[8][m_rd], i.BC);
    
    ++m_rd;
}

template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( MultiChannel4<OutSampleType> &i )
{
    i.FL = SampleFormats::convertSample<OutSampleType, T>(m_ch[0][m_rd]);
    i.FR = SampleFormats::convertSample<OutSampleType, T>(m_ch[1][m_rd]);
    
    readChannel(kFrontCenter,     m_ch[2][m_rd], i.FC);
    readChannel(kLowFrequencyOne, m_ch[3][m_rd], i.LFE);
    readChannel(kBackCenter,      m_ch[8][m_rd], i.BC);
    readChannel(kBackLeft,        m_ch[4][m_rd], i.BL);
    readChannel(kBackRight,       m_ch[5][m_rd], i.BR);
    
    ++m_rd;
}

template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( MultiChannel5<OutSampleType> &i )
{
    i.FL = SampleFormats::convertSample<OutSampleType, T>(m_ch[0][m_rd]);
    i.FR = SampleFormats::convertSample<OutSampleType, T>(m_ch[1][m_rd]);
    
    readChannel(kFrontCenter,     m_ch[2][m_rd],  i.FC);
    readChannel(kLowFrequencyOne, m_ch[3][m_rd], i.LFE);
    readChannel(kBackLeft,        m_ch[4][m_rd],  i.BL);
    readChannel(kBackRight,       m_ch[5][m_rd],  i.BR);
    readChannel(kSideLeft,        m_ch[9][m_rd],  i.SL);
    readChannel(kSideRight,       m_ch[10][m_rd], i.SR);
    
    ++m_rd;
}

template< typename T >
template< typename OutSampleType >
force_inline void TypedBuffer<T>::read( MultiChannel6<OutSampleType> &i )
{
    i.FL = SampleFormats::convertSample<OutSampleType, T>(m_ch[0][m_rd]);
    i.FR = SampleFormats::convertSample<OutSampleType, T>(m_ch[1][m_rd]);
    
    readChannel(kFrontCenter,     m_ch[2][m_rd],  i.FC);
    readChannel(kLowFrequencyOne, m_ch[3][m_rd],  i.LFE);
    readChannel(kBackLeft,        m_ch[4][m_rd],  i.BL);
    readChannel(kBackRight,       m_ch[5][m_rd],  i.BR);
    readChannel(kBackCenter,      m_ch[8][m_rd],  i.BC);
    readChannel(kSideLeft,        m_ch[9][m_rd],  i.SL);
    readChannel(kSideRight,       m_ch[10][m_rd], i.SR);
    
    ++m_rd;
}

template< typename T >
template< typename OutSampleType >
void TypedBuffer<T>::read( MultiChannel7<OutSampleType> &i )
{
    i.FL = SampleFormats::convertSample<OutSampleType, T>(m_ch[0][m_rd]);
    i.FR = SampleFormats::convertSample<OutSampleType, T>(m_ch[1][m_rd]);
    
    readChannel(kFrontCenter,        m_ch[2][m_rd],  i.FC);
    readChannel(kLowFrequencyOne,    m_ch[3][m_rd],  i.LFE);
    readChannel(kBackLeft,           m_ch[4][m_rd],  i.BL);
    readChannel(kBackRight,          m_ch[5][m_rd],  i.BR);
    readChannel(kFrontLeftOfCenter,  m_ch[6][m_rd],  i.FLc);
    readChannel(kFrontRightOfCenter, m_ch[7][m_rd],  i.FRc);
    readChannel(kSideLeft,           m_ch[9][m_rd],  i.SL);
    readChannel(kSideRight,          m_ch[10][m_rd], i.SR);
    
    ++m_rd;
}

template< typename T >
template< typename OutSampleType >
void TypedBuffer<T>::read(TypedBuffer<OutSampleType> &buffer)
{
    // Compatability check first. Buffers must be equal length in frames, and sample
    // rate. Required, or else resampling will need to be performed.
    if((buffer.m_format.sampleRate() != m_format.sampleRate()) ||
       (buffer.frames() != frames())
       )
    {
        // TODO: Raise an exception?
        return;
    }
    
    // Determine the channels that need to be copied (union of the channel layouts).
    // Apply the channel mask to prevent crash-causing inputs.
    Channels channels = (buffer.m_format.channels() & m_format.channels()) & kChannelMask;
    
    // Number of frames to copy.
    unsigned int length = frames();
    
    // Loop over each possible channel. As channels are converted and written,
    // unset that channel's bit. Loop will exit as soon as all channels present
    // have been processed.
    int i = 0;
    while(channels)
    {
        if( channels & kCanonicalChannels[i] )
        {
            SampleFormats::convertMany<T, OutSampleType>(m_ch[i], buffer.m_ch[i], length);
            channels ^= kCanonicalChannels[i];
        }
        ++i;
    }
    
}

template< typename T >
void TypedBuffer<T>::read(RawBuffer &buffer) {
    
    unsigned int length = std::min(buffer.space(), m_wr - m_rd);
    
    // Loop through each channel available in the raw buffer.
    for( int i = 0; i < buffer.mChannelCount; ++i ){
        
        // Skip the channel if the buffer doesn't support it.
        if( !(m_format.channels() & buffer.mBuffers[i].mChannel) ) {
            continue;
        }
        
        T *in = m_ch[ChannelToCanonicalIndex(buffer.mBuffers[i].mChannel)] + m_rd;
        
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
    m_rd += length;
}

/* Write(...) Functions */

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::writeChannel(Channel ch, T &os, InSampleType is )
{
    if( m_format.channels() & ch )
        os = SampleFormats::convertSample<InSampleType, T>(is);
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const Mono<InSampleType> &i )
{
    m_ch[0][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FC);
    ++m_wr;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const Stereo<InSampleType> &i )
{
    m_ch[0][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    m_ch[1][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    ++m_wr;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const Stereo21<InSampleType> &i )
{
    m_ch[0][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    m_ch[1][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    m_ch[3][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.LFE);
    ++m_wr;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const MultiChannel3<InSampleType> &i )
{
    m_ch[0][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    m_ch[1][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter,     m_ch[2][m_wr], i.FC);
    writeChannel(kLowFrequencyOne, m_ch[3][m_wr], i.LFE);
    writeChannel(kBackCenter,      m_ch[8][m_wr], i.BC);

    ++m_wr;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const MultiChannel4<InSampleType> &i )
{
    m_ch[0][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    m_ch[1][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter,     m_ch[2][m_wr], i.FC);
    writeChannel(kLowFrequencyOne, m_ch[3][m_wr], i.LFE);
    writeChannel(kBackCenter,      m_ch[8][m_wr], i.BC);
    writeChannel(kBackLeft,        m_ch[4][m_wr], i.BL);
    writeChannel(kBackRight,       m_ch[5][m_wr], i.BR);
    
    ++m_wr;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const MultiChannel5<InSampleType> &i )
{
    m_ch[0][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    m_ch[1][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter,     m_ch[2][m_wr],  i.FC);
    writeChannel(kLowFrequencyOne, m_ch[3][m_wr], i.LFE);
    writeChannel(kBackLeft,        m_ch[4][m_wr],  i.BL);
    writeChannel(kBackRight,       m_ch[5][m_wr],  i.BR);
    writeChannel(kSideLeft,        m_ch[9][m_wr],  i.SL);
    writeChannel(kSideRight,       m_ch[10][m_wr], i.SR);

    ++m_wr;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const MultiChannel6<InSampleType> &i )
{
    m_ch[0][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    m_ch[1][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter,     m_ch[2][m_wr],  i.FC);
    writeChannel(kLowFrequencyOne, m_ch[3][m_wr],  i.LFE);
    writeChannel(kBackLeft,        m_ch[4][m_wr],  i.BL);
    writeChannel(kBackRight,       m_ch[5][m_wr],  i.BR);
    writeChannel(kBackCenter,      m_ch[8][m_wr],  i.BC);
    writeChannel(kSideLeft,        m_ch[9][m_wr],  i.SL);
    writeChannel(kSideRight,       m_ch[10][m_wr], i.SR);
    
    ++m_wr;
}

template< typename T >
template< typename InSampleType >
void TypedBuffer<T>::write( const MultiChannel7<InSampleType> &i )
{
    m_ch[0][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FL);
    m_ch[1][m_wr] = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter,        m_ch[2][m_wr],  i.FC);
    writeChannel(kLowFrequencyOne,    m_ch[3][m_wr],  i.LFE);
    writeChannel(kBackLeft,           m_ch[4][m_wr],  i.BL);
    writeChannel(kBackRight,          m_ch[5][m_wr],  i.BR);
    writeChannel(kFrontLeftOfCenter,  m_ch[6][m_wr],  i.FLc);
    writeChannel(kFrontRightOfCenter, m_ch[7][m_wr],  i.FRc);
    writeChannel(kSideLeft,           m_ch[9][m_wr],  i.SL);
    writeChannel(kSideRight,          m_ch[10][m_wr], i.SR);

    ++m_wr;
}

template< typename T >
template< typename InSampleType >
void TypedBuffer<T>::write(const TypedBuffer<InSampleType> &buffer)
{
    // Compatability check first. Buffers must be equal length in frames, and sample
    // rate. Required, or else resampling will need to be performed.
    if((buffer.m_format.sampleRate() != m_format.sampleRate()) ||
       (buffer.frames() != frames())
       )
    {
        // TODO: Raise an exception?
        return;
    }
    
    // Determine the channels that need to be copied (union of the channel layouts).
    // Apply the channel mask to prevent crash-causing inputs.
    Channels channels = (buffer.m_format.channels() & m_format.channels()) & kChannelMask;
    
    // Number of frames to copy.
    unsigned int length = frames();
    
    // Loop over each possible channel. As channels are converted and written,
    // unset that channel's bit. Loop will exit as soon as all channels present
    // have been processed.
    int i = 0;
    while(channels)
    {
        if( channels & kCanonicalChannels[i] )
        {
            SampleFormats::convertMany<InSampleType, T>(buffer.m_ch[i], m_ch[i], length);
            channels ^= kCanonicalChannels[i];
        }
        ++i;
    }
    
}

template< typename T >
void TypedBuffer<T>::write( RawBuffer &buffer ) {
    
    unsigned int length = std::min(buffer.available(), frames() - m_wr);

    // Loop through each channel available in the raw buffer.
    for( int i = 0; i < buffer.mChannelCount; ++i ){
        
        // Skip the channel if the buffer doesn't support it.
        if( !(m_format.channels() & buffer.mBuffers[i].mChannel) ) {
            continue;
        }
        
        T *out = m_ch[ChannelToCanonicalIndex(buffer.mBuffers[i].mChannel)] + m_wr;
        
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
    m_wr += length;
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

