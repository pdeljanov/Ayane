/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef STARGAZER_STDLIB_AUDIO_TRACE_H_
#define STARGAZER_STDLIB_AUDIO_TRACE_H_

#include <mutex>
#include <ostream>


#define ERROR(signature)        Stargazer::Audio::Trace::instance().error(signature)
#define ERROR_THIS(signature)   Stargazer::Audio::Trace::instance().error(signature, this)

#define WARNING(signature)      Stargazer::Audio::Trace::instance().warning(signature)
#define WARNING_THIS(signature) Stargazer::Audio::Trace::instance().warning(signature, this)

#define NOTICE(signature)       Stargazer::Audio::Trace::instance().notice(signature)
#define NOTICE_THIS(signature)  Stargazer::Audio::Trace::instance().notice(signature, this)

#define INFO(signature)         Stargazer::Audio::Trace::instance().info(signature)
#define INFO_THIS(signature)    Stargazer::Audio::Trace::instance().info(signature, this)

#define TRACE(signature)        Stargazer::Audio::Trace::instance().trace(signature)
#define TRACE_THIS(signature)   Stargazer::Audio::Trace::instance().trace(signature, this)


namespace Stargazer {
    namespace Audio {
        
        class Trace {
            
        public:
            
            static Trace &instance() {
                static Trace trace;
                return trace;
            }
            
            typedef enum {
                kNone = 0,
                kError,
                kWarning,
                kNotice,
                kInfo,
                kTrace
            } Priority;
            
            Trace();
            ~Trace();
            
            Priority priority() const;
            void setPriority(Priority priority);
            
            
            std::ostream &trace(const char *signature);
            std::ostream &info(const char *signature);
            std::ostream &notice(const char *signature);
            std::ostream &warning(const char *signature);
            std::ostream &error(const char *signature);
            
            std::ostream &trace(const char *signature, const void *instance);
            std::ostream &info(const char *signature, const void *instance);
            std::ostream &notice(const char *signature, const void *instance);
            std::ostream &warning(const char *signature, const void *instance);
            std::ostream &error(const char *signature, const void *instance);
            
            
        private:
            Priority mMaximumPriority;
        };
        
        
    }
}

#endif