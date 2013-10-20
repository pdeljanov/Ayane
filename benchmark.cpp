
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
#define NUM_CHANNELS 7

int main(int argc, const char *argv[] )
{
    BufferFormat format( kSurround71, 48000 );
    BufferLength length( (unsigned int)NUMBER_OF_ELEMENTS );
    
    
    

    
    Surround70<SampleFloat32> f;
    f.FL = 1.00f;
    f.FR = 2.00f;
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
        Float32Buffer *b2 = new Float32Buffer(format, length);
        
        podSrc = new float[NUM_CHANNELS*NUMBER_OF_ELEMENTS];
        podDest = new float[NUM_CHANNELS*NUMBER_OF_ELEMENTS];
        
        // MEMCPY BENCHMARK
        
        watch.Restart();
        memcpy(podDest, podSrc, sizeof(float)*NUM_CHANNELS*NUMBER_OF_ELEMENTS);
        double resultMem = watch.Secs();
        
        watch.Restart();
        
        // FOR LOOP BENCHMARK
        
        watch.Restart();
        for( size_t i = 0; i < (NUM_CHANNELS*NUMBER_OF_ELEMENTS); ++i)
            podDest[i] = podSrc[i];
        double resultFor = watch.Secs();
        
        // INDIVIDUAL INSERT BENCHMARK
        
        watch.Restart();
        
        for( size_t i = 0; i < NUMBER_OF_ELEMENTS; ++i )
            *b << f;
        
        double result = watch.Secs();
        if( n >= IGNORE_FIRST )
            accumulator += result;

        
        // BUFFER INSERT BENCHMARK
        watch.Restart();
        *b2 << *b;
        double resultBuf = watch.Secs();
        
        
        delete b;
        delete b2;
        delete podSrc;
        delete podDest;

        std::cout << "Run (Ayane v2 Ind.) " << n << ": " << 1000000*result << std::endl;
        std::cout << "Run (Ayane v2 Buf.) " << n << ": " << 1000000*resultBuf << std::endl;
        std::cout << "Run (memcpy) " << n << ": " << 1000000*resultMem << std::endl;
        std::cout << "Run (for copy) " << n << ": " << 1000000*resultFor << std::endl;
        std::cout << "---" << std::endl;
    }
    
    double avg = accumulator / (N - IGNORE_FIRST);
    std::cout << "Average: " << 1000000*avg << std::endl;
    
    return 0;
    
}
