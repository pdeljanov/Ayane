/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-07.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */

#include "buffer.h"
#include "formats.h"

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
    
    d->format = format;
    d->length = length;
    
    // Calculate the total number of samples this buffer must store.
    d->samples = length.frames ( d->format.sampleRate() ) * d->format.channelCount();
    
    size_t bytes = sizeof ( SampleFloat32 ) * d->samples;
    
    //d->buffer = static_cast<SampleFloat32*> ( MemoryOperations::getMemoryOperations().malloc16ByteAligned ( bytes ) );
}

Buffer::Buffer ( const Buffer &source ) : d_ptr ( new BufferPrivate )
{
    S_D(Buffer);
    
    d->format = source.d_ptr->format;
    d->length = source.d_ptr->length;
    d->samples = source.d_ptr->samples;
    
    size_t bytes = sizeof ( SampleFloat32 ) * d->samples;
    
    //d->buffer = static_cast<SampleFloat32*> ( MemoryOperations::getMemoryOperations().malloc16ByteAligned ( bytes ) );
    
    memcpy ( d->buffer, source.d_ptr->buffer, bytes );
}

Buffer::~Buffer( )
{
    S_D(Buffer);
    
    //MemoryOperations::getMemoryOperations().freeAligned ( d->buffer );
    
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
    unsigned int length = buffer.m_length - buffer.m_consumed;
    
    if ( length > ( d->samples - offset ) ) length = d->samples - offset;
    
    //SampleFormatConverters::Converter[buffer.m_format]( &d->buffer[offset], ( uint8_t* ) buffer.buffer ( length ), length );
    
    return length;
}

unsigned int Buffer::fill ( SampleFloat32* buffer, unsigned int offset, unsigned int n )
{
    S_D(Buffer);
    
    // Calculate the amount of samples that can be copied into the buffer
    if ( n > ( d->samples - offset ) ) n = d->samples - offset;
    
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
    
    return RawBuffer( d->buffer, d->samples, Float32 );
}

