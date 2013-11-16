
//
//  main.cpp
//  Benchmark
//
//  Created by Philip Deljanov on 2013-05-10.
//  Copyright (c) 2013 Philip Deljanov. All rights reserved.
//

#include "clockprovider.h"
#include "clockobserver.h"
#include "bufferfactory.h"
#include "pool.h"

#include <audio/rawbuffer.h>
#include <audio/stage.h>
#include <audio/host/mac/coreaudioendpoint.h>

#include <vector>
#include <iostream>
#include <cstdint>
#include <exception>

#include <mutex>

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

#define SYNCHRONIZE_COUT

#if defined(SYNCHRONIZE_COUT) 
static std::mutex mutex;

#define COUT( line ) \
{ std::lock_guard<std::mutex> lock(mutex); std::cout << line << std::endl; }

#else

#define COUT( line ) \
std::cout << line << std::endl;

#endif

/*
class TestSource : public Stage
{
public:
    
    TestSource() : Stage(), m_dummy(nullptr)
    {
        addSource("decoder", new Stage::Source(*this));
    }
    
    virtual bool beginPlayback() {
        std::cout << "TestSource::beginPlayback" << std::endl;
        return true;
    }
    
    virtual bool stoppedPlayback() {
        std::cout << "TestSource::stoppedPlayback" << std::endl;
        return true;
    }
    
    virtual void process(){
        COUT("TestSource::process: Clock reads: " << clock()->presentationTime()
             << " (delta=" << clock()->deltaTime() << ").")
        
        
        auto b = std::shared_ptr<Buffer>( BufferFactory::make(kFloat32, m_format, m_length) );
        Buffer *ptrForMessage = b.get();
        output()->push(b);
        
        COUT("TestSource::process: Pushed buffer: " << ptrForMessage)
    }
    
    virtual bool reconfigureSink( const Sink &sink ){
        COUT("TestSource::reconfigureSink")
        return true;
    }

    Source *output()
    {
        return m_sources["decoder"];
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
    
    virtual void process(){
        COUT("TestDSP::process: Clock reads: " << clock()->presentationTime()
             << " (delta=" << clock()->deltaTime() << ").")
        
        
        std::shared_ptr<Buffer> b = input()->pull();
        Buffer *ptrForMessage = b.get();
        output()->push(b);
        
        COUT("TestDSP::process: Forwarding buffer: " << ptrForMessage)
    }
    
    virtual bool reconfigureSink( const Sink &sink ){
        COUT("TestDSP::reconfigureSink")
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
        addSink("speakers", new Stage::Sink(*this));
        
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
    
    virtual void process(){
        COUT("TestSink::process: Clock reads: " << clock()->presentationTime()
             << " (delta=" << clock()->deltaTime() << ").")
        
        std::unique_ptr<Buffer> b( input()->pull() );
        
        COUT("TestSink::process: Outputting: " << b.get() << ".")
    }
    
    virtual bool reconfigureSink( const Sink &sink ){
        COUT("TestSink::reconfigureSink")
        return true;
    }
    
    Sink *input()
    {
        return m_sinks["speakers"];
    }
    
};

*/

static SampleInt16 kSineWave[240] = {
    0, 858, 1715, 2571, 3425, 4277, 5126, 5971, 6813, 7649, 8481, 9306, 10126,
    10938, 11743, 12539, 13328, 14107, 14876, 15635, 16383, 17121, 17846,
    18559, 19260, 19947, 20621, 21280, 21925, 22555, 23170, 23768, 24351,
    24916, 25465, 25996, 26509, 27004, 27481, 27938, 28377, 28796, 29196,
    29575, 29934, 30273, 30591, 30888, 31163, 31418, 31650, 31862, 32051,
    32218, 32364, 32487, 32587, 32666, 32722, 32756, 32767, 32756, 32722,
    32666, 32587, 32487, 32364, 32218, 32051, 31862, 31650, 31418, 31163,
    30888, 30591, 30273, 29934, 29575, 29196, 28796, 28377, 27938, 27481,
    27004, 26509, 25996, 25465, 24916, 24351, 23768, 23170, 22555, 21925,
    21280, 20621, 19947, 19260, 18559, 17846, 17121, 16383, 15635, 14876,
    14107, 13328, 12539, 11743, 10938, 10126, 9306, 8481, 7649, 6813,
    5971, 5126, 4277, 3425, 2571, 1715, 858, 0, -858, -1715, -2571,
    -3425, -4277, -5126, -5971, -6813, -7649, -8481, -9306, -10126,
    -10938, -11743, -12539, -13328, -14107, -14876, -15635, -16384,
    -17121, -17846, -18559, -19260, -19947, -20621, -21280, -21925,
    -22555, -23170, -23768, -24351, -24916, -25465, -25996, -26509,
    -27004, -27481, -27938, -28377, -28796, -29196, -29575, -29934,
    -30273, -30591, -30888, -31163, -31418, -31650, -31862, -32051,
    -32218, -32364, -32487, -32587, -32666, -32722, -32756, -32767,
    -32756, -32722, -32666, -32587, -32487, -32364, -32218, -32051,
    -31862, -31650, -31418, -31163, -30888, -30591, -30273, -29934,
    -29575, -29196, -28796, -28377, -27938, -27481, -27004, -26509,
    -25996, -25465, -24916, -24351, -23768, -23170, -22555, -21925,
    -21280, -20621, -19947, -19260, -18559, -17846, -17121, -16384,
    -15635, -14876, -14107, -13328, -12539, -11743, -10938, -10126,
    -9306, -8481, -7649, -6813, -5971, -5126, -4277, -3425, -2571,
    -1715, -858 };


int main(int argc, const char *argv[] )
{
#pragma unused(argc)
#pragma unused(argv)
    
/*
    // Will be done by pipeline.
    std::unique_ptr<ClockProvider> cp( new ClockProvider() );
    Clock *sinkClock = new Clock();
    Clock *dspClock = new Clock();
    
    cp->registerClock(sinkClock);
    cp->registerClock(dspClock);
    //
    
    std::unique_ptr<TestSource> src( new TestSource );
    std::unique_ptr<TestDSP> dsp( new TestDSP );
    std::unique_ptr<TestSink> sink( new TestSink );

    // Link.
    Stage::link( src->output(), dsp->input() );
    Stage::link( dsp->output(), sink->input() );
    
    // Will be done by pipeline
    src->activate();
    dsp->activate();
    sink->activate();
    
    src->play( dspClock->makeObserver() );
    dsp->play( dspClock );
    sink->play( sinkClock );
    //
    
    while( true ) {
        
        char command;
        std::cin >> command;
        
        if( command == 'q' ) {
            break;
        }
        else {
            // Advance clock by 100 units.
            cp->publish( 100.0f );
        }
    }
    
    // Will be done by pipeline
    src->stop();
    dsp->stop();
    sink->stop();
    //
    
    // Unlink.
    Stage::unlink( src->output(), dsp->input() );
    Stage::unlink( dsp->output(), sink->input() );    
    */
    
    
    
    

    RawBuffer *rb = new RawBuffer(240, 2, kInt16, true);
    rb->mBuffers[0].mBuffer = kSineWave;
    rb->mBuffers[0].mChannel = kFrontRight;
    
    rb->mBuffers[0].mBuffer = kSineWave;
    rb->mBuffers[0].mChannel = kFrontLeft;
    

    BufferFormat format( kStereo21, 512 );
    BufferLength length( (unsigned int)512 );
    
    Int16Buffer *b = new Int16Buffer(format, length);
    
    *b << *rb;
    *b << *rb;
        
    
    while(true){
        char c;
        std::cin >> c;
    }

    
}




