#include "int16buffer.h"

#include <core/alignedmemory.h>

using namespace Stargazer::Audio;

Int16Buffer::Int16Buffer(const BufferFormat &format, const BufferLength &length) :
    Buffer( format, length )
{
    // Calculate the number of actual samples the buffer must store.
    size_t samples = length.frames ( format.sampleRate() ) * format.channelCount();
    
    // Allocate the buffer with 16 byte alignment.
    m_buffer = AlignedMemory::allocate16<SampleInt16>(samples);
    
    // Setup the mapper.
    m_mapper.reset(m_buffer, format, kInterleaved);
}

Int16Buffer::~Int16Buffer()
{
    // Deallocate the buffer.
    AlignedMemory::deallocate(m_buffer);
    
    // Call base class destructor.
    Buffer::~Buffer();
}

Int16Buffer& Int16Buffer::operator<< ( const Mono<SampleInt16>& f )
{ return write(f); }

Int16Buffer& Int16Buffer::operator<< ( const Mono<SampleFloat32>& f )
{ return write(f); }

Int16Buffer& Int16Buffer::operator<< ( const Stereo20<SampleInt16>& f )
{ return write(f); }

Int16Buffer& Int16Buffer::operator<< ( const Stereo20<SampleFloat32>& f )
{ return write(f); }

