#include "bufferformat.h"

using namespace Stargazer::Audio;

BufferFormat::BufferFormat() :
    mChannels ( 0 ),
    mSampleRate ( 0 ),
    mSamplesPerFrame ( 0 )
{
}

BufferFormat::BufferFormat ( Channels channels, SampleRate sampleRate ) :
    mChannels ( channels ),
    mSampleRate ( sampleRate ),
    mSamplesPerFrame ( 0 )
{
    
    // # of set bits in 0x00 to 0x0f
    const static int lookup[] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
    
    for ( ; channels != 0; channels >>= 4 ) {
        mSamplesPerFrame += lookup[ channels & 0x0f ];
    }
    
}

BufferFormat::BufferFormat ( const BufferFormat& format ) :
    mChannels ( format.mChannels ),
    mSampleRate ( format.mSampleRate ),
    mSamplesPerFrame ( format.mSamplesPerFrame )
{
}


bool BufferFormat::isValid() const {
    
    return ( ( mChannels > 0 ) && ( mSampleRate > 0 ) );
}

BufferFormat& BufferFormat::operator= ( const BufferFormat& right ) {
    
    mChannels = right.mChannels;
    mSampleRate = right.mSampleRate;
    mSamplesPerFrame = right.mSamplesPerFrame;
    return *this;
}

bool BufferFormat::operator== ( const BufferFormat& right ) const {
    
    return ( mChannels == right.mChannels ) && ( mSampleRate == right.mSampleRate );
}

bool BufferFormat::operator!= ( const BufferFormat& right ) const {
    
    return ( ( mChannels != right.mChannels ) || ( mSampleRate != right.mSampleRate ) );
}

bool BufferFormat::operator< ( const BufferFormat& right ) const {
    
    if( mSampleRate < right.mSampleRate ) {
        return true;
    }
    else if( mSampleRate == right.mSampleRate ) {
        return ( mSamplesPerFrame < right.mSamplesPerFrame );
    }
    else {
        return false;
    }
}

bool BufferFormat::operator<= ( const BufferFormat& right ) const {
    
    if( mSampleRate < right.mSampleRate ) {
        return true;
    }
    else if( mSampleRate == right.mSampleRate ) {
        return ( mSamplesPerFrame <= right.mSamplesPerFrame );
    }
    else {
        return false;
    }
}

bool BufferFormat::operator> ( const BufferFormat& right ) const {
    
    return !( *this <= right );
}

bool BufferFormat::operator>= ( const BufferFormat& right ) const {
    return !( *this < right );
}
