#ifndef STARGAZER_STDLIB_AUDIO_BUFFERFRAMES_H_
#define STARGAZER_STDLIB_AUDIO_BUFFERFRAMES_H_

#include <cstdio>
#include <core/attributes.h>

#include "bufferformat.h"
#include "formats.h"

namespace Stargazer {
    namespace Audio {
        
        /**
         *  Mono represents a frame of audio with 1 channel.
         */
        template< typename SampleType >
        struct Mono
        {
        public:
            union
            {
                /** Mono channel (physical output is hardware dependent). */
                SampleType FC;
                SampleType raw[1];
            };
        };
        
        /**
         *  Stereo represents a frame of audio with 2 (Left, Right) channels.
         */
        template< typename SampleType >
        struct Stereo
        {
        public:
            union
            {
                /** Left channel. */
                SampleType FL;
                /** Right channel. */
                SampleType FR;
                /** Raw sample access */
                SampleType raw[2];
            };
        };
        
        /**
         *  Stereo21 represents a frame of audio with 2.1 (Left, Right, LFE) channels.
         */
        template< typename SampleType >
        struct Stereo21
        {
        public:
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Low frequency channel. */
            SampleType LFE;
        };
        
        /**
         *  Stereo30 represents a frame of audio with 3.0 (Left, Right, Front Center) channels.
         */
        template< typename SampleType >
        struct Stereo30
        {
        public:
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
        };
        
        /**
         *  Surround30 represents a frame of audio with 3.0 (Left, Right, Back Center) channels.
         */
        template< typename SampleType >
        struct Surround30
        {
        public:
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Back center channel */
            SampleType BC;
        };
        
        /**
         *  Stereo31 represents a frame of audio with 3.1 (Left, Right, Front Center, LFE) channels.
         */
        template< typename SampleType >
        struct Stereo31
        {
        public:
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Low frequency channel. */
            SampleType LFE;
        };
        
        /**
         *  Surround31 represents a frame of audio with 3.0 (Left, Right, Back Center, LFE) channels.
         */
        template< typename SampleType >
        struct Surround31
        {
        public:
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Back center channel */
            SampleType BC;
            /** Low frequency channel. */
            SampleType LFE;
        };
        
        /**
         *  Quad40 represents a frame of audio with 4.0 (Left, Right, Back Left, Back Right) channels.
         */
        template< typename SampleType >
        struct Quad40
        {
        public:
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Back left channel. */
            SampleType BL;
            /** Back right channel. */
            SampleType BR;
        };
        
        /**
         *  Surround40 represents a frame of audio with 4.0 (Left, Right, Back Center, Front Center) channels.
         */
        template< typename SampleType >
        struct Surround40
        {
        public:
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Back center channel. */
            SampleType BC;
            /** Front center channel. */
            SampleType FC;
        };

        /**
         *  Quad41 represents a frame of audio with 4.0 (Left, Right, Back Left, Back Right, LFE) channels.
         */
        template< typename SampleType >
        struct Quad41
        {
        public:
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Back right channel. */
            SampleType BR;
            /** Back left channel. */
            SampleType BL;
            /** Low frequency channel. */
            SampleType LFE;
        };
        
        /**
         *  Surround41 represents a frame of audio with 4.1 (Left, Right, Back Center, Front Center, LFE) channels.
         */
        template< typename SampleType >
        struct Surround41
        {
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Back center channel. */
            SampleType BC;
            /** Front center channel. */
            SampleType FC;
            /** Low frequency channel. */
            SampleType LFE;
        };
        
        /**
         *  Surround50 represents a frame of audio with 5.0 (Left, Right, Front Center, Back Left, Back Right) channels.
         */
        template< typename SampleType >
        struct Surround50
        {
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Back left channel. */
            SampleType BL;
            /** Back right channel. */
            SampleType BR;
        };

        /**
         *  Side50 represents a frame of audio with 5.0 (Left, Right, Front Center, Side Left, Side Right) channels.
         */
        template< typename SampleType >
        struct Side50
        {
        public:
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Side left channel.*/
            SampleType SL;
            /** Side right channel. */
            SampleType SR;
        };
        
        /**
         *  Surround51 represents a frame of audio with 5.0 (Left, Right, Front Center,
         *  Back Left, Back Right, LFE) channels.
         */
        template< typename SampleType >
        struct Surround51
        {
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Back left channel. */
            SampleType BL;
            /** Back right channel. */
            SampleType BR;
            /** Low frequency channel. */
            SampleType LFE;
        };
        
        /**
         *  Side51 represents a frame of audio with 5.1 (Left, Right, Front Center,
         *  Side Left, Side Right, LFE) channels.
         */
        template< typename SampleType >
        struct Side51
        {
        public:
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Side left channel. */
            SampleType SL;
            /** Side right channel. */
            SampleType SR;
            /** Low frequency channel. */
            SampleType LFE;
        };
        
        
        /**
         *  Surround60 represents a frame of audio with 6.0 (Left, Right, Front Center,
         *  Back Left, Back Right, Back Center) channels.
         */
        template< typename SampleType >
        struct Surround60
        {
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Back left channel. */
            SampleType BL;
            /** Back right channel. */
            SampleType BR;
            /** Back center channel. */
            SampleType BC;
        };
        
        /**
         *  Side60 represents a frame of audio with 6.0 (Left, Right, Front Center,
         *  Side Left, Side Right, Back Center) channels.
         */
        template< typename SampleType >
        struct Side60
        {
        public:
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Side left channel.*/
            SampleType SL;
            /** Side right channel. */
            SampleType SR;
            /** Back center channel. */
            SampleType BC;
        };
        
        /**
         *  Surround61 represents a frame of audio with 5.0 (Left, Right, Front Center,
         *  Back Left, Back Right, Back Center, LFE) channels.
         */
        template< typename SampleType >
        struct Surround61
        {
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Back left channel. */
            SampleType BL;
            /** Back right channel. */
            SampleType BR;
            /** Back center channel. */
            SampleType BC;
            /** Low frequency channel. */
            SampleType LFE;
        };
        
        /**
         *  Side61 represents a frame of audio with 5.0 (Left, Right, Front Center,
         *  Side Left, Side Right, Back Center, LFE) channels.
         */
        template< typename SampleType >
        struct Side61
        {
        public:
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Side left channel.*/
            SampleType SL;
            /** Side right channel. */
            SampleType SR;
            /** Back center channel. */
            SampleType BC;
            /** Low frequency channel. */
            SampleType LFE;
        };
        
        /**
         *  Front70 represents a frame of audio with 7.0 (Left, Right, Front Center,
         *  Back Left, Back Right, Front Left of Center, Front Right of Center) channels.
         */
        template< typename SampleType >
        struct Front70
        {
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Back left channel. */
            SampleType BL;
            /** Back right channel. */
            SampleType BR;
            /** Front left of center channel. */
            SampleType FLc;
            /** Front right of center channel. */
            SampleType FRc;
        };
        
        /**
         *  Side70 represents a frame of audio with 7.0 (Left, Right, Front Center,
         *  Front Left of Center, Front Right of Center, Side Left, Side Right) channels.
         */
        template< typename SampleType >
        struct Side70
        {
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Front left of center channel. */
            SampleType FLc;
            /** Front right of center channel. */
            SampleType FRc;
            /** Side left channel. */
            SampleType SL;
            /** Side right channel. */
            SampleType SR;
        };
        
        /**
         *  Surround70 represents a frame of audio with 7.0 (Left, Right, Front Center,
         *  Back Left, Back Right, Side Left, Side Right) channels.
         */
        template< typename SampleType >
        struct Surround70
        {
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Back left channel. */
            SampleType BL;
            /** Back right channel. */
            SampleType BR;
            /** Side left channel. */
            SampleType SL;
            /** Side right channel. */
            SampleType SR;
        };
        
        /**
         *  Front71 represents a frame of audio with 7.0 (Left, Right, Front Center,
         *  Back Left, Back Right, Front Left of Center, Front Right of Center, LFE)
         *  channels.
         */
        template< typename SampleType >
        struct Front71
        {
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Back left channel. */
            SampleType BL;
            /** Back right channel. */
            SampleType BR;
            /** Front left of center channel. */
            SampleType FLc;
            /** Front right of center channel. */
            SampleType FRc;
            /** Low frequency channel. */
            SampleType LFE;
        };
        
        /**
         *  Side71 represents a frame of audio with 7.0 (Left, Right, Front Center,
         *  Front Left of Center, Front Right of Center, Side Left, Side Right, LFE)
         *  channels.
         */
        template< typename SampleType >
        struct Side71
        {
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Front left of center channel. */
            SampleType FLc;
            /** Front right of center channel. */
            SampleType FRc;
            /** Side left channel. */
            SampleType SL;
            /** Side right channel. */
            SampleType SR;
            /** Low frequency channel. */
            SampleType LFE;
        };
        
        /**
         *  Surround71 represents a frame of audio with 7.0 (Left, Right, Front Center,
         *  Back Left, Back Right, Side Left, Side Right, LFE) channels.
         */
        template< typename SampleType >
        struct Surround71
        {
            /** Left channel. */
            SampleType FL;
            /** Right channel. */
            SampleType FR;
            /** Front center channel. */
            SampleType FC;
            /** Back left channel. */
            SampleType BL;
            /** Back right channel. */
            SampleType BR;
            /** Side left channel. */
            SampleType SL;
            /** Side right channel. */
            SampleType SR;
            /** Low frequency channel. */
            SampleType LFE;
        };
        
        /**
         *  MultiChannel3 represents a frame of audio with 3 independent channels.
         */
        template< typename SampleType >
        struct MultiChannel3
        {
        public:
            union
            {
                /** Stereo 2.1 */
                Stereo21<SampleType> st21;
                /** Stereo 3.0 */
                Stereo30<SampleType> st30;
                /** Surround 3.0 */
                Surround30<SampleType> s30;
                /** Raw sample access */
                SampleType raw[3];
            };
        };
        
        /**
         *  MultiChannel4 represents a frame of audio with 4 independent channels.
         */
        template< typename SampleType >
        struct MultiChannel4
        {
        public:
            union
            {
                /** Stereo 3.1 */
                Stereo31<SampleType> st31;
                /** Surround 3.1 */
                Surround31<SampleType> s31;
                /** Quad 4.0 */
                Quad40<SampleType> q40;
                /** Surround 4.0 */
                Surround40<SampleType> s40;
                /** Raw sample access */
                SampleType raw[4];
            };
        };
        
        /**
         *  MultiChannel5 represents a frame of audio with 5 independent channels.
         */
        template< typename SampleType >
        struct MultiChannel5
        {
        public:
            union
            {
                /** Quad 4.1 */
                Quad41<SampleType> q41;
                /** Surround 4.1 */
                Surround41<SampleType> s41;
                /** Surround 5.0 */
                Surround50<SampleType> s50;
                /** Surround 5.1 using side channels. */
                Side50<SampleType> s50si;
                /** Raw sample access */
                SampleType raw[5];
            };
        };
        
        /**
         *  MultiChannel6 represents a frame of audio with 6 independent channels.
         */
        template< typename SampleType >
        struct MultiChannel6
        {
        public:
            union
            {
                /** Surround 5.1 */
                Surround51<SampleType> s51;
                /** Surround 5.1 using side channels */
                Side51<SampleType> s51si;
                /** Surround 6.0 */
                Surround60<SampleType> s60;
                /** Surround 6.0 using side channels */
                Side60<SampleType> s60si;
                /** Raw sample access */
                SampleType raw[6];
            };
            
        };
        
        /**
         *  MultiChannel7 represents a frame of audio with 7 independent channels.
         */
        template< typename SampleType >
        struct MultiChannel7
        {
        public:
            union
            {
                /** Surround 6.1 */
                Surround61<SampleType> s61;
                /** Surround 6.1 using side channels */
                Side61<SampleType> s61si;
                /** Surround 7.0 using front of center channels */
                Front70<SampleType> s70fr;
                /** Surround 7.0 using side channels */
                Side70<SampleType> s70si;
                /** Surround 7.0 */
                Surround70<SampleType> s70;
                /** Raw sample access */
                SampleType raw[7];
            };
            
        };
        
        /**
         *  MultiChannel8 represents a frame of audio with 8 independent channels.
         */
        template< typename SampleType >
        struct MultiChannel8
        {
        public:
            union
            {
                /** Surround 7.1 using front of center channels */
                Front71<SampleType> s71fr;
                /** Surround 7.1 using side channels */
                Side71<SampleType> s71si;
                /** Surround 7.1 */
                Surround71<SampleType> s71;
                /** Raw sample access */
                SampleType raw[8];
            };
        };
        


        
        
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
            size_t m_map[13] = { 0 };
            
            /* Channel-to-Buffer map functions */
            force_inline SampleType &fl()  { return m_base[m_map[0]];  }
            force_inline SampleType &fr()  { return m_base[m_map[1]];  }
            force_inline SampleType &fc()  { return m_base[m_map[2]];  }
            force_inline SampleType &lfe() { return m_base[m_map[3]];  }
            force_inline SampleType &bl()  { return m_base[m_map[4]];  }
            force_inline SampleType &br()  { return m_base[m_map[5]];  }
            force_inline SampleType &flc() { return m_base[m_map[6]];  }
            force_inline SampleType &frc() { return m_base[m_map[7]];  }
            force_inline SampleType &bc()  { return m_base[m_map[8]];  }
            force_inline SampleType &sl()  { return m_base[m_map[9]];  }
            force_inline SampleType &sr()  { return m_base[m_map[10]]; }
            
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
                fl() = SampleFormats::convertSample<InSampleType, SampleType>(i.FC);
                ++m_base;
            }
            
            template<typename InSampleType>
            force_inline void write( const Stereo<InSampleType> &i )
            {
                fl() = SampleFormats::convertSample<InSampleType, SampleType>(i.FL);
                fr() = SampleFormats::convertSample<InSampleType, SampleType>(i.FR);
                ++m_base;
            }
            
            template<typename InSampleType>
            force_inline void write( const MultiChannel3<InSampleType> &i )
            {
                fl() = SampleFormats::convertSample<InSampleType, SampleType>(i.raw[0]);
                fr() = SampleFormats::convertSample<InSampleType, SampleType>(i.raw[1]);
                
                ++m_base;
            }

            
            
        };
        
  

        
        
        
        
    }
}

#endif