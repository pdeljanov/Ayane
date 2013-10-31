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

            // Friend concrete TypedBuffer<T> classes so that it may access Buffer's
            // internals.
            template<typename T>
            friend class TypedBuffer;
            
        public:
            
            /** Enumeration of fixed point sample depths. **/
            enum FixedPointDepth
            {
                Depth24bit,
                Deptch32bit
            };
            
            /** Buffer stream flags. */
            typedef enum
            {
                /** No specific flags set. */
                kNone = 0,
                
                /** Format negotiation required. */
                kFormatNegotiation = 1<<0
            }
            StreamFlag;
            
            typedef uint32_t StreamFlags;
            
            Buffer ( const BufferFormat &format, const BufferLength &length );
            Buffer ( const Buffer &source );

            virtual ~Buffer();
            
            /** Streams a buffer stream flag. */
            inline void setFlag( StreamFlag flag ){
                m_flags |= flag;
            }
            
            /** Unsets a buffer stream flag. */
            inline void unsetFlag( StreamFlag flag ){
                m_flags ^= flag;
            }
            
            /** Gets the buffer stream flags. */
            StreamFlags flags() const {
                return m_flags;
            }
            
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
            
            virtual Buffer& operator<< (const Mono<SampleInt16>& ) = 0;
            virtual Buffer& operator<< (const Mono<SampleFloat32>& ) = 0;
            
            virtual Buffer& operator<< ( const Stereo<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const Stereo<SampleFloat32>& ) = 0;
    
            virtual Buffer& operator<< ( const Stereo21<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const Stereo21<SampleFloat32>& ) = 0;
            
            virtual Buffer& operator<< ( const MultiChannel3<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel3<SampleFloat32>& ) = 0;
            
            virtual Buffer& operator<< ( const MultiChannel4<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel4<SampleFloat32>& ) = 0;
            
            virtual Buffer& operator<< ( const MultiChannel5<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel5<SampleFloat32>& ) = 0;
            
            virtual Buffer& operator<< ( const MultiChannel6<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel6<SampleFloat32>& ) = 0;

            virtual Buffer& operator<< ( const MultiChannel7<SampleInt16>& ) = 0;
            virtual Buffer& operator<< ( const MultiChannel7<SampleFloat32>& ) = 0;
            

            /* --- Buffer Ops Overloads --- */
            
            /*
             * These are buffer sized operators. Unlike the frame-sized shift operators, overhead is not
             * a critical consideration since the number of frames processed will be very high.
             */
            
            virtual Buffer &operator<< ( const Buffer& ) = 0;
            
            //virtual Buffer &operator* ( const Buffer& ) = 0;
            
        protected:
            
            BufferFormat m_format;
            BufferLength m_length;

            // Timestamp
            Duration m_timestamp;
            
            // Buffer flags
            StreamFlags m_flags;
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
            
            virtual TypedBuffer<T>& operator<< (const Mono<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< (const Mono<SampleFloat32>& );
            
            virtual TypedBuffer<T>& operator<< ( const Stereo<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const Stereo<SampleFloat32>& );

            virtual TypedBuffer<T>& operator<< ( const Stereo21<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const Stereo21<SampleFloat32>& );
            
            virtual TypedBuffer<T>& operator<< ( const MultiChannel3<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel3<SampleFloat32>& );
            
            virtual TypedBuffer<T>& operator<< ( const MultiChannel4<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel4<SampleFloat32>& );
            
            virtual TypedBuffer<T>& operator<< ( const MultiChannel5<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel5<SampleFloat32>& );

            virtual TypedBuffer<T>& operator<< ( const MultiChannel6<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel6<SampleFloat32>& );
            
            virtual TypedBuffer<T>& operator<< ( const MultiChannel7<SampleInt16>& );
            virtual TypedBuffer<T>& operator<< ( const MultiChannel7<SampleFloat32>& );
            
            virtual TypedBuffer<T> &operator<< ( const Buffer& );

            
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

            
        private:
            
            typedef T* ChannelMap[11];
            
            size_t m_wr;
            size_t m_rd;
            ChannelMap m_ch = { nullptr };
            
            /**
             *  Builds the channel map for the requested channels using the specified
             *  buffer.
             */
            void buildChannelMap( ChannelMap map, Channels channels, T* base, unsigned int stride );
            
            
            template<typename InSampleType>
            void writeChannel( Channel ch, T &output, InSampleType is );

            
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
