/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "trace.h"

#include <iostream>

using namespace Stargazer::Audio;

#define ANSI_BOLD "\033[1m"

#define ANSI_BLACK "\033[30m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37"

#define ANSI_COLOUR_END "\033[39m"

class NullOutputBuffer : public std::streambuf {
public:
    virtual std::streamsize xsputn (const char * s, std::streamsize n) {
#pragma unused(s)
        return n;
    }
    virtual int overflow (int c) {
#pragma unused(c)
        return 1;
    }
};

class NullOutputStream : public std::ostream {
public:
    NullOutputStream() : std::ostream(&buf) {}
private:
    NullOutputBuffer buf;
};

NullOutputStream cnul;



Trace::Trace() : mMaximumPriority(kTrace)
{
}

Trace::~Trace()
{
}

Trace::Priority Trace::priority() const {
    return mMaximumPriority;
}

void Trace::setPriority(Priority priority) {
    mMaximumPriority = priority;
}

std::ostream &Trace::trace(const char *signature) {
    if(mMaximumPriority >= kTrace){
        std::cout << ANSI_GREEN << signature << ": " ANSI_COLOUR_END;
        return std::cout;
    }
    else {
        return cnul;
    }
}

std::ostream &Trace::trace(const char *signature, const void *instance) {
    if(mMaximumPriority >= kTrace){
        std::cout << ANSI_GREEN << "(" << instance << ") " << signature << ": " ANSI_COLOUR_END;
        return std::cout;
    }
    else {
        return cnul;
    }
}


std::ostream &Trace::info(const char *signature) {
    if(mMaximumPriority >= kInfo){
        
        std::cout << ANSI_CYAN << signature << ": " ANSI_COLOUR_END;
        return std::cout;
    }
    else {
        return cnul;
    }
}

std::ostream &Trace::info(const char *signature, const void *instance) {
    if(mMaximumPriority >= kInfo){
        
        std::cout << ANSI_CYAN << "(" << instance << ") " << signature << ": " ANSI_COLOUR_END;
        return std::cout;
    }
    else {
        return cnul;
    }
}


std::ostream &Trace::notice(const char *signature){
    if(mMaximumPriority >= kNotice){
        
        std::cout << ANSI_BLUE << signature << ": " ANSI_COLOUR_END;
        return std::cout;
    }
    else {
        return cnul;
    }
}

std::ostream &Trace::notice(const char *signature, const void *instance){
    if(mMaximumPriority >= kNotice){
        
        std::cout << ANSI_BLUE << "(" << instance << ") " << signature << ": " ANSI_COLOUR_END;
        return std::cout;
    }
    else {
        return cnul;
    }
}


std::ostream &Trace::warning(const char *signature){
    if(mMaximumPriority >= kWarning){
        
        std::cout << ANSI_YELLOW << signature << ": " ANSI_COLOUR_END;
        return std::cout;
    }
    else {
        return cnul;
    }
}

std::ostream &Trace::warning(const char *signature, const void *instance){
    if(mMaximumPriority >= kWarning){
        
        std::cout << ANSI_YELLOW << "(" << instance << ") " << signature << ": " ANSI_COLOUR_END;
        return std::cout;
    }
    else {
        return cnul;
    }
}



std::ostream &Trace::error(const char *signature){
    if(mMaximumPriority >= kError){
        
        std::cout << ANSI_RED << signature << ": " ANSI_COLOUR_END;
        return std::cout;
    }
    else {
        return cnul;
    }
}

std::ostream &Trace::error(const char *signature, const void *instance){
    if(mMaximumPriority >= kError){
        std::cout << ANSI_RED << "(" << instance << ") " << signature << ": " ANSI_COLOUR_END;
        return std::cout;
    }
    else {
        return cnul;
    }
}



