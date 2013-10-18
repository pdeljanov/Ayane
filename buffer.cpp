/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-07.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */

#include "buffer.h"


#include <cstring>

using namespace Stargazer::Audio;

Buffer::Buffer ( const BufferFormat &format, const BufferLength &length ) :
    m_format(format),
    m_length(length),
    m_samples(0),
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