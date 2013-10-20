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
    m_timestamp(0)
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

/*
unsigned int Buffer::fill ( RawBuffer &buffer, unsigned int offset )
{
    S_D(Buffer);
    
    // Calculate the amount of samples that can be copied into the buffer
    unsigned int length = buffer.remaining();
    
    if ( length > ( d->samples - offset ) )
        length = d->samples - offset;
    
    //SampleFormats::convert( buffer, RawBuffer(d->buffer, d->samples, Float32), length );

    return length;
}

unsigned int Buffer::fill ( SampleFloat32* buffer, unsigned int offset, unsigned int n )
{
    S_D(Buffer);
    
    // Calculate the amount of samples that can be copied into the buffer
    if ( n > ( d->samples - offset ) )
        n = d->samples - offset;
    
    for( unsigned int i = offset; i < n; ++i )
        d->buffer[i] = buffer[i];
    
    return n;
}

bool Buffer::copy ( const Buffer& source )
{
    S_D(Buffer);
    
    if ( d->format == source.d_ptr->format )
    {
        d->timestamp = source.d_ptr->timestamp;
        
        size_t length = ( d->samples > source.d_ptr->samples ) ? source.d_ptr->samples : d->samples;
        
        memcpy ( d->buffer, source.d_ptr->buffer, length * sizeof ( SampleFloat32 ) );
        
        return true;
    }
    else
    {
        return false;
    }
}

RawBuffer Buffer::samples() const
{
    S_D(const Buffer);
    
    return RawBuffer( reinterpret_cast<uint8_t*>(d->buffer), d->samples, Float32 );
}

*/


template< typename SampleType >
void ChannelMapper<SampleType>::reset(SampleType *base,
                                      const BufferFormat &format,
                                      BufferStorageScheme scheme)
{
    if(scheme == kInterleaved)
    {
        m_map[0] = 0;
        m_map[1] = 1;
        m_map[2] = 2;
        m_map[3] = 3;
        m_map[4] = 4;
        m_map[5] = 5;
        m_map[6] = 6;
        m_map[7] = 7;
        m_map[8] = 8;
        m_map[9] = 9;
        m_map[10] = 10;
    }

    m_stride = format.channelCount();
    m_base = base;
}



template<typename T>
TypedBuffer<T>::TypedBuffer( const BufferFormat &format, const BufferLength &length ) : Buffer( format, length )
{
    // Calculate the number of actual samples the buffer must store.
    size_t samples = length.frames ( format.sampleRate() ) * format.channelCount();
    
    // Allocate the buffer with 16 byte alignment.
    m_buffer = AlignedMemory::allocate16<T>(samples);
    
    // Setup the mapper.
    chs.reset(m_buffer, format, kInterleaved);
}

template<typename T>
TypedBuffer<T>::~TypedBuffer()
{
    // Deallocate the buffer.
    AlignedMemory::deallocate(m_buffer);
    
    // Call base class destructor.
    Buffer::~Buffer();
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
    chs.fl() = SampleFormats::convertSample<InSampleType, T>(i.FC);
    ++chs;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const Stereo<InSampleType> &i )
{
    chs.fl() = SampleFormats::convertSample<InSampleType, T>(i.FL);
    chs.fr() = SampleFormats::convertSample<InSampleType, T>(i.FR);
    ++chs;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const Stereo21<InSampleType> &i )
{
    chs.fl() = SampleFormats::convertSample<InSampleType, T>(i.FL);
    chs.fr() = SampleFormats::convertSample<InSampleType, T>(i.FR);
    chs.lfe() = SampleFormats::convertSample<InSampleType, T>(i.LFE);
    ++chs;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const MultiChannel3<InSampleType> &i )
{
    chs.fl() = SampleFormats::convertSample<InSampleType, T>(i.FL);
    chs.fr() = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter, chs.fc(), i.FC);
    writeChannel(kBackCenter,  chs.bc(), i.BC);
    
    writeChannel(kLowFrequencyOne, chs.lfe(), i.LFE);
    
    ++chs;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const MultiChannel4<InSampleType> &i )
{
    chs.fl() = SampleFormats::convertSample<InSampleType, T>(i.FL);
    chs.fr() = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter, chs.fc(), i.FC);
    writeChannel(kBackCenter,  chs.bc(), i.BC);
    writeChannel(kBackLeft,    chs.bl(), i.BL);
    writeChannel(kBackRight,   chs.br(), i.BR);
    
    writeChannel(kLowFrequencyOne, chs.lfe(), i.LFE);
    
    ++chs;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const MultiChannel5<InSampleType> &i )
{
    chs.fl() = SampleFormats::convertSample<InSampleType, T>(i.FL);
    chs.fr() = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter, chs.fc(), i.FC);
    writeChannel(kBackLeft,    chs.bl(), i.BL);
    writeChannel(kBackRight,   chs.br(), i.BR);
    writeChannel(kSideLeft,    chs.sl(), i.SL);
    writeChannel(kSideRight,   chs.sr(), i.SR);
    
    writeChannel(kLowFrequencyOne, chs.lfe(), i.LFE);
    
    ++chs;
}

template< typename T >
template< typename InSampleType >
force_inline void TypedBuffer<T>::write( const MultiChannel6<InSampleType> &i )
{
    chs.fl() = SampleFormats::convertSample<InSampleType, T>(i.FL);
    chs.fr() = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter, chs.fc(), i.FC);
    writeChannel(kBackLeft,    chs.bl(), i.BL);
    writeChannel(kBackRight,   chs.br(), i.BR);
    writeChannel(kSideLeft,    chs.sl(), i.SL);
    writeChannel(kSideRight,   chs.sr(), i.SR);
    writeChannel(kBackCenter,  chs.bc(), i.BC);
    
    writeChannel(kLowFrequencyOne, chs.lfe(), i.LFE);
    
    ++chs;
}

template< typename T >
template< typename InSampleType >
void TypedBuffer<T>::write( const MultiChannel7<InSampleType> &i )
{
    chs.fl() = SampleFormats::convertSample<InSampleType, T>(i.FL);
    chs.fr() = SampleFormats::convertSample<InSampleType, T>(i.FR);
    
    writeChannel(kFrontCenter,        chs.fc(),  i.FC);
    writeChannel(kBackLeft,           chs.bl(),  i.BL);
    writeChannel(kBackRight,          chs.br(),  i.BR);
    writeChannel(kSideLeft,           chs.sl(),  i.SL);
    writeChannel(kSideRight,          chs.sr(),  i.SR);
    writeChannel(kFrontLeftOfCenter,  chs.flc(), i.FLc);
    writeChannel(kFrontRightOfCenter, chs.frc(), i.FRc);
    
    writeChannel(kLowFrequencyOne, chs.lfe(), i.LFE);
    
    ++chs;
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
    Channels channels = buffer.format().channels() & m_format.channels();
    
    //if( channels & kFrontLeft )
    //    SampleFormats::convertMany<InSampleType, T>( srcIter, destIter );
    
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



