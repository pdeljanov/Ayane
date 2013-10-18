#ifndef STARGAZER_STDLIB_AUDIO_ABSTRACTSTAGE_H_
#define STARGAZER_STDLIB_AUDIO_ABSTRACTSTAGE_H_

#include "formats.h"
#include "channels.h"

#include <vector>

namespace Stargazer {
    namespace Audio {


        
        
        /**
         *  A Port represents an input or output for a stage.
         */
        class Port
        {
        public:
            
            
            /**
             *  Enumeration of port types.
             */
            typedef enum
            {
                
                /** Port acts as a source (producer) of audio data. **/
                kSource = 0,
                
                /** Port acts as a sink (consumer) of audio data. **/
                kSink
                
            } Type;
            
            /**
             *  Enumeration of port availability.
             */
            typedef enum
            {
                
                /** The port is always available. **/
                kAlways = 0,
                
                /** The port is dynamically added (and removed) by the stage. **/
                kDynamic,
                
                /** The port is available upon request by the application. **/
                kOnDemand
                
            } Availability;
            
            /**
             *  Enumeration of port link status.
             */
            typedef enum
            {
                /** The port is unlinked. **/
                kUnlinked = 0,
                
                /** The port is linked, but not negotiated. **/
                kLinked,
                
                /** The port is linked, and negotiated. **/
                kNegotiated
                
            } LinkStatus;
            
            /** List of supported sample formats. **/
            typedef std::vector<SampleFormat> SupportedSampleFormats;
            
            /** List of supported channel layouts. **/
            typedef std::vector<Channels> SupportedChannels;
            
            /** List of supported sample rates. **/
            typedef std::vector<SampleRate> SupportedSampleRates;
            
            
            
            const SupportedSampleFormats &supportedSampleFormats() const;
            const SupportedChannels &supportedChannels() const;
            const SupportedSampleRates &supportedSampleRates() const;

            bool supportsAnyChannelLayout() const;
            bool supportsAnySampleRate() const;

            
            /** Gets the availability of the port. **/
            Availability availability() const;
            
            /** Gets the port type. **/
            Type type() const;
            
            /** Returns the link status of the port. **/
            LinkStatus linkStatus() const;
            

            
            static void link( Port *port );
            
            void unlink();
            
            
        };
        
        
        
        
    }
}

#endif
