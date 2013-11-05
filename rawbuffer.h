#ifndef STARGAZER_STDLIB_AUDIO_RAWBUFFER_H_
#define STARGAZER_STDLIB_AUDIO_RAWBUFFER_H_

#include <cstddef>

#include <vector>

#include "formats.h"
#include "channels.h"

namespace Stargazer
{
    namespace Audio
    {

        class Buffer;

        class RawBuffer
        {
            template<typename T>
            friend class TypedBuffer;
            
        public:
            
            typedef struct {
                
                /** Pointer to the buffer. */
                void *mBuffer;
                
                /** Buffer channel assignment. */
                Channel mChannel;
                
            } BufferDescriptor;
            

            RawBuffer(uint32_t frames, uint32_t channels, SampleFormat format,
                      bool planar);

            /**
             *  Get the number of frames available to be read.
             */
            unsigned int available() const {
                return mWriteIndex - mReadIndex;
            }
            
            /**
             *  Get the number of frames not written.
             */
            unsigned int space() const {
                return mFrames - mWriteIndex;
            }
            
            /**
             *  Resets the read and write pointers.
             */
            void reset() {
                mReadIndex = 0;
                mWriteIndex = 0;
            }
            
            /**
             *  Returns a pointer to be used for reading from the raw buffer.
             */
            template<typename OutSampleType>
            OutSampleType *readAs( unsigned int channel ){
                
                OutSampleType *base = nullptr;
                
                if(mDataLayoutIsPlanar) {
                    base = reinterpret_cast<OutSampleType*>(mBuffers[channel].mBuffer);
                    base += mReadIndex;
                }
                else {
                    base = reinterpret_cast<OutSampleType*>(mBuffers[0].mBuffer) + channel;
                    base += (mReadIndex * mStride);
                }
                
                return base;
            }
            
            /**
             *  Returns a pointer to be used for writing from the raw buffer.
             */
            template<typename OutSampleType>
            OutSampleType *writeAs( unsigned int channel ){
                
                OutSampleType *base = nullptr;
                
                if(mDataLayoutIsPlanar) {
                    base = reinterpret_cast<OutSampleType*>(mBuffers[channel].mBuffer);
                    base += mWriteIndex;
                }
                else {
                    base = reinterpret_cast<OutSampleType*>(mBuffers[0].mBuffer) + channel;
                    base += (mWriteIndex * mStride);
                }
                
                return base;
            }
            
            RawBuffer& operator>> (Buffer& rhs);

            RawBuffer& operator<< (Buffer& rhs);
            
            
            
            /** The sample format of the raw buffer. */
            SampleFormat mFormat;
            
            /** True if the data layout is planar, false otherwise. */
            bool mDataLayoutIsPlanar;
            
            /** The number of frames the raw buffer can store. */
            unsigned int mFrames;
            
            /** The number of frames read. */
            unsigned int mReadIndex;
            
            /** The number of frames written. */
            unsigned int mWriteIndex;
            
            /** The number of channels. */
            uint32_t mChannelCount;
            
            /** The stride of the raw data. */
            uint32_t mStride;
            
            /** Buffer descriptors. */
            BufferDescriptor mBuffers[kMaximumChannels];
        };
        
    }
}

#endif
