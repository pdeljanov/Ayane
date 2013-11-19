/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef STARGAZER_STDLIB_AUDIO_TRACE_H_
#define STARGAZER_STDLIB_AUDIO_TRACE_H_

#include <core/macros.h>
#include <ostream>

#if defined(NDEBUG)
/** Executes a statement only in debug mode. */
#define DEBUG_ONLY(statement) ((void)0)
#else
/** Executes a statement only in debug mode. */
#define DEBUG_ONLY(statement) statement
#endif

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
            
            /** 
             *  Enumeration of priority levels that control what messages types
             *  will be printed.
             */
            typedef enum {
                
                /** No messages are printed. */
                kNone = 0,
                
                /** Error messages are printed. */
                kError,
                
                /** Error and warning messages are printed. */
                kWarning,
                
                /** Error, Warning, and Notice messages are printed. */
                kNotice,
                
                /** Error, Warning, Notice, and Info messages are printed. */
                kInfo,
                
                /** Error, Warning, Notice, Info and Trace messages are printed. */
                kTrace
                
            } Priority;
            
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
            STARGAZER_DISALLOW_DEFAULT_CTOR_COPY_AND_ASSIGN(Trace);

            Priority mMaximumPriority;
        };
        
        
    }
}

#endif