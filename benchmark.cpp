
//
//  main.cpp
//  Benchmark
//
//  Created by Philip Deljanov on 2013-05-10.
//  Copyright (c) 2013 Philip Deljanov. All rights reserved.
//

#include "buffer.h"

#include <vector>
#include <iostream>
#include <cstdint>
#include <exception>

#define NUMBER_OF_ELEMENTS 48000

class Stopwatch {
public:
    Stopwatch() : mClock( std::clock() ) {}
    
    std::clock_t Ticks() const {
        return std::clock() - mClock;
    }
    
    double Secs() const {
        double t = std::clock() - mClock;
        return t / CLOCKS_PER_SEC;
    }
    
    void Restart() {
        mClock = std::clock();
    }
    
private:
    std::clock_t mClock;
};

inline std::ostream & operator<<( std::ostream & os,
                                 const Stopwatch & c ) {
    return os << c.Secs();
}


using namespace Stargazer::Audio;


class BufferFactory
{
public:
    
    static Buffer* make( SampleFormat sampleFormat, const BufferFormat &format, const BufferLength &length )
    {
        switch (sampleFormat)
        {
            case Int16:
                return new Int16Buffer(format, length);
                
            case Float32:
                return new Float32Buffer(format, length);
                
            default:
                return nullptr;
        }
    }
    
};

float *podSrc;
float *podDest;

#define IGNORE_FIRST 2
#define NUM_CHANNELS 2

int main(int argc, const char *argv[] )
{
    BufferFormat format( kStereo20, 48000 );
    BufferLength length( (unsigned int)NUMBER_OF_ELEMENTS );
    
    
    

    
    Stereo<SampleFloat32> f;
    f.FL = -0.25f;
    f.FR = 0.50f;
    // FC
    //f.BL = -2.0f;
    //f.BR = 2.0f;
    // FLc
    // FRc
    //f.SL = -1.0f;
    //f.SR = 1.0f;
    
    
    Stopwatch watch;
    
    
    
    int N = 20;
    double accumulator = 0.0f;
    
    for( int n = 0; n < N; ++n )
    {
        //Buffer *b =  BufferFactory::make(Float32, format, length);
        Float32Buffer *b = new Float32Buffer(format, length);
        podSrc = new float[NUM_CHANNELS*NUMBER_OF_ELEMENTS];
        podDest = new float[NUM_CHANNELS*NUMBER_OF_ELEMENTS];
        
        watch.Restart();
        memcpy(podDest, podSrc, sizeof(float)*NUM_CHANNELS*NUMBER_OF_ELEMENTS);
        double resultMem = watch.Secs();
        
        watch.Restart();
        
        watch.Restart();
        for( size_t i = 0; i < (NUM_CHANNELS*NUMBER_OF_ELEMENTS); ++i)
            podDest[i] = podSrc[i];
        double resultFor = watch.Secs();
        
        watch.Restart();
        
        for( size_t i = 0; i < NUMBER_OF_ELEMENTS; ++i )
        {
            *b << f;
        }
        
        
        double result = watch.Secs();
        if( n >= IGNORE_FIRST )
            accumulator += result;

        delete b;
        delete podSrc;
        delete podDest;

        std::cout << "Run (Ayane) " << n << ": " << 1000000*result << std::endl;
        std::cout << "Run (memcpy) " << n << ": " << 1000000*resultMem << std::endl;
        std::cout << "Run (for) " << n << ": " << 1000000*resultFor << std::endl;

    }
    
    double avg = accumulator / (N - IGNORE_FIRST);
    std::cout << "Average: " << 1000000*avg << std::endl;
    
    return 0;
    
}
