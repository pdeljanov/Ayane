#ifndef STARGAZER_STDLIB_AUDIO_BUFFER_H_
#define STARGAZER_STDLIB_AUDIO_BUFFER_H_

/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-08.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */

/** @file audiobuffer.h
 * \brief Encapsulates a buffer of audio data.
 **/

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
    
        
        class Buffer
        {

        public:
            
            /** Enumeration of fixed point sample depths. **/
            enum FixedPointDepth
            {
                Depth24bit,
                Deptch32bit
            };
            
            Buffer ( const BufferFormat &format, const BufferLength &length );
            Buffer ( const Buffer &source );

            virtual ~Buffer();
            
            /** Get the audio buffer's timestamp.  A buffer's timestamp is the duration of time elapsed since the start
             * of the stream.
             **/
            const Duration &timestamp() const;
            
            void setTimestamp ( const Duration &timestamp );
            
            /** Get the duration of the buffer.
             **/
            Duration duration() const;

            
            /** Returns the amount of frames the buffer can contain. **/
            unsigned int frames() const;
            
            
            /**
             * Returns the multichannel audio format mode descriptor.
             *
             * \return The multichannel audio format.
             **/
            const BufferFormat &format() const;
            

            /** Replaces the contents of this buffer with the contents of the source buffer.
             *
             * The source buffer must have the same sample rate or else the copy will fail.
             * If the source buffer contains more samples than this one, the copy will be truncated.
             *
             * \param source The source buffer to copy from.
             *
             * \return True on success, false otherwise.
             **/
            bool copy ( const Buffer &source );
            
            
            /**
             *  Gets the sample format of the buffer.
             */
            //virtual SampleFormat sampleFormat() const = 0;
            
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
            
            // Mono
            virtual Buffer& operator<< (const Mono<SampleInt16>& ) = 0;
            virtual Buffer& operator<< (const Mono<SampleFloat32>& ) = 0;
            
            // Stereo20
            virtual Buffer& operator<< ( const Stereo20<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const Stereo20<SampleFloat32>& ) = 0;
    
            // Stereo21
            
        protected:
            
            BufferFormat m_format;
            BufferLength m_length;

            // Timestamp
            Duration m_timestamp;
            
        };
        
        
        template<typename T>
        class TypedBuffer : public Buffer
        {
        public:
            
            TypedBuffer( const BufferFormat &format, const BufferLength &length );
            ~TypedBuffer();
            
            /** Gets the underlying sample format identifier for the buffer. */
            //static SampleFormat identifier() { return Int16; }
            
            //virtual force_inline SampleFormat sampleFormat() const
            //{ return Int16Buffer::identifier(); }
            
            // Mono
            virtual TypedBuffer<T>& operator<< (const Mono<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< (const Mono<SampleFloat32>& );
            
            // Stereo20
            virtual TypedBuffer<T>& operator<< ( const Stereo20<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const Stereo20<SampleFloat32>& );
            
            /**
             *  Writes a frame of audio data to the buffer. Any necessary conversions
             *  are handled automatically. This is equivalent to using the left shift
             *  operator.
             *  @param frame The audio frame.
             */
            template<typename InFrameType>
            force_inline TypedBuffer<T> &write( const InFrameType &frame )
            {
                m_mapper.write(frame);
                return (*this);
            }
            
        private:
            T *m_buffer;
            Mapper<T> m_mapper;
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
