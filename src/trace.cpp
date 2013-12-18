/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "trace.h"

#include <sstream>
#include <iostream>

using namespace Ayane;

// Allow compiler flag to disable ANSI colour coding.
#ifndef AYANE_TRACE_NO_ANSI

#define ANSI_BOLD "\033[1m"

#define ANSI_BLACK "\033[30m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37m"

#define ANSI_COLOUR_END "\033[39m"

#else

#define ANSI_BOLD ""

#define ANSI_BLACK ""
#define ANSI_RED ""
#define ANSI_GREEN ""
#define ANSI_YELLOW ""
#define ANSI_BLUE ""
#define ANSI_MAGENTA ""
#define ANSI_CYAN ""
#define ANSI_WHITE ""

#define ANSI_COLOUR_END ""

#endif


class LockedOutputStream : public std::ostream {
public:
    
    LockedOutputStream(std::ostream &outputStream, std::mutex &mutex) :
        std::ostream(outputStream.rdbuf()),
        mMutex(mutex)
    {
        mMutex.lock();
    }
    
    explicit LockedOutputStream(LockedOutputStream &lockedStream) :
        std::ostream(lockedStream.rdbuf()),
        mMutex(lockedStream.mMutex)
    {
        
    }
    
    ~LockedOutputStream(){
        mMutex.unlock();
    }
    
private:
    std::mutex &mMutex;
};



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

        std::cout << ANSI_GREEN "(" << instance << ") " << signature << ": " ANSI_COLOUR_END;
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
        
        std::cout << ANSI_CYAN "(" << instance << ") " << signature << ": " ANSI_COLOUR_END;
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
        
        std::cout << ANSI_BLUE "(" << instance << ") " << signature << ": " ANSI_COLOUR_END;
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
        
        std::cout << ANSI_YELLOW "(" << instance << ") " << signature << ": " ANSI_COLOUR_END;
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
        std::cout << ANSI_RED "(" << instance << ") " << signature << ": " ANSI_COLOUR_END;
        return std::cout;
    }
    else {
        return cnul;
    }
}



