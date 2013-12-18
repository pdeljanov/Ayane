# Ayane
**Ayane**: when literally translated, it is the Japanese word for "colourful sound."

Ayane is a modern C++11 audio engine offering a clean, fast, and safe API for manipulating audio data and building graphs of signal processing components.

## Architecture
Ayane is largely based on Source-Sink architecture.  A "Stage" is the main building block in this architecture.  A stage can have a number sink ports (inputs), and source ports (ouputs).  In typical operation, a stage will pull an audio buffer from each of its sinks, process the buffers in some way, and then push resultant buffers to its source ports.  A stage may also simply forward a buffer pulled on a sink port to a source port.

Stages are connected to each other by linking a source-sink pair.  Stages may be linked in any way so long as they form an acyclic graph (that is, a graph that has no cycles).  In any audio graph, one node must have a clock provider. Generally speaking, the clock is provided by the slowest pure-sink node.  Typically, though certainly not limited to, the pure-sink node will be an operating system audio, or file output.  When playback begins, each stage is provided a reference to the clock provider which is then used to the clock each stage in the graph.

### Simple Application

```
Link Type:      {      Async.     }            {     Sync.    }

   +----------+ 
   | Decoder0 | Src0 ----+
   +----------+          |          +--------+
      ClkIn              +---> Snk0 |        |                 +-----------+
+-------^                           | Mixer0 | Src0 ----> Snk0 | OS Output |
|                        +---> Snk1 |        |                 +-----------+
|    +----------+        |          +--------+                    ClkPrvd0
|    | Decoder1 | Src0 --+            ClkIn                          |
|    +----------+                       ^----------------------------|
|       ClkIn                                                        |
|         ^----------------------------------------------------------|
+--------------------------------------------------------------------|
```

## Notable Features
### Unified buffer interface

Ayane's unified buffer interface is at the heart of all audio data manipulation. The unified buffer interface provides one common API for manipulating audio buffers of heterogenous sample format, channel layout, and storage format. 

The unified buffer interface provides the following features:

* Automatic sample format conversion
* Automatic channel mapping
* IOStream-like interface
* Raw interface
* Transparent conversion between planar and interleaved storage formats
* Type-safety

Unlike other audio APIs, Ayane does not encourage access to the underlying byte buffer storing the audio data.  In fact, there is no ability to access the underlying buffer directly.  The rationale for this is that accessing a byte buffer is type-unsafe and requires all functions manipulating the buffer to know the underlying buffer storage format, sample format, and channel layout.  Manipulation of the audio frames in the buffer can only be accomplished by using the read, write, and transform functions, or the stream interface.  These two interfaces work on either frames or buffers.  A notable benefit of this scheme is that these frames or buffers can be of hetergenous sample format or channel layout and Ayane will transparently perform the necessary conversions.

However, since most other audio libraries tend to work directly on byte buffers, a facility is required to enable interoperation between Ayane and other libraries. This facility is known as the Raw interface.  Simply put, the Raw interface allows a developer to wrap a byte buffer with a description of the buffer's contents (sample format, channels, storage format, and length). The wrapped buffer can then be read and written to by the unified buffer interface. Using this method, a developer can "import and export" audio data from Ayane.

### Automatic buffer management
Automatic buffer management is a feature that allows Stage developers to not have to worry about allocating or freeing buffers.  The audio graph interface enforces that all buffers that are pushed or pulled are managed buffers.  Managed buffers are buffers that are allocated by a buffer pool, which is typically owned by a stage.  When a managed buffer falls out of scope, it is returned to the stage that owns the buffer pool.  In this way, buffers can be reused throughout the system.  If the stage no longer exists, or the stage's output format changes, the buffer will simply be deallocated.

### Threaded execution model
Since stages may be linked together in an arbitrary manner (provided the final running form is a directed acyclic graph), certain stages become "convergence points" in the pipeline.  The inputs at these points of convergence can safely run in parallel.  When the audio graph is started, each stage will probe its immediate topography to determine if it would be advantageous to run in parallel.

Generally speaking, when looking at a link between a stage pair, if the sink has only one input, and the source only has one ouput, the downstream stage will run in the same thread as the upstream stage.  In other words, when looking at the audio graph, one-to-one relationships run on the same thread while many-to-one, one-to-many, and many-to-many relationships run in parallel.  One-to-one stages can be forced to run asynchronously, but it is not recommended.

Regardless, stage developers do not need to worry about of the stage is threaded as the stage interface is thread-safe. Stage developers should take thread-safety into account if they wish to expose a public interface on their components.

### Automatic buffering
When stages run in parallel, stage execution is at the mercy of the operating system's scheduler. To accomodate for the inability to ensure stage threads run at the correct time, parallel connections are double-buffered automatically. Double-buffering prevents most audio dropouts and keeps the pipeline latency to a minimal level.

### Live audio-graph manipulation
Ayane allows stages to be manipulated during playback.  All public stage interface members can be called during playback.  Stages may even be linked or unlinked from the audio graph during playback with no disruption.  Though *highly* unrecommended, a linked stage may even be deleted outright.

Live audio-graph manipulation is largely possible because of automatic stage link negotiation.    That is, stages are largely lazy when it comes to negotiating buffer formats on its sink ports.  Stages can only fully initialize once a buffer is received on a sink port.  The stage execution model guarantees that before a stage can pull a buffer on a sink port, the stage has been notified of its format and is allowed to configure itself accordingly, or reject the buffer.

### Safety
Ayane was designed to be type-safe, memory-safe, and thread-safe. 

## Progress
Ayane is a pre-alpha software! Despite the majority of the API being complete, it is still in flux, and is not recommended for production use.

## Examples

### Simple Audio Buffer Manipulations
```
// Create a buffer format representing Stereo 48kHz audio
BufferFormat format(Stereo, 48000);

// Create a buffer length of 500ms.
BufferLength length(Duration(0.5));

// Create a Float32 buffer manually.
std::unique_ptr<Float32Buffer> f32Buffer(new Float32Buffer(format, length));

// Use the buffer factory to dynamically create a buffer.
std::unique_ptr<Buffer> int16Buffer(BufferFactory::make(kInt16, format, length));

// Create a Surround Sound 5.1 audio frame in Float32 format.
Surround51<SampleFloat32> f51;

f51.FL = 0.5f;	// Front Left
f51.FR = 0.5f;	// Front Right
f51.FC = 0.25f	// Front Centre
// ...

// Insert the frame f into the second audio buffer. 
// Ayane will automatically convert the Float32 samples into Int16 samples, 
// and then insert the relevant channels (FL & FR) into the buffer.
(*int16Buffer) << f51;

// Copy the contents of the second buffer into the first buffer.
// Ayane will automatically convert the Int16 samples of the second buffer into
// Float32 samples.
(*f32Buffer) << (*int16Buffer);

// Read a Stereo Float32 frame from the first buffer.
Stereo<SampleFloat32> f20;
(*f32Buffer) >> f20;

```

### Simple Stage Usage
```
// Hypothetical decoder stage. One source port: output.
std::unique_ptr<Decoder> decoder;

// Hypothetical output stage. One sink port: input.
std::unique_ptr<OSOutput> speakers;

// Activate the stages.
decoder->activate();
speakers->activate();

// Link the stages.
Stage::link(decoder->output(), speakers->input());

// Select a clock provider.
ClockProvider &provider = speakers->clockProvider();

// Begin playback.
decoder->play(clockProvider);
speakers->play(clockProvider);

// Do something else for a while... 

// Now let's just say, you very randomly want to change what is playing, 
// so you create a new decoder.
Decoder decoder2;
decoder2->activate();
decoder2->play(clockProvider);

// You can quickly swap the old decoder for the new one without any interruption
// at all!
Stage::replace(decoder->output(), decoder2->output(), speakers->input());

// Get rid of the first decoder. If playing, deactivate will first stop the stage.
decoder->deactivate();

// Do something till you get bored...

// Stop playback.
speakers->stop();
decoder2->stop();

// Deactivate.
speakers->deactivate();
decoder2->deactivate();

// Optional: Unlink the stages. Stages will be automatically unlinked when 
// they are deallocated.
Stage::unlink(decoder2->output(), speakers->input());

```