<!-- DRAFT -->

# Rhythm Game
> Benedict Henshaw
> Goldsmiths, University of London
> 2018

## Contents
+ Abstract
+ Introduction
+ Background
+ Specification
+ Design and Implementation
+ Testing and Evaluation
+ Conclusion
+ Appendix A (Main Source Code)
+ Appendix B (Additional Source Code)
+ Appendix C (Definitions and Additional Information)

## Abstract
<!-- Rhythm game with supporting systems built from scratch... -->
This document describes the design and development of a two-player mini-game based video game. The project focusses on the technical implementation, and attempts to demonstrate how many common components of a video game are developed (such as renderers and memory allocators), and how they can be tailored to fit the requirements of the project.

## Introduction
<!-- Discuss the game idea as it relates to the games that inspired it. -->
This document discusses the process of developing a video game based on the concept of rhythm and the interactions of two players as they work together and compete in a series of mini-games. Each mini-game attempts to challenge the players in different ways. The game is inspired by the video games 'Rhythm Heaven (リズム天国)' (2006) and 'WarioWare' (2003), both of which were released on the Nintendo Game Boy Advance. Both of these games feature mini-games and have a humorous theme, represented in their graphics and mechanics.

<!-- State that the project has a focus on DIY for educational purposes, and argue that it also creates better software. -->
I created this project in the C programming language, implementing many of the technical systems that support the game. This includes graphics, audio, memory management, and input. This was done to further my own understanding of these aspects of game programming, and with the goal of creating a lightweight and efficient program (when compared to using pre-existing libraries and game engines).

<!-- Discuss the structure of the this report. -->
This report will cover in detail the conceptual development of the project, as well as the planning, implementation, testing, and evaluation of the software.

## Background
<!-- Discuss the high-level design of the game as it relates to other games, emphasising what is novel about it. -->
In order to differentiate my project from games that have come before it, I sought to find an aspect of the game-play to innovate on. I decided to explore the concept of multi-player, and the experiences that two players have when they need to interact together directly. While my initial inspiration came from Rhythm Heaven, in which each mini-game's solution is a pattern that can be memorised perfectly, I wanted to explore more free-form interaction. Wrought rhythm tracks also do not allow the game to react to the player's actions beyond giving them a score. Also, if the game can react to player action, in a two-player context, one player's actions can affect the other.

### Planning the Implementation
Since the initial idea was conceived, I wanted to implement as much of the project as possible, not relying on libraries or other tools. I wanted to do this for educational purposes, but also in an attempt to build a high-quality piece of software.

<!-- Discuss the technical sub-systems of the project, with examples and critiques of how they are implemented in other games. (Several paragraphs.) -->

#### Graphics

## Specification
<!-- Discuss the range of interactions that the mini-games explore. -->
In the titles that inspired this project, mini-games allow highly varied styles of game-play and aesthetics to be employed in a way that does not confuse the player; their expectations are to see something new and unexpected each time they start a new mini-game. To me, this is an enticing aspect of the design of these games as it opens the door to much creative freedom, in all aspects of design, broadening the range of experiences that a player can have.

I wanted to explore competitive and cooperative game-play.

<!-- Discuss desires for what the player will experience when playing. -->

<!-- Discuss technical goals, and how they support design goals. -->

## Design and Implementation
<!-- Discuss the overall design of the game describing the mini-games and their characteristics. (Several paragraphs.) -->


As the game features several mini-games, each will be discusses separately below.

#### Heart Beat

### Technical Overview
<!-- Discuss the high-level design and structure of the software. -->
The project has five major sub-systems, and a core that ties them together. These sub-systems are Assets, Audio, Graphics, Memory, and Scenes. Each sub-system resides in a single C source file.

There are two major kinds of assets in this project: bitmap graphics, and PCM audio. Graphics are stored using a file format called Portable Arbitrary Map, which contains a brief ASCII text header followed by a block of uncompressed pixel data. Audio is stored in a custom file format designed to match the PAM format used for graphics, following the same principal of plain-text header and uncompressed raw data.

Audio is outputted to the sound hardware via a callback supplied by the platform (initially, the SDL2 library). This callback is run on a separate high-priority thread, meaning it is given more CPU time by the OS scheduler. In this callback, a custom audio mixer creates a single stream of floating-point PCM data and copies it into the output buffer. The mixer holds several separate sounds along with some parameters for how it should be played.

Graphics, much like sound, is handled by a custom renderer that produces a single final bitmap to be displayed on screen. While there is support for some graphical primitive rendering, the core functionality of the renderer is drawing bitmap graphics to the screen. These bitmap graphics can be any size, supporting transparency. They can also also be animated (composed of several bitmaps), or text, generated from a set of character bitmaps.

Memory is handled simply and efficiently by a custom memory allocator. The allocator holds a large pool of memory and sub-allocates from the pool on request. There are several sub-pools each allocating memory with a certain lifetime allowing freeing of memory to be automatic and efficient.

Scenes are used to encapsulate different pieces of the game. Each scene contains some state and a set of functions that handle input and output. This is where game-play code is written. Transitions between scenes can be made at will from anywhere in the program.

### Assets
<!-- Discuss the management of assets and custom file formats. -->
#### Bitmap Graphics
Here is an example header for a Portable Arbitrary Map, for a file with a width and height of 256, in RGBA format, one byte per channel:

```
P7
WIDTH 256
HEIGHT 256
DEPTH 4
MAXVAL 255
TUPLTYPE RGB_ALPHA
ENDHDR
```

For my project I am able to curate all of the files that will be read by my code, so I chose to only support the exact format that I would be using. With this in mind, I could parse the entire header with this single call to `fscanf`, given that all parameters except width and height are constant:

```
fscanf(file,
       "P7\n"
       "WIDTH %d\n"
       "HEIGHT %d\n"
       "DEPTH 4\n"
       "MAXVAL 255\n"
       "TUPLTYPE RGB_ALPHA\n"
       "ENDHDR\n",
       &width, &height);
```

Once the header is parsed and the width and height are known, the pixel data can be read into a buffer as so:

```
int pixels_read = fread(pixels, sizeof(u32), width * height, file);
```

Unfortunately, the pixel format used by .pam files is big-endian, and I am targeting little-endian machines. The byte order of each pixel must be swapped:

```
for (int pixel_index = 0; pixel_index < pixel_count; ++pixel_index)
{
    u32 p = pixels[pixel_index];
    pixels[pixel_index] = rgba(get_alpha(p), get_blue(p), get_green(p), get_red(p));
}
```

See the Graphics sub-section of this major section for the definition of `rgba` and `get_red`, etcetera. Finally the image's pixel data and dimensions are returned in an `Image` structure:

```
typedef struct
{
    u32 * pixels;
    int width;
    int height;
}
Image;
```

#### PCM Audio
The format used for audio samples in this project is single-precision IEEE floating-point, making each sample 32 bits long. All audio data has a sample rate of 48KHz. Here is an example file header containing one second (48000 samples) of audio:

```
SND
SAMPLE_COUNT 48000
ENDHDR
```

Much the same as above, the header is parsed in a single call to `fscanf`, the raw data read with a call to `fread`, and the byte order must be swapped before returning in a `Sound` structure:

```
typedef struct
{
    f32 * samples;
    int sample_count;
}
Sound;
```

### Audio
<!-- Discuss the playback of audio and the mixer. -->
All sound is in 32-bit floating-point format at a 48KHz sample rate. This uniformity of format allows all audio data to be handled in the same way, without conversions during transformation. Not all platforms support this format for output, so this format can be converted to the relevant format as a final stage before playback. Here is a basic example of how to convert to signed 16-bit integer format:

```
for (int sample_index = 0; sample_index < sample_count; ++sample_index)
{
    s16_samples[sample_index] = (u16)(f32_samples[sample_index] * 32767)
}
```

This works as sound in `f32` format expresses all waveforms in the range -1.0 to +1.0; multiplying by the highest value that can be stored in a signed 16-bit integer (32,767) will produce an array of samples in the range (-32,767 to +32,767), correct for the audio format desired. One must be certain that their floating-point samples do not exceed the range -1.0 to +1.0 or the resulting integer values will wrap. Thus it may be sensible to clamp the value, which must be done *before* casting, as the type of the result of the expression is a floating-point number, and can hold any values that may exceed the desired range without wrapping.

#### Mixing
Audio is output by a high-frequency callback. This callback requests a number of samples, which is produced at will by the custom audio mixer. This audio mixer has a list of all playing sounds and their current state, and uses this to mix together a single stream of audio for playback. Each individual sound is simply a chunk of audio samples:

```
typedef struct
{
    f32 * samples;
    int sample_count;
}
Sound;
```

Mixer channels are used to manage the playback of sounds:

```
typedef struct
{
    f32 * samples;    // The audio data itself.
    int sample_count; // Number of samples in the data.
    int sample_index; // Index of the last sample written.
    f32 left_gain;    // How loud to play the sound in the left channel.
    f32 right_gain;   // Same for the right channel.
    bool loop;        // If the sound should repeat.
    bool playing;     // If the sound is playing right now or not.
}
Mixer_Channel;
```

When one wants to play a sound, they must stop the audio callback and insert their sound into a channel. There is a fixed number of sound channels, so the first free channel is found and used. Some parameters must also be set, such as how loud the sound should be, or whether it should loop.

```
// Immediately start playing a sound.
// Returns the index of the channel that holds the sound,
// or -1 if no channel was available.
int play_sound(Mixer * mixer, Sound sound,
    f32 left_gain, f32 right_gain, bool loop)
{
    for (int i = 0; i < mixer->channel_count; ++i)
    {
        if (mixer->channels[i].samples == NULL)
        {
            SDL_LockAudioDevice(audio_device);
            mixer->channels[i].samples      = sound.samples;
            mixer->channels[i].sample_count = sound.sample_count;
            mixer->channels[i].sample_index = 0;
            mixer->channels[i].left_gain    = left_gain;
            mixer->channels[i].right_gain   = right_gain;
            mixer->channels[i].loop         = loop;
            mixer->channels[i].playing      = true;
            SDL_UnlockAudioDevice(audio_device);
            return i;
        }
    }
    return -1;
}
```

Sound can also be queued up, so that it is ready to be played by setting a boolean on the relevant channel. This is helpful for critical sounds as it reserves a channel, as it is possible to run out of channels and have an attempt to play a sound fail (although the number of channels is set to be higher than the common case). One can also set the `sample_index` of a channel to a negative number. The mixer will increment this index without producing any sound, and thus allowing a sound to be queued up with sample-accurate timing.

<!-- Discuss audio synthesis and effects. -->

### Graphics
<!-- Discuss general graphics info. -->
All graphics in the project use the 32-bit RGBA pixel format. This means that there are four channels, red, green, blue, and alpha (transparency), each of which is one byte large (holding values in the range 0 to 255), and stored such that the red byte is on the high end of the 32-bit value, and the alpha byte is on the low end. To write a pixel in C-style hexadecimal with the red, green, blue, and alpha values of 0x11, 0x22, 0x33, and 0x44 respectively:

```
u32 pixel = 0x11223344
```

These utility functions are also used to manipulate pixel data:

```
// Pack an RGBA pixel from its components.
u32 rgba(u8 r, u8 g, u8 b, u8 a)
{
    return (r << 24u) | (g << 16u) | (b << 8u) | a;
}
```

```
// Access individual components of an RGBA pixel.
u32 get_red(u32 colour)   { return (colour & 0xff000000) >> 24; }
u32 get_blue(u32 colour)  { return (colour & 0x00ff0000) >> 16; }
u32 get_green(u32 colour) { return (colour & 0x0000ff00) >>  8; }
u32 get_alpha(u32 colour) { return (colour & 0x000000ff) >>  0; }
```

#### Displaying Graphics
The software renderer holds an internal pixel buffer of a fixed resolution. This pixel buffer is where the results of the renderer are written. To display a frame on screen, the internal pixel buffer is sent to the GPU via a streaming texture for display. One can also directly access the window's buffer and copy the pixel data into it, but there is no guarantee of the format of these pixels (although one can detect and convert appropriately), no way to render with vertical sync to avoid screen tearing, and visual artefacts can occur when the window interacts with other programs, such as Valve's Steam Overlay. Thus, the GPU is utilised for the final stage of rendering, and performs up-scaling to the monitor resolution.

#### Bitmaps
As all pixels are stored in the same format, the bulk of the work required to display an image on screen is done by directly copying data from the image's pixel buffer to the software renderer's buffer. As all pixel buffers are stored as a single array of packed RGBA values, a simple formula is used to access two-dimensional data from the one-dimensional array:

```
int index = x + y * width;
u32 p = pixels[index];
```

Some checks can also be done to ensure that the pixel array will not be accessed out of bounds:

```
// Returns false if the given coordinates are off screen.
bool set_pixel(int x, int y, u32 colour)
{
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
    {
        pixels[x + y * WIDTH] = colour;
        return true;
    }
    return false;
}
```

But there is good reason to directly access the buffer without this check every time; if some operations need to occur in bulk, such as drawing an image into the buffer, checks should be made outside of the inner loop to improve performance.

<!-- Discuss the actual copying of pixel data between buffers. -->
To render a bitmap, one must copy pixel-by-pixel from an image buffer to the renderer buffer:

```
int max_x = min(x + image.width, WIDTH);
int max_y = min(y + image.height, HEIGHT);
for (int sy = y, iy = 0; sy < max_y; ++sy, ++iy)
{
    for (int sx = x, ix = 0; sx < max_x; ++sx, ++ix)
    {
        u32 p = image.pixels[ix + iy * image.width];
        screen_buffer[sx + sy * WIDTH] = p;
    }
}
```

The key aspect of this method is that one must keep track of both the current pixel on screen to be written to, and the current pixel in the image to be read from. Some checks are done to ensure that neither buffer will be improperly accessed (which is not shown here).

#### Animation
<!-- Discuss the rendering of animations. -->
Animations have a time in milliseconds that states how long each frame of the animation will be displayed on screen. Using this time, and a time-stamp from when the animation started playing, the current frame can be deduced and displayed much the same way as a still bitmap.

#### Text
<!-- Discuss the rendering of text. -->
Text is rendered using a bitmap font; as opposed to generating font geometry on-the-fly. Fonts in this project are defined to be a bitmap image holding all of the drawable characters defined in the ASCII standard, packed horizontally, and every character is the same width (mono-space). The location of a character in the image can be calculated using its ASCII code as an offset from the first character:

```
int x = font.char_width * (string[c] - ' ');
```

The 'space' character is the first character defined in the font bitmap, and so has an offset of zero. Since all characters are represented as numbers, one can simply subtract the number that represents 'space', producing an index that, when multiplied by the width of a character, gives an x pixel coordinate denoting the first pixel of that glyph. Given that all of the glyphs are packed horizontally, every character's bitmap starts at a y position of zero, giving enough information to render the glyph the same way all other bitmaps are rendered. One only needs to keep track of how far left they have moved after drawing each character to place the next letter correctly.

This system is incredibly simple and straightforward to implement. The downside is that it only supports ASCII characters and therefore only the English language.

### Memory
<!-- Discuss the management of memory. -->
While custom memory allocators are often considered a must-have for large high-performance video games, many projects settle for standard memory allocation utilities; many don't consider the possibility. When building a replacement, one must think of how they can improve upon a general purpose allocator's offerings; in performance, in ease of use, or other factors.

As with any general-purpose or generic approach, there are trade-offs. A general purpose allocator tries to do its best at the problems of wasting as little memory as possible, allocating and deallocating as fast as possible, and avoiding fragmentation. I propose that it can be very easy to beat a general purpose allocator (such as `malloc`) on all of these fronts, simply by constructing a solution that more directly supports the project that is being made.

Here is my assessment of the memory concerns for my project:

##### Assets Live Forever
Graphics are loaded from disk and must be stored for the entire lifetime of them program.

##### There Can Only Be One Active Scene
The active scene is swapped out at will, and if a scene is returned to, it starts from the beginning; no state is preserved.

##### Some Things Only Last One Frame
Some objects are created for rendering purposes, such as dynamic strings, and will simply be discarded after use. One could use stack memory for some of these things, but it would be more reliable to have some way to ensure their allocation.

#### The solution
The project employs a pool-based approach. It is very simple, with the main allocation function consisting of only 12 lines of code. This allocator follows a principal often employed in high-performance video games: allocate up front, and sub-allocate after that point. There are three pools: the persistent pool, the scene pool, and the frame pool. The persistent pool performs allocations that are never deallocated. The scene pool is emptied when the scene changes, so the next scene has the entire empty pool. The frame pool is emptied every frame after rendering occurs.

This is both easier to use (as `free` never needs to be called) and faster than `malloc` as the implementation is vastly simpler. The trade off is more memory is allocated than absolutely necessary, but in the final iteration of the memory allocator only 32 megabytes are allocated up front, and that limit has never been reached in my testing.

##### The Only Allocation
There are many options once can choose when allocating memory. There are language-level features, such as the `malloc` family of functions, or `new` in other languages. There are OS API calls, such as `VirtualAlloc` on Windows, or `mmap` on UNIX and other POSIX-compliant systems. Allocations can also be made using stack memory, or static memory. There are trade-offs for each, so I first had to discover what my requirements were to make an educated decision.

As stated earlier in this section, I calculated that a fixed upper limit of 32 megabytes would satisfy the needs of my project. This ruled out stack memory, as the default stack limit in clang and GCC is 8MB (which I found by calling `getrlimit`), and in MSVC it is 1MB. While the limits of stack memory can be specified at link time, this would require different actions to build the program on each platform and tool-chain.

I wanted to avoid the overhead of calling `malloc`, as I did not need its internal management of memory. I also wanted to avoid platform-specific functions if possible, but since this case would likely be a single function call I was happy to make an exception. I initially allocated the memory using `mmap`, and only using a subset of its features so that it would be compatible on Linux and macOS. I then put in a compile-time condition to use `VirtualAlloc` if building on Windows. Since I happened to be using the MinGW tool-kit on Windows, `mmap` was also supported, so both options are available.

After some time, I returned to this question. I had noticed that I had not considered another option: static memory. Considering that I had modest requirements, I looked into what the characteristics of static allocation are. On Windows, static data is limited to 2GB on both 32-bit and 64-bit versions of the OS (Lionel, 2011). On macOS and Linux, I could not so easily find an answer, but in my own testing the programme would crash at values higher than 2GB on macOS, but did not crash on Linux at any value. These limits are far beyond what I require, and the implementation is simpler that the previous; both in that it does not require even a function call, and that it is identical on all platforms. This is the implementation that remains in the final iteration of the project.

A compile-time flag allows the use of the previous `mmap` implementation, as it produced better memory debugging information on my macOS machine.

##### Pre-faulting Pages
On most modern operating systems, memory pages are not actually allocated when requested, but when modified; this access is called a page fault. If the program never touches the memory, it is not allocated. This can be demonstrated by watching the program in a system resource monitor (such as Task Manager, Activity Monitor or `top`) and calling an allocator without modifying the memory. This is counter-acted by the `mmap` argument `MAP_POPULATE`, which pre-faults every allocated page to ensure they are available and there is no overhead when accessing pages for the first time. This argument is unfortunately not cross-platform (it is not available on macOS), so immediately after the initial allocation is made, the game manually accesses the first byte of each page to ensure that all pages are readily available.

##### Thread Safety
One aspect not implemented in the final code-base, but researched, was making the allocator thread-safe. Initially, my implementation used a `mutex` to guarantee that allocations could be made from any thread without interference, but after more of the project was implemented, I found that I never allocated memory from a thread other than the main one, and so this feature was removed as it added cost with no benefit.

### Scenes
<!-- Discuss the management of scenes and mini-games. -->
Scenes are sets of function pointers that can be swapped out at will. These function pointers are called at specific times, including when the user presses a button, or when a frame of graphics needs to be rendered.

Each mini-game is a single scene, and each menu is also a separate scene.

### Building the Binary
For this project, I compile the entire source as a single translation unit. This is sometimes called a 'Unity Build'; achieved by using the `#include` preprocessor directive to combine all source files into a single file, and compiling it. It builds faster than more traditional methods which wherein each file becomes a separate translation unit, and can allow the compiler to produce a better optimised binary. This method also makes header files unnecessary, as nothing needs to be forward declared when the entire code based coexists in a translation unit.

## Testing and Evaluation
<!-- Discuss the knowledge gathered about how games are tested. -->
In order to effectively iterate on the project, testing with users was a must. I gathered some information regarding the current knowledge around game testing including techniques and best practices here.

<!-- Discuss the methods chosen, and specifics about how testing was carried out. -->

<!-- Show the information collected during testing. -->
### Testing the Heart Mini-Game
The heart mini-game is the first interaction with the game that players will have. Therefore, I started to test it early on in development. I wanted to avoid upfront explanation and tutorial, so needed to ensure that players could figure out what was happening, and what was expected of them. Here are some of the points that I received, which helped to inform the final mini-game:

#### The Timing Bar Looks Like A Charge Up Bar Used In Other Games
This mini-game contains a bar that indicates how accurate the players are tapping on the beat, on a meter of slow to fast with the correct timing in the centre. One piece of feedback that I received was that this bar looks like a 'charge-up' bar used in other video games, causing an interpretation that they should press their button when the dial on the bar is in the green area. They interpreted the bar to be giving instruction, instead of simply displaying state.

Mid-session, I turned off the entire interface, and the player quickly realised that they needed to be focussed on the sound and not the bar. I learned that while I constructed the interface to help players learn how to play, it could increase confusion about what the focus was, and what they were expected to do. The final version of this mini-game displays only the heart beating with the metronome sound to highlight that the sound is the focus, and then introduces the interface when the player begins to struggle.

<!-- Show how testing information was used to improve the project. -->

<!-- Discuss the cycles of iteration. -->

## Conclusion
<!-- Evaluate the methods used to construct the game. -->

<!-- Evaluate the software performance and responsiveness with timings and user feedback. -->

<!-- Discuss the topics learned, evaluating their importance and difficulty. -->

<!-- Evaluate the final game produced, stating my opinions and how the outcome relates to the initial goals. -->

## Bibliography
Handmade Hero
https://handmadehero.org/

Lionel, Steve (Intel), 2011
Memory Limits for Applications on Windows
https://software.intel.com/en-us/articles/memory-limits-applications-windows

setrlimit(2)
https://linux.die.net/man/2/setrlimit
`getrlimit(RLIMIT_STACK, &limit);`

getpagesize(2)
https://linux.die.net/man/2/getpagesize

## Appendix A
<!-- Program source code. -->

## Appendix B
<!-- Custom tools and build management source code. -->

## Appendix C
<!-- Copy of the original proposal. -->
### Original Project Proposal
This section contains the project proposal, handed in on the 15th of December, 2017.

#### Final Project
BSc Games Programming
Year 3, 2017 - 2018
Benedict Henshaw

##### Overview
> Competitive local multi-player one-button rhythm game.

My final project is a video game. It is a rhythm game in the vein of 'Rhythm Tengoku' and the 'WarioWare' series, and will be all about keeping time with music.

The focus is on multi-player mini-games which each have their own spin on keeping time.

##### Mini-Games
I would like to implement at least one of these mini-games by the end of my project.

###### Arm wrestling
Each player's button pressing controls one arm. The arm that pushes the other over wins.

###### Possible mechanics
They must press in time with the beat, but they can choose at what rate they press. The force with with each arm is pressing is based on the accuracy and the rate at which they press. Whichever arm has the highest force will push the other until one arm is bent completely over.

###### Horse racing
Each player's pressing controls a horse in a race. The horse that reaches the end fastest wins.

###### Possible mechanics
To run faster the player must press at different time signatures and rhythms. Their speed is based on accuracy and the rhythm at which they press.

The player must transition into the next rhythm to increase speed.

###### Factory
Each player controls a robotic arm that is taking objects off a conveyor belt in a factory. The robot that has successfully collected the most objects or reaches a target amount wins.

###### Possible mechanics
Both players press once to put the object on the conveyor belt. The object can end up further to the left or right of the belt based on who was most accurate in the first press. Then they must press again to pick up the object.

##### Technology
I want to write the program in C, using SDL2 as a platform layer. I will write a custom audio back end and sequencer for playing custom audio tracks and performing some on-the-fly synthesis. The graphics will be handled by a custom 2D software-based renderer. I will handle memory using some custom allocation techniques, but this game does not require much data throughput so these systems will be relatively simple. Some basic multi-threading will be used; possibly employing a job system. I hope to achieve low-latency input and audio output, and 60 FPS video output.

###### Input
When the user presses a button, that event will be queued. During the next frame, the event queue will be run through and each event handled. This may be done more often if the game does not feel responsive enough. These events are timestamped and I will use that time stamp to check if a player has pressed the button in time with the music. I would like to allow external devices like game controllers to be used in the game too.

###### Audio
I will be employing a custom audio mixer for this project. I will allow for mixing of several 'channels' of PCM audio data. This will allow playback of multiple sounds at once, and may be used to handle individual 'instruments' in an audio track. It will also allow me to know with high accuracy how much music has played and this will help match input to audio output.

A sequencer will handle playing music. The sequencer will have 'instruments' that can produce sound at will with some parameters. There will be at least two kinds of instruments: sample-based and synthesis-based. Sample-based instruments will simply write some pre-recorded sound data into the buffer, while synthesis-based instruments will generate sound data on-the-fly. I would also like to employ some audio effects, such as delay and some filters (low-pass, high-pass, etc.).

I may need to write a secondary tool for authoring audio sequences, though it may just be a system that interprets a plain text format.

###### Graphics
A custom 2D software renderer will fill a buffer with pixels each frame. The most important feature will be a simple sprite copy, but will also have rotation and scaling features. It will handle text using a bitmap font system. Some graphical primitives may also be implemented. Alpha keying (if not alpha blending) will allow for transparency in images. A sprite sheet will contain all graphical elements and the renderer will have a table of locations for each sprite.

###### Data
Graphical data will be handled in the form of '.png' or '.bmp' images. I will opt for '.png' if the total file size of the images becomes inconveniently large using the uncompressed '.bmp'. Sprite sheets will allow lots of graphical components to exist inside one file, which will improve performance when loading data from disk.

Audio sample data will be in '.wav' format unless there is so much of it that a compressed format is needed. In that case I will use '.ogg' as there is a high quality public-domain library available for decoding. 'Audio fonts' will allow lots of separate sounds to share a single file, to improve load performance (and decode performance if used).

A basic (probably plain-text) file format will be used if any player settings or save data needs to be stored.

<!-- Copy of weekly logs. -->
### Weekly Logs
This section contains several blog posts made during the development of the project. They are listed in chronological order.

#### Getting Started (2017-09-30)
I want to build rhythm game for my final year project.

##### Design
My favourite in the genre is Rhythm Tengoku (2006, GBA), which is mini-game based with a comedic theme. This game is single-player, with each mini-game requiring the player to demonstrate some ability to stay in time with music, but each having its own take on what the player should be doing. The game does not use many buttons for this as it is not about reflexes and hyper-awareness so much as a raw sense of rhythm and use of logic.

Dance Dance Revolution (PS, 1998) and Guitar Hero (PS2, 2005) are also rhythm games, but their focus (and theme) are very different. Both are score focused, with competitive elements, where the player has a higher score the more beats they hit successfully, and the player with the highest score wins. These games are also more reflex based, with an emphasis on memorising the patterns of button presses mapped to a song. I find this less interesting, and can be alienating to those who may not have the physical capacity to play them, or the time to memorise songs. This is not the kind of game I would like to make, but I do like the competitive nature of them.

I would like to borrow the aspects of these games that I like for my project, and include new features that the genre has not seen yet.

#### Building the Basics (2017-10-15)
I've put together some code that demonstrates the basics that I will need for my game. I have graphics with rectangles and line drawing, and some simple audio output via a callback.

I have been experimenting with getting reasonable input to audio output latency. On macOS I can set my buffer size to 1 sample (which doesn't actually feel different from say, 64 samples), so output latency is not bad. I am almost happy with the latency, but it could be better. I think the first problem to tackle lies with my input handling.

Each frame I check the event queue for input, handling any that have been buffered. The events can come in at any time, but I only act on them once at the start of the frame. Since my graphics code supports v-sync my frame time is a reliable 16ms, I will assume my input latency is at least 16ms. At least, that is the amount of delay I may have control over. If I disable v-sync I do find that I can feel the difference when using keyboard input to control audio out. I like having v-sync (and it drastically reduces CPU/battery usage), so I may have to find a way to check for input more often without being tied to graphics output.

#### Program Structure (2017-11-20)
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

#### Memory Allocation (2018-02-15)
For my project, I have employed a very simple and fast method of memory management. It is centred around the concept of memory pools, which are stack-like structures that have a fixed amount of memory under their control. When an allocation is made from a pool, it selects some memory from the top of its pool and returns the address of that memory.

The allocated memory is also ensured to be aligned correctly for any type. This can be ensured by making the address of the returned pointer is a multiple of the largest alignment needed. There is a C language type called `maxalign_t`, and taking the size of this type will give you the maximum alignment needed. On two Intel machines I tested this gave 16 bytes. Not aligning correctly will only cause access to that memory to be slower, nothing fatal.

I have three memory pools, the persistent pool, scene pool, and frame pool. Each of which defines a lifetime of the memory it allocates. Any memory allocated by the persistent pool will live for the entire lifetime of the program. Scene pool allocated memory will live until the scene changes. The frame pool is reset after every frame. Individual allocations cannot be arbitrarily freed when allocated by a memory pool, all memory is deallocated when the pool is reset. One simply has to decide how long the allocation should last when allocating, and choose the appropriate pool.

This is a convenient way to manage memory as any functions that would like to allocate memory can ask the caller which pool they would like to use, giving more flexibility to the caller. It is also far faster than `malloc` or `new`, and as it does not have the concept of granular deallocation, and so does not force the overhead of `free`/`delete` on the caller.

The trade-off is that more memory may be allocated to the program than needed; although this is counteracted by the fact that operating systems don't actually allocate memory pages to programs until they attempt to write to them. This adds some overhead, so I may want to touch every allocated memory page at start up to ensure that is ready to go before an allocation is made. Overall this solution is better (faster, simpler), mostly because is is tailored to suit the needs of my game, and general purpose allocators are not.

The fixed amount of underlying memory managed by the pools is small enough (in my case) to comfortably allocate completely in static memory. Some research lead me to believe that 2GB is a reasonable maximum of static memory to assume (https://software.intel.com/en-us/articles/memory-limits-applications-windows).

#### Assets, Graphics and Animation (2018-01-10)
Following the theme of do-it-yourself, I have written an image file reader and writer. It operates on Portable Arbitrary Map (`.pam`) files. The format is very simple, with a short plain-text header and uncompressed contents. As this game does not rely on high resolution graphics, it is perfectly reasonable to use uncompressed data for graphical assets.

Now that I can easily import (and export) pixel data, I have fleshed out the rendering system. At its core, my renderer has an internal pixel buffer. This buffer is where everything is rendered. Finally, this entire buffer is displayed on screen. This step can done by sending the pixel data to the OS, or by copying the data to a streaming texture in video memory and displaying with the GPU.

##### Animation
Rendering still images is very useful, but I would like to produce animations that are made up of multiple frames. In order to achieve this, I first need to consider how I will deliver this data to the program.

Bitmap rendering in this project is often concerned with raw pixels. The formula `x + y * width` is employed throughout the renderer to access, set, and copy pixels. `width` is actually not the most correct way to describe this formula; `pitch` is more correct than `width`. The difference is that while you might want to draw an image of width `20px`, The pixels of that image might have come from a much larger image. `pitch` is the width of entire pixel buffer. This is a very common feature of renderers often called a texture atlas (or sprite sheet): an image that holds many images packed together.

I noticed that I could lean on my previous image rendering code if I made my `pitch` and `width` equal. This can be done by packing in images vertically; each consecutive frame is underneath the previous. I would simply have to pass in a pointer to the top-left-most pixel of the frame that I want. It is a simple equation to calculate this:

```C
// To render frame 2 of an animation:
int animation_frame = 2;
int pixels_per_frame = width * height;
int pixel_offset_to_current_frame = pixels_per_frame * animation_frame;
u32 * final_pointer = pointer_to_start_of_image + pixel_offset_to_current_frame;
```

#### Rendering Text (2018-01-13)
While text in most modern programs is rendered on the fly to allow scaling and differing pixel densities, my program has a fixed internal resolution, so the result of this text rendering is known ahead of time. This reason, and the fact that the implementation is far simpler, lead to my decision to render text the same way I handle animations: an atlas of sub-images.

Firstly, I wanted a way to print anything, including the values of variables. I used the standard library function `vsnprintf` to achieve this. The `v` in its name states that it allows the use of a variadic function argument list to provide its arguments; the `s` states that it will write its result to a string; the `n` means that a hard character limit must be specified. Aside from these aspects, it performs the same function as `printf`: it converts data into strings.

Once I have my string, I must find some way to map the characters to bitmap images in my atlas. I could make a list of characters and their associated `x, y` positions in the bitmap, but this is more time consuming than I would like. The method I used leans on the fact that all characters are of course represented by numbers, and these number have a reliable ordering. At this point I made the concious decision to only support the English language in my initial implementation, and to rely on the ASCII standard for character representation.

An important characteristic of the ASCII standard is that all 'drawable' characters are grouped sequentially. All characters from `!` (value 33) to the character `~` (value 126), are drawable, in that they are not white space or a 'control' character. Knowing this, I could use the character's value to calculate an index into my font bitmap.

In order to keep the rendering process simple, I opted for mono-space fonts, where I will always know the width and height of every character, as they are all equal. Using a font bitmap that packs each glyph horizontally, starting at the space ' ' character (value 32), here is how I calculate the index:

I index the string to get the character I want, then subtract ' ' (32) from it to map it to my index which starts from zero.

```C
int top_left_x = character_width * (text[c] - ' ')
```

This value, and the fact that each glyph is packed horizontally, means that I know where in my texture atlas my glyph is (`top_left_x, 0`), and can render it.

As I mentioned in my previous post, I will also need the `pitch` of my texture atlas to index into it. I could save this separately, but it is derivable from the width of a character. 96 is the difference between '~' (126) and ' ' (32) (the total number of characters in the font bitmap), but it starts from zero so -1.
```C
int pitch_of_font_bitmap = 95 * character_width;
```

This combined with keeping a sum of how far towards the left I move after rendering each character gives me a simple and fast way to render text. I can easily use this render text into a buffer if it is being used frequently.

#### The Audio Mixer (2018-01-21)
In the same way that I send a single pixel buffer to the screen, I can only send one audio signal to the sound card. Therefore, in much the same way as graphics, I need a way to combine all of my playing sounds into a single stream of audio. This is commonly called a mixer.

At its most basic, a mixer will add each sample in each stream together:

```
...,  0.3, -0.5,  0.1,  0.2, ...
...,   +     +     +     +   ...
...,  0.1,  0.2, -0.1,  0.3, ...
...,   =     =     =     =   ...
...,  0.4, -0.3,  0.0,  0.5  ...
```

The resulting stream of numbers will be the mixed sound, and will appear to contain both sounds.

My implementation of audio mixing is based on `channel`s. Each `channel` can hold one sound (an array of numbers), and a set of parameters for playing the sound, such as how loud it should sound in each ear. These channels also keep track of how much of the sound has been played, and what should be played next. I have a fixed (but quite large) number of channels, as this is both simple and efficient.

Since the frequencies of audio data are much higher than video (48000hz vs 60hz), the data often needs its own thread to ensure that it is being handled without delay. While I investigated handling the audio on the main thread (as it would provide better synchronisation with input events), it was difficult to avoid occasional hitches in the sound as the frame loop is not currently time-locked strongly enough. Thus I conceded to using another thread in the form of an asynchronous callback. This meant that the data shared between threads must be handled carefully, and ideally not modified too often. I used a simple built-in locking mechanism to achieve synchronisation, and set my buffer size to a relatively small value (~64 samples) to keep latency low. The performance of this implementation is adequate, but I will keep a close eye on it to see if it must be improved later on.
