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
This document describes the design and development of a two-player mini-game based video game. The project focusses on the technical implementation, and attempts to demonstrate the idea that building a game from scratch is entirely achievable, especially when focussing efforts on the technical systems that best support the game.

## Introduction
<!-- Discuss the game idea as it relates to the games that inspired it. -->
This document discusses the process of developing a video game based on the concept of rhythm and the interactions of two players as they work together and compete in a series of mini-games. Each mini-game attempts to challenge the players in different ways.

The game is inspired by the video games 'Rhythm Heaven (リズム天国)' (2006) and 'WarioWare' (2003), both of which were released on the Nintendo Game Boy Advance. Both of these games feature mini-games and have a humorous theme, represented in their graphics and mechanics.

<!-- State that the project has a focus on DIY for educational purposes, and argue that it also creates better software. -->
I created this project in the C programming language, implementing many of the technical systems that support the game. This includes graphics, audio, memory management, and input. This was done to further my own understanding of these aspects of game programming, and with the goal of creating a lightweight and efficient program (when compared to using pre-existing libraries and game engines).

<!-- Discuss the structure of the this report. -->
This report will cover in detail the conceptual development of the project, as well as the planning, implementation, testing, and evaluation of the software.

## Background
<!-- Discuss the high-level design of the game as it relates to other games, emphasising what is novel about it. -->
In order to differentiate my project from games that have come before it, I sought to find an aspect of the game-play that I could innovate on. I decided to explore the concept of multi-player, and the experiences that two players have when they need to interact together directly.

### Planning the Implementation
Software written today generally sits atop a large stack of software written by others. Taking any 'modern' website as an example; many web-based libraries are used (each of which must be downloaded to the user's machine, or ready in the browser's cache). All of these libraries are written in JavaScript or perhaps some higher abstraction level, compiled to JavaScript. This JavaScript code is run by an interpreter, utilising a virtual machine and possibly a JIT compiler. This code is sand-boxed, to improve security (although exploits seem to be discovered often). All of this sits inside the browser, which also has a rendering engine for the HTML and CSS code, and likely relies (as most modern browser such as FireFox and Chrome do) on a rendering library that provides routines to render objects using the system's graphics API, implemented by the graphics driver, in a window that is created using the Operating System's API, in a process that is launched by the language's run-time, which calls OS code, which calls into the kernel.

At each level, the people involved have their own ideas and concerns, and build their piece of software around this. If they are designing libraries, they are likely to make decisions that support the most general case at all times. I argue that any software one writes has specific problems, and that generic solutions to specific problems are bad solutions.

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

This works as sound in f32 format expresses all waveforms in the range -1.0 to +1.0; multiplying by the highest value that can be stored in a signed 16-bit integer (32,767) will produce an array of samples in the range (-32,767 to +32,767), correct for the audio format desired. One must be certain that their floating-point samples do not exceed the range -1.0 to +1.0 or the resulting integer values will wrap. Thus it may be sensible to clamp the value, which must be done *before* casting, as the type of the result of the expression is a floating-point number, and can hold any values that may exceed the desired range without wrapping.

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
Text is rendered using a bitmap font; as opposed to generating font geometry on-the-fly. Fonts in this project are defined to be a bitmap image holding all of the drawable characters defined in the ASCII standard, packed horizontally, and every character is the same width (monospace). The location of a character in the image can be calculated using its ASCII code as an offset from the first character:

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

### Scenes
<!-- Discuss the management of scenes and mini-games. -->
Scenes are sets of function pointers that can be swapped out at will. These function pointers are called at specific times, including when the user presses a button, or when a frame of graphics needs to be rendered.

Each mini-game is a single scene, and each menu is also a separate scene.

## Testing and Evaluation
<!-- Discuss the knowledge gathered about how games are tested. -->
In order to effectively iterate on the project, testing with users was a must. I gathered some information regarding the current knowledge around game testing including techniques and best practices here.

<!-- Discuss the methods chosen, and specifics about how testing was carried out. -->

<!-- Show the information collected during testing. -->

<!-- Show how testing information was used to improve the project. -->

<!-- Discuss the cycles of iteration. -->

## Conclusion
<!-- Evaluate the methods used to construct the game. -->

<!-- Evaluate the software performance and responsiveness with timings and user feedback. -->

<!-- Discuss the topics learned, evaluating their importance and difficulty. -->

<!-- Evaluate the final game produced, stating my opinions and how the outcome relates to the initial goals. -->

## Bibliography
+ Handmade Hero (https://handmadehero.org/)
+ ...

## Appendix A
<!-- Program source code. -->

## Appendix B
<!-- Custom tools and build management source code. -->

## Appendix C
<!-- Copy of the original proposal. -->

<!-- Copy of weekly logs. -->
