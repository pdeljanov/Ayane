#ifndef STARGAZER_STDLIB_AUDIO_BUFFERFRAMES_H_
#define STARGAZER_STDLIB_AUDIO_BUFFERFRAMES_H_

#include <cstdio>
#include <core/attributes.h>

#include "bufferformat.h"
#include "formats.h"

namespace Stargazer {
    namespace Audio {
        
        
        /** Enumeration of buffer storage schemes. */
        typedef enum
        {
            /** Interleaved storage. Frames are stored consecutively in a 
             *  single buffer. Each frame contains 1 sample for each channel.
             */
            kInterleaved = 0,
            
            /** Planar storage. The samples for the first channel are stored first
             *  for all the frames, then all the samples for the second channel, and
             *  so on.
             */
            kPlanar = 1,
            
            /** Scatter/Gather (Vectored) storage. Similar to planar except each channel
             *  has its own unique buffer in memory.
             */
            kScatterGather = 2
        }
        BufferStorageScheme;
        
        
        /**
         *  Mono represents a frame of audio with 1 channel.
         */
        template< typename SampleType >
        struct Mono
        {
        public:
            /** Mono channel (physical output is hardware dependent). */
            SampleType mono;
        };
        
        /**
         *  Stereo20 represents a frame of audio with 2.0 (Left, Right) channels.
         */
        template< typename SampleType >
        struct Stereo20
        {
        public:
            /** Left channel. */
            SampleType left;
            /** Right channel. */
            SampleType right;
        };
        
        /**
         *  Stereo21 represents a frame of audio with 2.1 (Left, Right, LFE) channels.
         */
        template< typename SampleType >
        struct Stereo21
        {
        public:
            /** Left channel. */
            SampleType left;
            /** Right channel. */
            SampleType right;
            /** Low frequency channel. */
            SampleType lfe;
        };
        
        
        
        /**
         *  Mapper maintains a set of pointers to each channel in a buffer.
         *  Channel pointers are mapped arbitrarily and are incremented in the specified
         *  scheme.
         */
        template< typename SampleType >
        class Mapper
        {
        private:
            // Buffer base address.
            SampleType *m_base;
            
            // Buffer offset array.
            size_t m_map[12] = { 0 };
            
        public:

            
            Mapper() : m_base(nullptr){}
            
        
            
            
            void reset( SampleType *base, const BufferFormat &format, BufferStorageScheme scheme )
            {
                if(scheme == kInterleaved)
                {
                    m_map[0] = 0;
                    m_map[1] = sizeof(SampleType);
                }
                
                // Store the base pointer.
                m_base = base;
            }
            
            template<typename InSampleType>
            force_inline void write( const Mono<InSampleType> &i )
            {
                m_base[m_map[0]] = SampleFormats::convertSample<InSampleType, SampleType>(i.mono);
                ++m_base;
            }
            
            template<typename InSampleType>
            force_inline void write( const Stereo20<InSampleType> &i )
            {
                m_base[m_map[0]] = SampleFormats::convertSample<InSampleType, SampleType>(i.left);
                m_base[m_map[1]] = SampleFormats::convertSample<InSampleType, SampleType>(i.right);
                ++m_base;
            }
            

            
            
        };
        
  

        
        
        

        
        /**
         *  BufferFrame30 represents a frame of audio with 3 independent channels.
         */
        template< typename SampleType >
        struct BufferFrame30
        {
        public:
            /** Left channel. */
            SampleType left;
            /** Right channel. */
            SampleType right;
            
            union
            {
                /** Front center channel. (3.0 Stereo only) */
                SampleType frontCenterSt;
                /** Back center channel (3.0 Surround only) */
                SampleType backCenterSd;
            };
            
            template<typename OutBufferSampleType>
            force_inline void write( Mapper<OutBufferSampleType> &mapper ) const
            {
                // TODO: Implement.
            }
        };
        
        /**
         *  BufferFrame31 represents a frame of audio with 3 independent channels and one
         *  low frequency channel.
         */
        template< typename SampleType >
        struct BufferFrame31
        {
        public:
            /** Left channel. */
            SampleType left;
            /** Right channel. */
            SampleType right;
            /** Low frequency channel. */
            SampleType lfe;
            
            union
            {
                /** Front center channel. (3.1 Stereo only) */
                SampleType frontCenterSt;
                /** Back center channel (3.1 Surround only) */
                SampleType backCenterSd;
            };
            
            template<typename OutBufferSampleType>
            force_inline void write( Mapper<OutBufferSampleType> &mapper ) const
            {
                // TODO: Implement.
            }
        };
        
        
        
        
        /**
         *  BufferFrame40 represents a frame of audio with 4 independent channels.
         */
        template< typename SampleType >
        struct BufferFrame40
        {
        public:
            /** Left channel. */
            SampleType left;
            /** Right channel. */
            SampleType right;
            
            union
            {
                /** Back center channel. (4.0 Surround only) */
                SampleType backCenterSd;
                /** Back right channel. (4.0 Quad only) */
                SampleType backRightQ;
            };
            
            union
            {
                /** Front center channel. (4.0 Surround only) */
                SampleType frontCenterSd;
                /** Back left channel. (4.0 Quad only) */
                SampleType backLeftQd;
            };
            
            template<typename OutBufferSampleType>
            force_inline void write( Mapper<OutBufferSampleType> &mapper ) const
            {
                // TODO: Implement.
            }
        };
        
        /**
         *  BufferFrame41 represents a frame of audio with 3 independent channels and one
         *  low frequency channel.
         */
        template< typename SampleType >
        struct BufferFrame41
        {
        public:
            /** Left channel. */
            SampleType left;
            /** Right channel. */
            SampleType right;
            /** Low frequency channel. */
            SampleType lfe;
            
            union
            {
                /** Back center channel. (4.0 Surround only) */
                SampleType backCenterSd;
                /** Back right channel. (4.0 Quad only) */
                SampleType backRightQd;
            };
            
            union
            {
                /** Front center channel. (4.0 Surround only) */
                SampleType frontCenterSd;
                /** Back left channel. (4.0 Quad only) */
                SampleType backLeftQd;
            };
            
            template<typename OutBufferSampleType>
            force_inline void write( Mapper<OutBufferSampleType> &mapper ) const
            {
                // TODO: Implement.
            }
        };
        
        
        
        /**
         *  BufferFrame50 represents a frame of audio with 5 independent channels.
         */
        template< typename SampleType >
        struct BufferFrame50
        {
        public:
            /** Left channel. */
            SampleType left;
            /** Right channel. */
            SampleType right;
            /** Front center channel. */
            SampleType frontCenter;
            
            union
            {
                /** Back left channel. (5.0 Surround only) */
                SampleType backLeftSd;
                /** Side left channel. (5.0 Side only) */
                SampleType sideLeftSi;
            };
            
            union
            {
                /** Back right channel. (5.0 Surround only) */
                SampleType backRightSd;
                /** Side right channel. (5.0 Side only) */
                SampleType sideRightSi;
            };
            
            template<typename OutBufferSampleType>
            force_inline void write( Mapper<OutBufferSampleType> &mapper ) const
            {
                // TODO: Implement.
            }
        };
        
        
        /**
         *  BufferFrame51 represents a frame of audio with 5 independent channels and one
         *  low frequency channel.
         */
        template< typename SampleType >
        struct BufferFrame51
        {
        public:
            /** Left channel. */
            SampleType left;
            /** Right channel. */
            SampleType right;
            /** Front center channel. */
            SampleType frontCenter;
            
            union
            {
                /** Back left channel. (5.1 Surround only) */
                SampleType backLeftSd;
                /** Side left channel. (5.1 Side only) */
                SampleType sideLeftSi;
            };
            
            union
            {
                /** Back right channel. (5.1 Surround only) */
                SampleType backRightSd;
                /** Side right channel. (5.1 Side only) */
                SampleType sideRightSi;
            };
            
            template<typename OutBufferSampleType>
            force_inline void write( Mapper<OutBufferSampleType> &mapper ) const
            {
                // TODO: Implement.
            }
        };
        
        
        
        
        /**
         *  BufferFrame60 represents a frame of audio with 6 independent channels.
         */
        template< typename SampleType >
        struct BufferFrame60
        {
        public:
            /** Left channel. */
            SampleType left;
            /** Right channel. */
            SampleType right;
            /** Back center channel. */
            SampleType backCenter;
            /** Front center channel. */
            SampleType frontCenter;
            
            union
            {
                /** Back left channel. (6.0 Surround only) */
                SampleType backLeftSd;
                /** Side left channel. (6.0 Side only) */
                SampleType sideLeftSi;
            };
            
            union
            {
                /** Back right channel. (6.0 Surround only) */
                SampleType backRightSd;
                /** Side right channel. (6.0 Side only) */
                SampleType sideRightSi;
            };
            
            template<typename OutBufferSampleType>
            force_inline void write( Mapper<OutBufferSampleType> &mapper ) const
            {
                // TODO: Implement.
            }
        };
        
        
        /**
         *  BufferFrame61 represents a frame of audio with 6 independent channels and one
         *  low frequency channel.
         */
        template< typename SampleType >
        struct BufferFrame61
        {
        public:
            /** Left channel. */
            SampleType left;
            /** Right channel. */
            SampleType right;
            /** Back center channel. */
            SampleType backCenter;
            /** Front center channel. */
            SampleType frontCenter;
            
            union
            {
                /** Back left channel. (6.1 Surround only) */
                SampleType backLeftSd;
                /** Side left channel. (6.1 Side only) */
                SampleType sideLeftSi;
            };
            
            union
            {
                /** Back right channel. (6.1 Surround only) */
                SampleType backRightSd;
                /** Side right channel. (6.1 Side only) */
                SampleType sideRightSi;
            };
            
            template<typename OutBufferSampleType>
            force_inline void write( Mapper<OutBufferSampleType> &mapper ) const
            {
                // TODO: Implement.
            }
        };
        
        
        
        
        
        
        /**
         *  BufferFrame70 represents a frame of audio with 7 independent channels.
         */
        template< typename SampleType >
        struct BufferFrame70
        {
        public:
            /** Left channel. */
            SampleType left;
            /** Right channel. */
            SampleType right;
            /** Front center channel. */
            SampleType frontCenter;
            
            union
            {
                /** Back left channel. (6.0 Surround only) */
                SampleType backLeftSd;
                /** Side left channel. (6.0 Side only) */
                SampleType sideLeftSi;
            };
            
            union
            {
                /** Back right channel. (6.0 Surround only) */
                SampleType backRightSd;
                /** Side right channel. (6.0 Side only) */
                SampleType sideRightSi;
            };
            
            template<typename OutBufferSampleType>
            force_inline void write( Mapper<OutBufferSampleType> &mapper ) const
            {
                // TODO: Implement.
            }
        };
        
        
        
        
    }
}

#endif