# Program Structure (2017-11-20)
I like flat structures in programs. I don't want layer upon layer of abstraction. I will however create a layer encapsulating all features that belong to the 'platform'; in this initial case: SDL2. As I am writing a software renderer (not dependent on a particular graphics platform) this will allow easy porting to other platforms in the future.

This platform layer will provide the following:

+ A pixel buffer, displayed on screen every frame.
+ An audio callback.
+ An event callback.
+ Some utility functions.

I will tailor the details of these features to best support my game. I also may change my mind about some of these features as the game develops.

Above this layer, I will have a simple, relatively (but not militantly) modular code base. The birds eye view is as follows:

+ Graphics
    - Primitive rendering (point, line)
    - Bitmap rendering (possibly with rotation and scale)
    - Simple animation (bitmap frames)
+ Audio
    - Mixer
    - Synthesiser
    - Sequencer
+ Input
    - Buttom mapping
    - Timing
+ Scene
    - Loads assets
    - Swaps out frame and input callbacks
+ Memory
    - Pool-based allocators
+ Assets
    - Load image and audio files
    - Packed asset format
+ Common
    - Primitive types
    - Maths
    - Anything else

*Note (2018-02-10): I did not end up going with this idea in the end, but it was a interesting exercise.*
