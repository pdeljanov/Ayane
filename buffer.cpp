/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-07.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */

#include "buffer.h"

#include <core/alignedmemory.h>

using namespace Stargazer::Audio;

Buffer::Buffer ( const BufferFormat &format, const BufferLength &length ) :
    m_format(format),
    m_length(length),
    m_timestamp(0),
    m_flags(kNone)
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


template<typename T>
TypedBuffer<T>::TypedBuffer( const BufferFormat &format, const BufferLength &length ) :
    Buffer( format, length ),
    m_wr(0), m_rd(0)
{
    // Calculate the number of frames in the buffer.
    unsigned int frames = length.frames ( format.sampleRate() );
    
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
     * form the start address of the channel buffer. Channel pointers are stored in the canonical channel
     * ordering. If a channel is not used, m_bufs for that channel index is null. The first pointer in
     * m_bufs *must* be base as ChannelMapper takes ownership of the memory.
     */
    
    channels &= kChannelMask;
    
    // map[0] must *always* be a pointer to the start of the buffer.
    map[0] = base;
    
    int i = 0;  // Channel we're testing.
    int j = 0;  // Previous buffer we wrote to.
    
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
{ static SampleFormat sampleFormat(){ return Int16; } };

template <>
struct TypedBufferTraits<SampleInt32>
{ static SampleFormat sampleFormat(){ return Int32; } };

template <>
struct TypedBufferTraits<SampleFloat32>
{ static SampleFormat sampleFormat(){ return Float32; } };

template <>
struct TypedBufferTraits<SampleFloat64>
{ static SampleFormat sampleFormat(){ return Float64; } };

template<typename T>
SampleFormat TypedBuffer<T>::sampleFormat() const
{ return TypedBufferTraits<T>::sampleFormat(); }


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
    // rate.
    if((buffer.m_format.sampleRate() != m_format.sampleRate()) ||
       (buffer.frames() != frames())
       )
    {
        // TOOD: Raise an exception?
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




/* Write Shift-Operators */

// A wild buffer appears!
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< (const Buffer& buffer )
{
    switch(buffer.sampleFormat())
    {
        case Int16:
            write(static_cast<const Int16Buffer&>(buffer));
            break;
        case Int32:
            write(static_cast<const Int32Buffer&>(buffer));
            break;
        case Float32:
            write(static_cast<const Float32Buffer&>(buffer));
            break;
        case Float64:
            write(static_cast<const Float64Buffer&>(buffer));
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
TypedBuffer<T> &TypedBuffer<T>::operator<< (const Mono<SampleFloat32>&f ){
    write(f);
    return (*this);
}

// Stereo
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const Stereo<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const Stereo<SampleFloat32>&f ){
    write(f);
    return (*this);
}

// Stereo21
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const Stereo21<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const Stereo21<SampleFloat32>&f ){
    write(f);
    return (*this);
}

// MultiChannel3
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel3<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel3<SampleFloat32>&f ){
    write(f);
    return (*this);
}

// MultiChannel4
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel4<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel4<SampleFloat32>&f ){
    write(f);
    return (*this);
}

// MultiChannel5
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel5<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel5<SampleFloat32>&f ){
    write(f);
    return (*this);
}

// MultiChannel6
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel6<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel6<SampleFloat32>&f ){
    write(f);
    return (*this);
}

// MultiChannel7
template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel7<SampleInt16>&f ){
    write(f);
    return (*this);
}

template<typename T>
TypedBuffer<T> &TypedBuffer<T>::operator<< ( const MultiChannel7<SampleFloat32>&f ){
    write(f);
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



