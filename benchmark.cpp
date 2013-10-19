
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



int main(int argc, const char *argv[] )
{
    BufferFormat format( kStereo20, 48000 );
    BufferLength length( (unsigned int)NUMBER_OF_ELEMENTS );

    Stereo<SampleFloat32> f;
    f.FL = 0.5f;
    f.FR = 0.5f;
    
    
    Stopwatch watch;
    
    
    
    int N = 20;
    double accumulator = 0.0f;
    
    for( int n = 0; n < N; ++n )
    {
        Buffer *b =  BufferFactory::make(Float32, format, length);
        
        watch.Restart();
        
        for( int i = 0; i < NUMBER_OF_ELEMENTS; ++i )
        {
            *b << f;
        }
        
        double result = watch.Secs();
        accumulator += result;

        delete b;

        std::cout << "Run " << n << ": " << 1000000*result << std::endl;
    }
    
    double avg = accumulator / N;
    std::cout << "Average: " << 1000000*avg << std::endl;
    
    return 0;
    
}
