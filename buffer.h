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
#include <core/dpointer.h>

#include "bufferlength.h"
#include "bufferformat.h"
#include "rawbuffer.h"
#include "duration.h"
#include "formats.h"

namespace Stargazer
{
    namespace Audio
    {
        
        typedef enum
        {
            kInterleaved = 0
        }
        BufferStorageScheme;
        
        
        /**
         *  Mapper maintains a set of pointers to each channel in a buffer.
         *  Channel pointers are mapped arbitrarily and are incremented in the specified
         *  scheme.
         */
        template< typename SampleType >
        class Mapper
        {
        public:

            // Actual samples
            SampleType *left;
            SampleType *right;
            SampleType *lfe;
            
            Mapper() :
                left(nullptr), right(nullptr), lfe(nullptr) {}
            
            
            void reset( SampleType *base, const BufferFormat &format, BufferStorageScheme scheme )
            {
                if(scheme == kInterleaved)
                {
                    left = &base[0];
                    right = &base[1];
                    lfe = nullptr;
                }
            }
            
            force_inline Mapper& operator--()
            {
                --left;
                --right;
                --lfe;
                return (*this);
            }
            
            force_inline Mapper& operator++()
            {
                ++left;
                ++right;
                ++lfe;
                return (*this);
            }
            

        };
        
        /**
         *  Mono represents a frame of audio with 1 (Centre) channel.
         */
        template< typename SampleType >
        struct Mono
        {
        public:
            SampleType centre;
            
            template<typename OutBufferSampleType>
            force_inline void write( Mapper<OutBufferSampleType> &mapper ) const
            {
                *mapper.left = SampleFormats::convertSample<SampleType, OutBufferSampleType>(centre);
            }
            
        };
        
        /**
         *  Stereo20 represents a frame of audio with 2.0 (Left, Right) channels.
         */
        template< typename SampleType >
        struct Stereo20
        {
        public:
            SampleType left;
            SampleType right;
            
            template<typename OutBufferSampleType>
            force_inline void write( Mapper<OutBufferSampleType> &mapper ) const
            {
                // TODO: Check the mapper can do >=2 channels.
                *mapper.left = SampleFormats::convertSample<SampleType, OutBufferSampleType>(left);
                *mapper.right = SampleFormats::convertSample<SampleType, OutBufferSampleType>(right);
            }
            
        };
        
        /**
         *  Stereo21 represents a frame of audio with 2.1 (Left, Right, LFE) channels.
         */
        template< typename SampleType >
        struct Stereo21
        {
        public:
            SampleType *left;
            SampleType *right;
            SampleType *lfe;
            
            template<typename OutBufferSampleType>
            force_inline void write( Mapper<OutBufferSampleType> &mapper ) const
            {
                // TODO: Check the mapper can do >=2.1 channels.
                *mapper.left = SampleFormats::convertSample<SampleType, OutBufferSampleType>(left);
                *mapper.right = SampleFormats::convertSample<SampleType, OutBufferSampleType>(right);
                *mapper.lfe = SampleFormats::convertSample<SampleType, OutBufferSampleType>(lfe);
            }
        };
        

        
        
        
        
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
            virtual SampleFormat sampleFormat() const = 0;
            
            /* --- Shift Operator Overloads --- */
            
            /* Each shift operator will need to be implemented in the actual concrete buffer
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
             * margin. We'll accept the overhead to gain this performance.
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
            
            // The number of (format indepdenant) samples in the buffer.
            size_t m_samples;
  
            // Timestamp
            Duration m_timestamp;
            
        };
        
        

        
    }
    
}

#endif
