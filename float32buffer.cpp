#include "float32buffer.h"

#include <core/alignedmemory.h>

using namespace Stargazer::Audio;

Float32Buffer::Float32Buffer(const BufferFormat &format, const BufferLength &length) :
    Buffer( format, length )
{
    // Calculate the number of actual samples the buffer must store.
    size_t samples = length.frames ( format.sampleRate() ) * format.channelCount();
    
    // Allocate the buffer with 16 byte alignment.
    m_buffer = AlignedMemory::allocate16<SampleFloat32>(samples);
    
    // Setup the mapper.
    m_mapper.reset(m_buffer, format, kInterleaved);
}

Float32Buffer::~Float32Buffer()
{
    // Deallocate the buffer.
    AlignedMemory::deallocate(m_buffer);
    
    // Call base class destructor.
    Buffer::~Buffer();
}

Float32Buffer& Float32Buffer::operator<< ( const Mono<SampleInt16>& f )
{ return write(f); }

Float32Buffer& Float32Buffer::operator<< ( const Mono<SampleFloat32>& f )
{ return write(f); }

Float32Buffer& Float32Buffer::operator<< ( const Stereo20<SampleInt16>& f )
{ return write(f); }

Float32Buffer& Float32Buffer::operator<< ( const Stereo20<SampleFloat32>& f )
{ return write(f); }

