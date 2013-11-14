/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef STARGAZER_STDLIB_AUDIO_BUFFER_H_
#define STARGAZER_STDLIB_AUDIO_BUFFER_H_

#include <cstddef>

#include <core/macros.h>
#include <core/attributes.h>

#include "bufferlength.h"
#include "bufferformat.h"
#include "bufferframes.h"

#include "duration.h"
#include "formats.h"

namespace Stargazer
{
    namespace Audio
    {
        
        class RawBuffer;
        
        class Buffer
        {

            // Friend concrete TypedBuffer<T> classes so that it may access Buffer's
            // internals.
            template<typename T>
            friend class TypedBuffer;
            
        public:

            /** Buffer stream flags. */
            typedef enum
            {
                /** No specific flags set. */
                kNone = 0,
                
                /** Format negotiation required. */
                kEndOfStream = 1<<0
            }
            StreamFlag;
            
            typedef uint32_t StreamFlags;
            
            Buffer ( const BufferFormat &format, const BufferLength &length );

            virtual ~Buffer();
            
            /** 
             *  Sets a buffer stream flag. 
             */
            inline void setFlag( StreamFlag flag ){
                mFlags |= flag;
            }
            
            /** 
             *  Unsets a buffer stream flag. 
             */
            inline void unsetFlag( StreamFlag flag ){
                mFlags ^= flag;
            }
            
            /**
             *  Gets the buffer stream flags. 
             */
            StreamFlags flags() const {
                return mFlags;
            }
            
            /** 
             *  Get the buffer's presentation timestamp. The presentation
             *  timestamp at which the buffer should be played.
             */
            const Duration &timestamp() const;
            
            /**
             *  Sets the buffer's presentation timestamp.
             */
            void setTimestamp ( const Duration &timestamp );
            
            /** 
             *  Get the duration of the buffer.
             */
            Duration duration() const;

            /** 
             *  Returns the maximum amount of frames the buffer may contain.
             */
            unsigned int frames() const;
            
            /**
             *  Returns the number of frames available to be read.
             */
            unsigned int available() const;
            
            /**
             *  Returns the number of frames that have yet to be written.
             */
            unsigned int space() const;
            
            /**
             *  Returns the buffer format descriptor.
             *
             *  \return The buffer format.
             */
            const BufferFormat &format() const;
            

            /** Replaces the contents of this buffer with the contents of the source buffer.
             *
             * The source buffer must have the same sample rate or else the copy will fail.
             * If the source buffer contains more samples than this one, the copy will be truncated.
             *
             * \param source The source buffer to copy from.
             *
             * \return True on success, false otherwise.
             */
            bool copy ( const Buffer &source );

            
            /**
             *  Gets the buffer's sample format.
             */
            virtual SampleFormat sampleFormat() const = 0;
            
            
            
            /* --- Shift Operator Overloads --- */
            
            /* 
             * Each shift operator will need to be implemented in the actual concrete buffer
             * classes. This is slightly messy, but is the best way (performance wise) to achieve 
             * the required level of indirection to allow the compiler to inline the conversion 
             * operations.
             *
             * At compile time, only one of the two operands for the shift types are static. In the
             * write case, the input type is known; likewise, in the read case, the output type is 
             * known. However, for the compiler to inline the conversion code, both the input/output
             * types need to be known (i.e., the type of the caller, and the type of buffer). This requires
             * a level of indirection, and so these overloads must be present in all concrete buffer types.
             *
             * Objectively, this is the *most* performant way to gain the features desired by a large
             * margin. Since we would need a table of conversion functions for all possible buffer, sample
             * format and frame combinations (3d matrix), and we want to remain type safe, virtual functions
             * are a reasonable choice. VTables will only be generated once, so memory overhead remains the same.
             *
             */
            
            /* Writers */
    
            virtual Buffer& operator<< (const Mono<SampleInt16>& ) = 0;
            virtual Buffer& operator<< (const Mono<SampleInt32>& ) = 0;
            virtual Buffer& operator<< (const Mono<SampleFloat32>& ) = 0;
            virtual Buffer& operator<< (const Mono<SampleFloat64>& ) = 0;
            
            virtual Buffer& operator<< ( const Stereo<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const Stereo<SampleInt32>& ) = 0;
            virtual Buffer& operator<< ( const Stereo<SampleFloat32>& ) = 0;
            virtual Buffer& operator<< ( const Stereo<SampleFloat64>& ) = 0;
    
            virtual Buffer& operator<< ( const Stereo21<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const Stereo21<SampleInt32>& ) = 0;
            virtual Buffer& operator<< ( const Stereo21<SampleFloat32>& ) = 0;
            virtual Buffer& operator<< ( const Stereo21<SampleFloat64>& ) = 0;
            
            virtual Buffer& operator<< ( const MultiChannel3<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel3<SampleInt32>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel3<SampleFloat32>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel3<SampleFloat64>& ) = 0;
            
            virtual Buffer& operator<< ( const MultiChannel4<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel4<SampleInt32>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel4<SampleFloat32>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel4<SampleFloat64>& ) = 0;
            
            virtual Buffer& operator<< ( const MultiChannel5<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel5<SampleInt32>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel5<SampleFloat32>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel5<SampleFloat64>& ) = 0;
            
            virtual Buffer& operator<< ( const MultiChannel6<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel6<SampleInt32>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel6<SampleFloat32>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel6<SampleFloat64>& ) = 0;

            virtual Buffer& operator<< ( const MultiChannel7<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel7<SampleInt32>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel7<SampleFloat32>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel7<SampleFloat64>& ) = 0;
            
            /* Readers */
            
            virtual Buffer& operator>> ( Mono<SampleInt16>& ) = 0;
            virtual Buffer& operator>> ( Mono<SampleInt32>& ) = 0;
            virtual Buffer& operator>> ( Mono<SampleFloat32>& ) = 0;
            virtual Buffer& operator>> ( Mono<SampleFloat64>& ) = 0;
            
            virtual Buffer& operator>> ( Stereo<SampleInt16>& ) = 0;
            virtual Buffer& operator>> ( Stereo<SampleInt32>& ) = 0;
            virtual Buffer& operator>> ( Stereo<SampleFloat32>& ) = 0;
            virtual Buffer& operator>> ( Stereo<SampleFloat64>& ) = 0;
            
            virtual Buffer& operator>> ( Stereo21<SampleInt16>& ) = 0;
            virtual Buffer& operator>> ( Stereo21<SampleInt32>& ) = 0;
            virtual Buffer& operator>> ( Stereo21<SampleFloat32>& ) = 0;
            virtual Buffer& operator>> ( Stereo21<SampleFloat64>& ) = 0;
            
            virtual Buffer& operator>> ( MultiChannel3<SampleInt16>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel3<SampleInt32>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel3<SampleFloat32>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel3<SampleFloat64>& ) = 0;
            
            virtual Buffer& operator>> ( MultiChannel4<SampleInt16>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel4<SampleInt32>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel4<SampleFloat32>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel4<SampleFloat64>& ) = 0;
            
            virtual Buffer& operator>> ( MultiChannel5<SampleInt16>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel5<SampleInt32>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel5<SampleFloat32>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel5<SampleFloat64>& ) = 0;
            
            virtual Buffer& operator>> ( MultiChannel6<SampleInt16>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel6<SampleInt32>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel6<SampleFloat32>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel6<SampleFloat64>& ) = 0;
            
            virtual Buffer& operator>> ( MultiChannel7<SampleInt16>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel7<SampleInt32>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel7<SampleFloat32>& ) = 0;
            virtual Buffer& operator>> ( MultiChannel7<SampleFloat64>& ) = 0;
            
            /* --- Buffer Ops Overloads --- */
            
            /*
             * These are buffer sized operators. Unlike the frame-sized shift operators, overhead is not
             * a critical consideration since the number of frames processed will be very high.
             */
            
            virtual Buffer &operator<< ( const Buffer& ) = 0;
            virtual Buffer &operator<< ( RawBuffer& ) = 0;
            
            virtual Buffer &operator>> ( Buffer& ) = 0;
            virtual Buffer &operator>> ( RawBuffer& ) = 0;
            
            //virtual Buffer &operator* ( const Buffer& ) = 0;
            
        protected:
            
            BufferFormat mFormat;
            BufferLength mLength;

            // Timestamp
            Duration mTimestamp;
            
            // Buffer flags
            StreamFlags mFlags;
            
            unsigned int mWriteIndex;
            unsigned int mReadIndex;
        };

        
        
        template<typename T>
        class TypedBuffer : public Buffer
        {
            // Befriend the other types of TypedBuffers.
            template< typename S > friend class TypedBuffer;
            
        public:
         
            TypedBuffer( const BufferFormat &format, const BufferLength &length );
            ~TypedBuffer();
            
            virtual SampleFormat sampleFormat() const;
            
            /* Writers */
            virtual TypedBuffer<T>& operator<< (const Mono<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< (const Mono<SampleInt32>& );
            virtual TypedBuffer<T>& operator<< (const Mono<SampleFloat32>& );
            virtual TypedBuffer<T>& operator<< (const Mono<SampleFloat64>& );
            
            virtual TypedBuffer<T>& operator<< ( const Stereo<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const Stereo<SampleInt32>& );
            virtual TypedBuffer<T>& operator<< ( const Stereo<SampleFloat32>& );
            virtual TypedBuffer<T>& operator<< ( const Stereo<SampleFloat64>& );

            virtual TypedBuffer<T>& operator<< ( const Stereo21<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const Stereo21<SampleInt32>& );
            virtual TypedBuffer<T>& operator<< ( const Stereo21<SampleFloat32>& );
            virtual TypedBuffer<T>& operator<< ( const Stereo21<SampleFloat64>& );
            
            virtual TypedBuffer<T>& operator<< ( const MultiChannel3<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel3<SampleInt32>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel3<SampleFloat32>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel3<SampleFloat64>& );
            
            virtual TypedBuffer<T>& operator<< ( const MultiChannel4<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel4<SampleInt32>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel4<SampleFloat32>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel4<SampleFloat64>& );
            
            virtual TypedBuffer<T>& operator<< ( const MultiChannel5<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel5<SampleInt32>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel5<SampleFloat32>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel5<SampleFloat64>& );

            virtual TypedBuffer<T>& operator<< ( const MultiChannel6<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel6<SampleInt32>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel6<SampleFloat32>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel6<SampleFloat64>& );
            
            virtual TypedBuffer<T>& operator<< ( const MultiChannel7<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel7<SampleInt32>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel7<SampleFloat32>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel7<SampleFloat64>& );
            
            virtual TypedBuffer<T> &operator<< ( const Buffer& );
            virtual TypedBuffer<T> &operator<< ( RawBuffer& );

            
            template<typename InSampleType>
            force_inline void write( const Mono<InSampleType> &frame );
            
            template<typename InSampleType>
            force_inline void write( const Stereo<InSampleType> &frame );
            
            template<typename InSampleType>
            force_inline void write( const Stereo21<InSampleType> &frame );
            
            template<typename InSampleType>
            force_inline void write( const MultiChannel3<InSampleType> &frame );
            
            template<typename InSampleType>
            force_inline void write( const MultiChannel4<InSampleType> &frame );
            
            template<typename InSampleType>
            force_inline void write( const MultiChannel5<InSampleType> &frame );
            
            template<typename InSampleType>
            force_inline void write( const MultiChannel6<InSampleType> &frame );
            
            template<typename InSampleType>
            force_inline void write( const MultiChannel7<InSampleType> &frame );
            
            template<typename InSampleType>
            void write( const TypedBuffer<InSampleType> &buffer );

            void write( RawBuffer &buffer );

            
            /* Readers */
            virtual TypedBuffer<T>& operator>> ( Mono<SampleInt16>& );
            virtual TypedBuffer<T>& operator>> ( Mono<SampleInt32>& );
            virtual TypedBuffer<T>& operator>> ( Mono<SampleFloat32>& );
            virtual TypedBuffer<T>& operator>> ( Mono<SampleFloat64>& );
            
            virtual TypedBuffer<T>& operator>> ( Stereo<SampleInt16>& );
            virtual TypedBuffer<T>& operator>> ( Stereo<SampleInt32>& );
            virtual TypedBuffer<T>& operator>> ( Stereo<SampleFloat32>& );
            virtual TypedBuffer<T>& operator>> ( Stereo<SampleFloat64>& );
            
            virtual TypedBuffer<T>& operator>> ( Stereo21<SampleInt16>& );
            virtual TypedBuffer<T>& operator>> ( Stereo21<SampleInt32>& );
            virtual TypedBuffer<T>& operator>> ( Stereo21<SampleFloat32>& );
            virtual TypedBuffer<T>& operator>> ( Stereo21<SampleFloat64>& );
            
            virtual TypedBuffer<T>& operator>> ( MultiChannel3<SampleInt16>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel3<SampleInt32>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel3<SampleFloat32>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel3<SampleFloat64>& );
            
            virtual TypedBuffer<T>& operator>> ( MultiChannel4<SampleInt16>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel4<SampleInt32>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel4<SampleFloat32>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel4<SampleFloat64>& );
            
            virtual TypedBuffer<T>& operator>> ( MultiChannel5<SampleInt16>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel5<SampleInt32>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel5<SampleFloat32>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel5<SampleFloat64>& );
            
            virtual TypedBuffer<T>& operator>> ( MultiChannel6<SampleInt16>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel6<SampleInt32>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel6<SampleFloat32>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel6<SampleFloat64>& );
            
            virtual TypedBuffer<T>& operator>> ( MultiChannel7<SampleInt16>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel7<SampleInt32>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel7<SampleFloat32>& );
            virtual TypedBuffer<T>& operator>> ( MultiChannel7<SampleFloat64>& );
            
            virtual TypedBuffer<T>& operator>> ( Buffer& );
            virtual TypedBuffer<T>& operator>> ( RawBuffer& );

            
            template<typename OutSampleType>
            force_inline void read( Mono<OutSampleType> &frame );
            
            template<typename OutSampleType>
            force_inline void read( Stereo<OutSampleType> &frame );
            
            template<typename OutSampleType>
            force_inline void read( Stereo21<OutSampleType> &frame );
            
            template<typename OutSampleType>
            force_inline void read( MultiChannel3<OutSampleType> &frame );
            
            template<typename OutSampleType>
            force_inline void read( MultiChannel4<OutSampleType> &frame );
            
            template<typename OutSampleType>
            force_inline void read( MultiChannel5<OutSampleType> &frame );
            
            template<typename OutSampleType>
            force_inline void read( MultiChannel6<OutSampleType> &frame );
            
            template<typename OutSampleType>
            force_inline void read( MultiChannel7<OutSampleType> &frame );
            
            template<typename OutSampleType>
            void read( TypedBuffer<OutSampleType> &buffer );
            
            void read( RawBuffer &buffer );
            
        private:
            STARGAZER_DISALLOW_COPY_AND_ASSIGN(TypedBuffer<T>);
            
            typedef T* ChannelMap[kMaximumChannels];
            

            ChannelMap mChannels = { nullptr };
            
            /**
             *  Builds the channel map for the requested channels using the specified
             *  buffer.
             */
            void buildChannelMap( ChannelMap map, Channels channels, T* base, unsigned int stride );
            
            
            template<typename InSampleType>
            void writeChannel( Channel ch, T &os, InSampleType is );

            template<typename OutSampleType>
            void readChannel( Channel ch, T &is, OutSampleType &os );
            
        };
        
        
        extern template class TypedBuffer<SampleInt16>;
        extern template class TypedBuffer<SampleInt32>;
        extern template class TypedBuffer<SampleFloat32>;
        extern template class TypedBuffer<SampleFloat64>;

        typedef TypedBuffer<SampleInt16> Int16Buffer;
        typedef TypedBuffer<SampleInt32> Int32Buffer;
        typedef TypedBuffer<SampleFloat32> Float32Buffer;
        typedef TypedBuffer<SampleFloat64> Float64Buffer;
        
    }
    
}

#endif
