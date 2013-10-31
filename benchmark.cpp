
//
//  main.cpp
//  Benchmark
//
//  Created by Philip Deljanov on 2013-05-10.
//  Copyright (c) 2013 Philip Deljanov. All rights reserved.
//

#include "buffer.h"
#include "refcountedpool.h"
#include "abstractstage.h"

#include <vector>
#include <iostream>
#include <cstdint>
#include <exception>

#define NUMBER_OF_ELEMENTS 1920000

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
            case Int32:
                return new Int32Buffer(format, length);
            case Float32:
                return new Float32Buffer(format, length);
            case Float64:
                return new Float64Buffer(format, length);
            default:
                return nullptr;
        }
    }
    
};

float *podSrc;
float *podDest;

#define IGNORE_FIRST 2
#define NUM_CHANNELS 2

int benchmark()
{
    BufferFormat format( kStereo20, 48000 );
    BufferLength length( (unsigned int)NUMBER_OF_ELEMENTS );
    
    Stereo<SampleFloat32> f;
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

};

class PoolAllocator
{
    
public:
    int *operator()() const 
    {
        static int i = 1;
        int *out = new int;
        *out = (i*=2);
        std::cout << "[PoolAllocator] Allocated: " << *out << " @ " << out << std::endl;
        return out;
    };
};


class TestSource : public Stage
{
public:
    
    TestSource() : Stage(), m_dummy(nullptr)
    {
        addSource("test-source", new Stage::Source(*this));
    }
    
    virtual bool beginPlayback() {
        std::cout << "TestSource::beginPlayback" << std::endl;
        return true;
    }
    
    virtual bool stoppedPlayback() {
        std::cout << "TestSource::stoppedPlayback" << std::endl;
        return true;
    }
    
    virtual void process( const Clock &clock ){
        std::cout << "TestSource::process: Clock reads: "
        << clock.time() << "(delta=" << clock.deltaTime() << ")." << std::endl;
        
        // Return an incrementing buffer.
        
        auto b = std::shared_ptr<Buffer>( BufferFactory::make(Float32, m_format, m_length) );
        
        std::cout << "TestSource::process: Pushing buffer: " << b.get() << std::endl;
        
        output()->push(b);
    }
    
    virtual bool reconfigureSink( const Sink &sink ){
        std::cout << "TestSource::reconfigureSink" << std::endl;
        return true;
    }

    Source *output()
    {
        return m_sources["test-source"];
    }
    
    BufferFormat m_format;
    BufferLength m_length;
    int *m_dummy;
};


class TestDSP : public Stage
{
public:
    
    TestDSP() : Stage()
    {
        addSink("input", new Stage::Sink(*this));
        addSource("output", new Stage::Source(*this));
    }
    
    virtual bool beginPlayback() {
        std::cout << "TestDSP::beginPlayback" << std::endl;
        return true;
    }
    
    virtual bool stoppedPlayback() {
        std::cout << "TestDSP::stoppedPlayback" << std::endl;
        return true;
    }
    
    virtual void process( const Clock &clock ){
        std::cout << "TestDSP::process: Clock reads: "
        << clock.time() << "(delta=" << clock.deltaTime() << ")." << std::endl;
        
        std::shared_ptr<Buffer> b = input()->pull();
        std::cout << "TestDSP::process: Forwarding buffer: " << b.get() << std::endl;
        output()->push(b);
    }
    
    virtual bool reconfigureSink( const Sink &sink ){
        std::cout << "TestDSP::reconfigureSink" << std::endl;
        return true;
    }
    
    Sink *input()
    {
        return m_sinks["input"];
    }
    
    Source *output()
    {
        return m_sources["output"];
    }
};


class TestSink : public Stage
{
public:
    
    TestSink() : Stage()
    {
        addSink("test-sink", new Stage::Sink(*this));
        
        // Force an async connection on the input to emulate OS output behaviour.
        input()->setScheduling(Sink::kForceAsynchronous);
    }
    

    virtual bool beginPlayback() {
        std::cout << "TestSink::beginPlayback" << std::endl;
        return true;
    }
    
    virtual bool stoppedPlayback() {
        std::cout << "TestSink::stoppedPlayback" << std::endl;
        return true;
    }
    
    virtual void process( const Clock &clock ){
        std::cout << "TestSink::process: Clock reads: "
        << clock.time() << "(delta=" << clock.deltaTime() << ")." << std::endl;
        
        std::cout << "TestSink::process: Outputting: " << input()->pull().get() << "." << std::endl;
    }
    
    virtual bool reconfigureSink( const Sink &sink ){
        std::cout << "TestSink::reconfigureSink" << std::endl;
        return true;
    }
    
    Sink *input()
    {
        return m_sinks["test-sink"];
    }
    
};





int main(int argc, const char *argv[] )
{/*
    PoolAllocator p;
    RefCountedPool<int, PoolAllocator> pool = RefCountedPoolFactory::create<int, PoolAllocator>(p, 2);
        
    // Get int 1, give it back.
    PooledRefCount<int> i0 = pool->acquire();
    PooledRefCount<int> i1 = pool->acquire();
    PooledRefCount<int> i2 = pool->acquire();
    
    i1.reset();
    
    pool.reset();
    
    i2.reset();
  */
    
    std::unique_ptr<TestSource> src( new TestSource );
    std::unique_ptr<TestDSP> dsp( new TestDSP );
    std::unique_ptr<TestSink> sink( new TestSink );

    // Link.
    Stage::link( src->output(), dsp->input() );
    Stage::link( dsp->output(), sink->input() );
    
    src->activate();
    dsp->activate();
    sink->activate();
    
    src->play();
    dsp->play();
    sink->play();
    
    while( true ) {

        // Advance clock.
        
        char command;
        std::cin >> command;
        
        if( command == 'q' ) {
            break;
        }
        
    }
    
    src->stop();
    dsp->stop();
    sink->stop();
    
    // Unlink.
    Stage::unlink( src->output(), dsp->input() );
    Stage::unlink( dsp->output(), sink->input() );    
    

    
}




