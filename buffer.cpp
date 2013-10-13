/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-07.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */

#include "buffer.h"
#include "formats.h"

#include <core/alignedmemory.h>

#include <cstring>


namespace Stargazer
{
    namespace Audio
    {

        class BufferPrivate
        {
        public:
            
            SampleFloat32 *buffer;
            
            unsigned int samples;
            
            BufferFormat format;
            BufferLength length;
            
            Duration timestamp;
            
        public:
            
            BufferPrivate() :
                buffer ( NULL ), samples ( 0 ), timestamp ( 0.0f )
            {
            }
            
        };
        
    }
}


using namespace Stargazer::Audio;

Buffer::Buffer ( const BufferFormat &format, const BufferLength &length ) : d_ptr ( new BufferPrivate )
{
    S_D(Buffer);
    
    // Initialize the buffer attributes.
    d->format = format;
    d->length = length;
    
    // Calculate the total number of samples the buffer must store.
    d->samples = length.frames ( d->format.sampleRate() ) * d->format.channelCount();
    
    // Allocate the buffer with 16 byte alignment.
    d->buffer = AlignedMemory::allocate16<SampleFloat32>(d->samples);
}

Buffer::Buffer ( const Buffer &source ) : d_ptr ( new BufferPrivate )
{
    S_D(Buffer);
    
    // Copy the buffer attributes from the source buffer.
    d->format = source.d_ptr->format;
    d->length = source.d_ptr->length;
    d->samples = source.d_ptr->samples;
    
    // Allocate the buffer with 16 byte alignment.
    d->buffer = AlignedMemory::allocate16<SampleFloat32>(d->samples);
    
    // Copy the source buffer's data into this buffer.
    memcpy ( d->buffer, source.d_ptr->buffer, sizeof ( SampleFloat32 ) * d->samples );
}

Buffer::~Buffer( )
{
    S_D(Buffer);
    
    // Deallocate the buffer.
    AlignedMemory::deallocate<SampleFloat32>(d->buffer);

    // Destroy the d-pointer.
    delete d_ptr;
}

Duration Buffer::duration() const
{
    S_D(const Buffer);
    return Duration ( d->length.duration ( d->format.m_sampleRate ) );
}

const Duration& Buffer::timestamp() const
{
    S_D(const Buffer);
    return d->timestamp;
}

void Buffer::setTimestamp ( const Duration& timestamp )
{
    S_D(Buffer);
    d->timestamp = timestamp;
}

const BufferFormat& Buffer::format() const
{
    S_D(const Buffer);
    return d->format;
}

unsigned int Buffer::length() const
{
    S_D(const Buffer);
    return d->samples;
}

unsigned int Buffer::frames() const
{
    S_D(const Buffer);
    return d->length.frames ( d->format.m_sampleRate );
}

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

