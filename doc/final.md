\pagenumbering{gobble}

\maketitle

**Abstract:**
This document describes the design and development of a game engine -- a set of supporting technical components -- for a two-player, one button, mini-game-based video game, and an example game. The project focusses on the technical implementation, and attempts to demonstrate ways in which common components of video games can be developed (such as renderers and memory allocators) without using libraries, and how they can be tailored to fit the requirements of a specific project.

\newpage

\pagenumbering{roman}

\tableofcontents

\newpage

\pagenumbering{arabic}

# Introduction
When setting out to develop a video game -- as with any software -- there are many choices one can make: platforms, languages, libraries, and more. It is very common in modern game development to use a 'game engine', such as Unity\[7] or Unreal\[8], or any number of libraries\[9]\[10]. As these tools are available and accessible, many developers rely on them and do not build their games from scratch. I argue that this causes a subtle stagnation of the technologies that are used in game development, and of the design of the games themselves. Each library or engine will have its own strengths and weaknesses, and this imparts some friction on the development process. An engine developer can only account for so much, so a designer using an engine may find it difficult to design something new that pushes the boundaries of the medium.

In this document, I describe the process of developing the technical components of a video game; the parts that one could call an 'engine'. These technical components are *not* intended to be generic and applicable to the design of any game. They are designed to support a specific game: one that roughly models the video games 'Rhythm Tengoku' (2006) and 'WarioWare' (2003). I have chosen to do this as an attempt to avoid the constraints of any currently available game engines, and to learn more about (and document) how to write the technical systems that support a video game.

Both Rhythm Tengoku and WarioWare feature 2D graphics, game-play that is divided up into mini-games, use a small number of inputs, have a humorous theme, and were both released on the Nintendo Game Boy Advance. They were in-fact developed by much of the same team. I chose these games as inspiration both because I enjoy them, and because they do not have complex designs. These are 'rhythm' games, which are mostly concerned with the players ability to tap a button to music. They do not have complex simulation or player decision making, narrative or vast amounts of content. Drawing inspiration from these games in particular allows the focus of the project to be on the supporting technical systems and not the game-play of the game.

One might question the decision to build engine-level technology that is not designed to support many kinds of games. Firstly, I did not want to tackle the problem of developing a generic game engine, as I do not advocate their use[^1]. Secondly, I think that generic solutions are not good solutions. I believe one always has a specific problem at hand and should design a solution that best solves that problem. In my opinion, a lot of poor quality software is built by combining a set of generic solutions to perform a specific task. Therefore, I want all of the code in the project to work together to directly solve the problems put forward by the design of the game.

[^1]: See [Appendix A] for more on this topic.

\pagebreak

## Goals
Before I began the project, I had a set of goals. See the conclusion in section 7 for an evaluation of how well the goals were met.

**To, wherever possible, only use code that I have written.** I wanted to learn a breadth of techniques from this project. I did not want to leave major aspects of the software, such as memory allocation or rendering, up to a library to carry out.

**To tailor every aspect of the code to the design of the game.** I wanted to implement every system as a solution to a known problem. While this may seem obvious, one could take the opposite route and write code that performs many functions in an attempt to make it flexible. This flexibility may have significant costs, and must be fully justified. If it is not, it serves only as a poor solution to the problem.

**To produce a good quality piece of software.** I wanted to produce software that doesn't crash, is highly responsive to user input, and makes effective use of system resources (CPU cycles, RAM, storage, etc.). My concrete performance goals are rendering graphics at a minimum of 60 frames per second, playing audio without any hitches (not 'dropping' any samples), and handling user input such that the game responds within 10ms.

**To learn about many areas of game development, and document them.** Learning new techniques was always a key motivator for this project. I would also like to document what I learned in a detailed way, which I have done in this dissertation.

## Structure
This report will cover in detail the conceptual development of the project, as well as the planning, implementation, testing, and evaluation of the software. Section 2 ([Background]) describes some components found in video games and some concerns for their implementation. Section 3 ([Specification]) concretely defines the task that was attempted, outlining the entire project at a high level. Section 4 ([Design and Implementation]) describes the design decisions made, explains aspects of how the software works, and how development was carried out. Section 5 ([Testing]) describes how this software was tested and how the information gathered informed its development. Section 6 ([Evaluation]) evaluates and critiques various aspects of the project. Section 7 ([Conclusion]) provides a brief discussion of the resulting software and its development.

\newpage

# Background
<!-- Compare technologies. -->
Features most commonly found in video games are input, visuals, and sound. Behind these features there are many commonly occurring technical systems, such as graphical rendering systems, audio mixing and effects systems, memory allocation and management systems, timing systems, and more.

## Overview of Game Engines, Frameworks and Libraries
An overview of some game engines and libraries is provided below.

Name             Platforms  Languages        Graphics      Audio
---------------- ---------- ---------------- ------------- --------------------
Unity            Multi      C# (Cg, HLSL)    3D, 2D;       Built-in mixer
                 Consoles                    Shaders       No buffer access
Unreal           Multi      C++, Blueprints, 3D; Shaders   Built-in mixer
                 Consoles   UnrealScript,                  No buffer access
                            (GLSL, HLSL, Cg)
LÖVE             Multi      Lua              2D; Shaders   OpenAL-based
                                                           No buffer access
SDL2             Multi      C, C++           2D            Callback, Queueing
                 Consoles   (Many bindings)  DirectX       Indirect buffer
                                             OpenGL        access
                                             Buffer access
Microsoft XNA    Windows    C# (.NET)        2D, 3D;       Built-in mixer
                 XBox 360   DirectX                        No buffer access
Cocos2D          Multi      Objective-C      2D; Buffer    Implementation-based
                            (Many bindings)                (often OpenAL)
**This Project** Multi      C                2D            Built-in mixer
                                             Buffer access Buffer access

As mentioned earlier in this document, there are many tools available for the creation of video games on many platforms. Some of these tools, such as Unity and Unreal are attempting to provide a vast amount of functionality in an attempt to be general purpose and provide a useful foundation for any video game. Others, such as LÖVE and Cocos2D are attempting to support only games with 2D graphics. Each of these tools target certain platforms, some of which include video game consoles. This project has a narrower scope than all of the given examples, as it is designed around supporting a specific game.

Many of these tools offer 3D graphics, support for many programming languages (some of which include an embedded interpreter or virtual machine), and impose limits on the control of audio output. These are all features that I did not want in this project, and their existence in the final binary of the game make it more complex with no gain. This was the initial inspiration for this project: to produce a piece of software that only includes necessary components.

### Why Does It Matter How Large the Code Base Is?
A talk given recently by Casey Muratori\[15] illustrates this point very clearly. To summarise the points relevant to this project:

**Every line of source code (or instruction in the binary) is a possible target for malicious attack.** Given that mistakes happen and things go unaccounted for, any line of code could provide a way for an attacker to perform malicious actions on a user's computer. Removing that code -- especially if it is not completely necessary ('cruft', as described by Muratori) -- will improve the security of the system as a whole.

**There are millions of lines of code between a user process and the hardware.** Given the size of operating systems, drivers, and firmware in the modern world, this has become true. As expressed in the 2015 paper by S. Peter et al\[16] on Arrakis -- a modified Linux kernel, one can obtain very large performance gains by creating a shorter code path in the operating system that allows the hardware to perform the actions that user-space code wants to achieve with less overhead. The same hardware, and the same code in the program, can achieve an almost 5x performance gain (in the example of HTTP transaction throughput) by lowering the overhead of the OS. One might suggest that a trade-off was made; perhaps eschewing some security code, but I leave the reader with this quote from the paper itself:

> We conclude that it is possible to provide the same kind of QoS enforcement in this case, rate-limiting in Arrakis, as in Linux. Thus, we are able to retain the protection and policing benefits of user-level application execution while providing improved network performance.

To summarise: all the code I am writing is sitting atop enough *cruft* already; if I can choose to have less I will.

## Inspiration

The game that I have taken most inspiration from, Rhythm Tengoku, appears to utilise detailed bitmap graphics, with frame-based and motion based animation. That is to say, it animates things on screen both by displaying a series of discrete images in succession, and by drawing the same image in changing locations on the screen. It also performs rotation and scaling of bitmaps to further animate them.

As Rhythm Tengoku runs on the Nintendo Game Boy Advance, it has limited audio output capabilities. The system has two 8-bit PCM[^2] sound channels, three programmable square wave channels, and one noise channel\[4]. Despite this, the game has sections with vocals and recorded instruments. While the quality is does not compare favourably with modern standards, it performs well enough to add much to the experience of playing the game.

[^2]: Pulse Code Modulation -- A method of representing an analogue signal with digital samples.

The Game Boy Advance has twelve inputs that can be used by games that run on it. Rhythm Tengoku rarely uses more than four of them, and often less than that. When it does, multiple buttons are often mapped to the same function. I was very interested in the limited number of inputs, as it turns the focus of the game partially away from player dexterity. Many successful music games like Guitar Hero (2005) and Dance Dance Revolution (1998) do have a focus on player dexterity, but that was not my area of interest for this project.

Input latency is also a major concern for Rhythm Tengoku and WarioWare. As these games don't utilise many buttons for input, the timing of button presses is given more focus. This is easier to achieve on dedicated hardware such as the Game Boy Advance, but may be more difficult on platforms that aren't designed with this concern in mind, as many modern consumer computers are.

![Screen shots of various mini-games from Rhythm Tengoku.](data/rhythm_tengoku_shots.png)

### Game-play
Each mini-game in Rhythm Tengoku is reminiscent of a music video. It has some (usually minimal) animated graphics, and features a track of music that is the same every time. The player is given a tutorial at the start of the mini-game which explains what they need to do -- essentially what their cue to tap a certain button is. Given that the game is in Japanese (and there has never been an official translation), this tutorial has limited use for me as a non-Japanese speaker. I found it very interesting to have to figure out what you as the player are expected to do, without instruction.

As the game progresses, the complexity of the mini-games increases requiring more attention and timing ability. Each mini-game introduces a different kind of rhythmic interaction; some are about tapping to a certain note of a repeating melody, some are about keeping a constant beat going, some ask that the player press a certain button based on the animation on screen, and some are more abstract.

## Design in One Button Games
Although this project is focussed on the technical foundation, I still felt the need to differentiate my project from games that have come before it. I sought to find an aspect of the game-play to innovate on, so I decided to explore the concept of multi-player and the experiences that two players have when they need to interact together directly. While my initial inspiration came from Rhythm Tengoku, in which each mini-game's solution is a pattern that can be memorised perfectly, I wanted to explore more free-form interaction. Wrought rhythm tracks also do not allow the game to react to the player's actions beyond giving them a score. Also, if the game can react to player action, in a two-player context, one player's actions can affect the other.

Further research in the area of limited input games brings up the burgeoning field of 'one button' games\[11]. These kinds of games are often taught when introducing students to game design as they force designers to consider player interaction closely. Neither Rhythm Tengoku or WarioWare are strictly *one* button games, so I felt that this was another way in which my project could stand on its own.

## Planning the Implementation
From the initial conception of the idea for this project, I wanted to implement as much of the it as possible, not relying on libraries or other tools. I wanted to do this for educational purposes, but also in an attempt to build a high-quality piece of software. Considering the technical aspects of the games that I took as inspiration, I decided on a set of features I aimed to implement in my engine. These included bitmap rendering, audio mixing and playback, low-latency input, memory management and asset management.

\newpage

# Specification
This section contains an outline of the design and technical features of the project, as decided in advance of their implementation.

## Design of the Showcased Game
This project is primarily designed to exemplify the ways in which the technical systems of a video game can be built, and how those systems can be tailored to support a specific game. For that goal to be realised a game must be designed to showcase the usage of these features. The following is a description of some high-level decisions that I made about this example game and their motivations.

In the titles that inspired this project, mini-games allow highly varied styles of game-play and aesthetics to be employed in a way that does not confuse the player; their expectations are to see something new and unexpected each time they start a new mini-game. To me, this is an enticing aspect of the design of these games as it opens the door to much creative freedom, in all aspects of design, and broadens the range of experiences that a player can have. Therefore, I wanted implement a system that facilitates mini-games, including swapping between them.

Considering that I prefer the aspects of rhythm games that emphasise timing and de-emphasise dexterity, I decided that a one button game would align with these ideals. I also enjoy the experience of playing with other players, and the games that I have taken inspiration from do not have multi-player. I decided that constructing a game made for two players would allow it to be novel, despite the game mainly being a showcase for technical implementation.

### Theme
To make the mini-games feel like a more cohesive whole, I decided to find an overarching theme for the game. My initial proposal for this project (which can be found in Appendix B) laid out some ideas for mini-games that could be included, but none of these were implemented as they were either too artistically demanding (I produced all of the artwork for the project myself), or did not fit with the theme.

The final theme for the showcase game is the human body. It is titled "Rhythms of the Body". There are three mini-games: the first is a beating heart, the second is breathing lungs, and the third is the digestive system.

\pagebreak

## Technical Overview
In order to approach the task of building a software stack for a video game, I sought to figure out what kinds of data I would be dealing with. I took this approach, as I find working with data and understanding the structure and patterns of data to be the most affective mindset, as opposed to constructing software in a more conceptual way.

There were several kinds of data that I needed to manage. Data for graphics in the form of bitmaps, data for audio in the form of sample streams, data for user input, and the specifics of any on-disk asset formats.

### Graphical Data
Graphical data for reproduction on a video screen is often handled in pixels. These pixels can have a number of separate components, each of which representing some *primary* colour. These primaries are mixed to produce a final colour. One can also use an index-based method, in which a table of colours is kept, and bitmap images have an index at each location referring to a colour from the table. As I did not know what direction the art style would take, and what concerns the graphical system would have, I decided that I would use a very flexible format: RGBA. This format is very common; it has four channels for the primaries, red, green, and blue, and a channel for alpha which is used for blending colours for transparency. While this storage format is larger than others, it is simple to manipulate, and supports a large number of possible colours.

In order to display the data on screen, pixels need to be sent to the graphical hardware to be output. This process can be done in many ways, but I intended to use a buffer which would be regularly updated and sent off to be displayed. This buffer has its own format, and if this format differs from the format used by my renderer conversions would need to be made. Using the RGBA format helps avoid this, as this is a format natively supported by lots of graphics hardware and software -- this is another major reason why I chose to use this format for graphical assets. I also intended to use a low resolution buffer, as this more closely matches the games I took inspiration from, and also lowers the performance requirements of some aspects of the rendering.

### Audio Data
To play sounds digitally from a speaker, one needs to produce vibrations by setting the speaker's position rapidly. This can be done on-the-fly, meaning the speaker positions are generated by the software itself (synthesis). This can also be done by using a set of pre-recorded positions. While I did want to perform some audio synthesis, I also knew that I would need to handle pre-recorded sounds.

Audio formats (when not compressed) are relatively simple, in that they are usually just a buffer of numbers. But, this still leaves room for incompatibility as there are many ways to store a number on a computer (not to mention endianness[^3]). As with pixel data, I decided that I would support one of the more flexible and high quality formats for storing audio: 32-bit floating point. Within the audio mixer I intended to use a floating point format regardless as it simplifies the mixing process, as mixing using integers requires implementing a method to handle overflow.

[^3]: The order in which bytes are stored within the register of a computer -- whether it starts with the *big end* or the *small end*. The term was originally used in reference to the book Gulliver's Travels, wherein two tribes of people wage war over which end of a boiled egg one should crack open first -- the big end or the small.

While 32-bit float is a relatively large format -- audio such as that on a CD is commonly in 16-bit format -- I was less constrained by storage and memory space than developers on other platforms, and I decided that it would be reasonable to store my audio recordings in this format on disk, loading them into the process without conversion where they can be used directly by the mixer.

Along with the format of each audio sample, there is the concern of the frequency of samples. Frequency has a major affect on the range of pitch that can be expressed by the data. 48KHz is the frequency used by DVD audio, and is widely supported by consumer computers. 44.1KHz is also very common, being the format of CD audio, having slightly lower fidelity but smaller storage space per second of audio. As I did not expect to have large amounts of audio data, and am also not hard-bound by storage space, I chose to use 48KHz audio.

### User Input Data
User input differers greatly from the other forms of data being input into the program. Input is concerned with timing -- and the process's ability to react quickly to it -- as much as it is with content. As I decided to construct a 'one button' game for this project, I was even less concerned with what button the player is pressing, but mainly the exact time at which they pressed.

### File Formats
As I intended to not rely on libraries for as many aspects of the project as possible, I also sought to write custom file readers. These file readers were intended to be simple, but facilitate all the features needed by the game. Ideally, the file formats used would specify enough information in their headers such that the file reader could check that all of the data had been read in successfully. I decided to utilise a common file format for bitmap graphics called Portable Arbitrary Map (an extension of Portable Pixel Map[^4]) as it is simple to generate and parse, and to use a custom audio format inspired by this bitmap format.

[^4]: Both of these formats are part of the wider Netpbm family of file formats.

### Management of Assets
Video games built for platforms which are purpose built to run them have a very different approach to the management of assets (such as image and sound data). On the Game Boy Advance -- the console on which Rhythm Tengoku and WarioWare run -- all assets must be stored on the 'GamePak'; a small cartridge that slots into the console itself. Once the game has booted up, any assets stored on the cart are accessible via mapped memory. This means that assets are not loaded in at will as games on other platforms need to do. Video games developed for a Windows computer, for example, may need to load data from any number of files into the virtual memory of the process in order to access them. This process requires significantly more effort, although there are many approaches with their own costs and benefits.

### Platform Layer
I sought to target Windows, macOS, and Linux for this project. I intended to implement the project atop the SDL2 platform layer to help facilitate development across multiple platforms. Once the project was more mature, I would replace the features used from this library with custom code on each platform. This platform layer provides -- among other things -- a way to create a window, open an audio device (via callback or buffer queueing), and handle user input via an event queue. In order to implement my own platform layer, I would need to implement separate methods of handling each of these things on each platform, and I felt that this was lower priority that other aspects of the project, so I left it as a possible goal for later on.

\newpage

# Design and Implementation
## Design of the Mini-Games
I will begin by discussing the design of the example game, as it informs the technical decisions made in supporting engine. There are three mini-games implemented in the final iteration of the project.

![Screenshot of the beating heart mini-game.](data/heart_shot.png)

The first mini-game requires players to press their buttons on the beat, alternating between each other. This is visualised with a beating heart: one player's press expands the heart, the other player's press contracts it. Together, they must hold a stable rhythm at a target beats per minute. Once they have held this rhythm for a number of seconds, they will have successfully completed the mini-game.

![Screenshot of the breathing lungs mini-game.](data/lungs_shot.png)

The second mini-game is based on the lungs, with players having to press, hold, and release in time with the beat, and do it together; their presses inhaling into the lungs, and their releases exhaling. This game is similar to the first, but they must press together instead of alternating. If both players can successfully keep a stable rhythm going at a target BPM they will have completed the mini-game.

![Screenshot of the digestive system mini-game.](data/digestion_shot.png)

The third and final mini-game is based on the digestive system, with one player's press moving food through the system, and the other player expelling the waste. This is done in 5/4 time, with one player tapping the first four beats, and the other player tapping the final beat. If the players can hold a target BPM at this time signature for long enough they will have successfully completed the mini-game.

## Technical Overview
<!-- Discuss the high-level design and structure of the software. -->
The project has five major sub-systems, and a core that ties them together. These sub-systems are Assets, Audio, Graphics, Memory, and Scenes. Each sub-system resides in a single C source file, and an additional file -- `main.c` -- ties them together, and defines the program entry point.

There are two major kinds of assets in this project: bitmap graphics, and PCM audio. Graphics are stored using a file format called Portable Arbitrary Map, which contains a brief ASCII text header followed by a block of uncompressed pixel data. Audio is stored in a custom file format designed to match the PAM format used for graphics, following the same principal of plain-text header and uncompressed raw data.

Audio is outputted to the sound hardware via a callback supplied by the platform (initially, the SDL2 library). This callback is run on a separate high-priority thread, meaning it is given more CPU time by the OS scheduler. In this callback, a custom audio mixer creates a single stream of floating-point PCM data and copies it into the output buffer. The mixer holds several separate sounds along with some parameters for how they should be played.

Graphics, much like sound, is handled by a custom renderer that produces a single final bitmap to be displayed on screen. While there is support for some graphical primitive rendering, the core functionality of the renderer is drawing bitmap graphics to the screen. These bitmap graphics can be any size, supporting transparency. They can also also be animated (composed of several bitmaps), or text, generated from a set of character bitmaps.

Memory is handled simply and efficiently by a custom memory allocator. The allocator holds a large pool of memory and sub-allocates from the pool on request. There are several sub-pools each allocating memory with a certain lifetime allowing freeing of memory to be automatic and efficient.

Scenes are used to encapsulate different pieces of the game. Each scene contains some state and a set of functions that handle input and output. This is where game-play code is written. Transitions between scenes can be made at will from anywhere in the program.

### Assets
#### Bitmap Graphics Files
Portable Arbitrary Map is a simple file format consisting of a plain (ASCII) text header and a block of uncompressed pixel data. It was chosen for its simplicity and widespread support in image conversion software. Here is an example file header for an image with a width and height of 256, in RGBA format, with one byte per channel:

~~~C
P7
WIDTH 256
HEIGHT 256
DEPTH 4
MAXVAL 255
TUPLTYPE RGB_ALPHA
ENDHDR
~~~

While the Portable Arbitrary Map format supports several pixel formats, for my project I am able to curate all of the files that will be read by my code, so I chose to only support the exact format that I would be using -- RGBA, with one byte per channel. With this in mind, I could parse the entire header with this single call to `fscanf`, given that all parameters except width and height are constant:

~~~C
fscanf(file,
       "P7\n"
       "WIDTH %d\n"
       "HEIGHT %d\n"
       "DEPTH 4\n"
       "MAXVAL 255\n"
       "TUPLTYPE RGB_ALPHA\n"
       "ENDHDR\n",
       &width, &height);
~~~

The `DEPTH` of 4 and the `MAXVAL` of 255 specify that the format is four channels, and that each channel occupies one byte as 255 is the maximum number a byte can hold. Once the header is parsed and the width and height are known, the pixel data can be read into a buffer as so:

~~~C
int pixels_read = fread(pixels, sizeof(u32), width * height, file);
~~~

Unfortunately, the pixel format used by .pam files is big-endian, and I am targeting little-endian machines[^5]. The byte order of each pixel must be swapped:

[^5]: Little endian machines are more common in consumer comuting as of writing; all x86 CPUs (which are very common) are little-endian.

~~~C
for (int pixel_index = 0; pixel_index < pixel_count; ++pixel_index)
{
    u32 p = pixels[pixel_index];

    // Pull out each component separately.
    u8 red   = (p & 0x000000ff) >>  0;
    u8 green = (p & 0x0000ff00) >>  8;
    u8 blue  = (p & 0x00ff0000) >> 16;
    u8 alpha = (p & 0xff000000) >> 24;

    // Combine the components in the correct order.
    pixels[pixel_index] = (red << 24) | (green << 16) | (blue << 8) | (alpha);
}
~~~

Finally the image's pixel data and dimensions are returned in an `Image` structure:

~~~C
typedef struct
{
    u32 * pixels;
    int width;
    int height;
}
Image;
~~~

#### PCM Audio Files
The format used for audio samples in this project is single-precision IEEE floating-point, making each sample 32 bits wide. All audio data has a sample rate of 48KHz. Here is an example file header containing one second (48000 samples) of audio:

~~~C
SND
SAMPLE_COUNT 48000
ENDHDR
~~~

Much the same as above, the header is parsed in a single call to `fscanf`, and the raw data read with a call to `fread` before returning in a `Sound` structure:

~~~C
typedef struct
{
    f32 * samples;
    int sample_count;
}
Sound;
~~~

As I had defined this format myself, and I knew that my target was little-endian machines, the raw data contained in this file is in little-endian format.

### Audio
<!-- Discuss the playback of audio and the mixer. -->
All sound is in 32-bit floating-point format at a 48KHz sample rate, the same format used in the mixer. This uniformity of format allows all audio data to be handled in the same way, without conversions during loading or transformation. Not all platforms support this format for output, so this it can be converted to the relevant format as a final stage before playback. Here is a basic example of how to convert to a signed 16-bit integer format:

~~~C
for (int sample_index = 0; sample_index < sample_count; ++sample_index)
{
    s16_samples[sample_index] = (u16)(f32_samples[sample_index] * 32767)
}
~~~

This works as sound in 32-bit float format expresses all waveforms in the range -1.0 to +1.0. Multiplying by the highest value that can be stored in a signed 16-bit integer (32,767) will produce an array of samples in the range (-32,767 to +32,767), which is correct for the audio format desired. One must be certain that their floating-point samples do not exceed the range -1.0 to +1.0 or the resulting integer values will wrap. Thus it may be sensible to clamp the value, which must be done *before* casting to `u16` but after the multiply, as the expression will be promoted to floating-point (as per the rules laid out in the C standard) and can hold any values that may exceed the desired range without wrapping.

#### Mixing
Audio is output by a high-frequency callback. This callback requests a number of samples, which is produced at will by the custom audio mixer. This audio mixer has a list of all playing sounds and their current state, and uses this to mix together a single stream of audio for playback. Each individual sound is simply a chunk of audio samples:

~~~C
typedef struct
{
    f32 * samples;
    int sample_count;
}
Sound;
~~~

Mixer channels are used to manage the playback of sounds:

~~~C
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
~~~

When one wants to play a sound, they must stop the audio callback and insert their sound into a channel. There is a fixed number of sound channels, so the first unused channel is found and used. Some parameters must also be set, such as how loud the sound should be, or whether it should loop.

~~~C
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
~~~

Sound can also be queued up, so that it is ready to be played by setting a boolean on the relevant channel. This is helpful for critical sounds as it reserves a channel, and it is possible to run out of channels and have an attempt to play a sound fail (although the number of channels is set to be far higher than the common case). One can also set the `sample_index` of a channel to a negative number. The mixer will increment this index without producing any sound until it is non-negative, and thus allows a sound to be queued up with sample-accurate timing.

\pagebreak

#### Synthesis
I had initially intended to perform live audio synthesis to generate sounds during the game. This was never implemented as it was a feature considered less important, and the development of the project had time constraints. However, the audio mixer does support synthesis, as one can write data directly into an audio channel's buffer and the mixer will output it.

### Graphics
<!-- Discuss general graphics info. -->
All graphics in the project use the 32-bit RGBA pixel format. This means that there are four channels, red, green, blue, and alpha (transparency), each of which is one byte large (holding values in the range 0 to 255), and stored such that the red byte is on the high end of the 32-bit value, and the alpha byte is on the low end. To write a pixel in C-style hexadecimal with the red, green, blue, and alpha values of 0x11, 0x22, 0x33, and 0x44 respectively:

~~~C
u32 pixel = 0x11223344;
~~~

These utility functions are also used to manipulate pixel data:

~~~C
// Pack an RGBA pixel from its components.
u32 rgba(u8 r, u8 g, u8 b, u8 a)
{
    return (r << 24u) | (g << 16u) | (b << 8u) | a;
}
~~~

~~~C
// Access individual components of an RGBA pixel.
u32 get_red(u32 colour)   { return (colour & 0xff000000) >> 24; }
u32 get_blue(u32 colour)  { return (colour & 0x00ff0000) >> 16; }
u32 get_green(u32 colour) { return (colour & 0x0000ff00) >>  8; }
u32 get_alpha(u32 colour) { return (colour & 0x000000ff) >>  0; }
~~~

#### Displaying Graphics
The software renderer holds an internal pixel buffer of a fixed resolution. This pixel buffer is where the results of the renderer are written. To display a frame on screen, the internal pixel buffer is sent to the GPU via a streaming texture for display. One can also directly access the window's buffer and copy the pixel data into it, but there is no guarantee of the format of these pixels (although one can detect and convert appropriately), no way to render with vertical sync to avoid screen tearing, and visual artefacts can occur when the window interacts with other programs, such as Valve's Steam Overlay. Thus, the GPU is utilised for the final stage of rendering, and also performs up-scaling to the full monitor resolution.

\pagebreak

#### Bitmaps
As all pixels are stored in the same format, the bulk of the work required to display an image on screen is done by directly copying data from the image's pixel buffer to the software renderer's buffer. As all pixel buffers are stored as a single array of packed RGBA values, a simple formula is used to access two-dimensional data from the one-dimensional array:

~~~C
int index = x + y * width;
u32 p = pixels[index];
~~~

Some checks can also be done to ensure that the pixel array will not be accessed out of bounds:

~~~C
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
~~~

But there is good reason to directly access the buffer without this check every time; if some operations need to occur in bulk, such as drawing an image into the buffer, checks should be made outside of the inner loop to improve performance.

To render a bitmap, one must copy pixel-by-pixel from an image buffer to the renderer buffer:

~~~C
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
~~~

The key aspect of this method is that one must keep track of both the current pixel on screen to be written to, and the current pixel in the image to be read from. Some checks are done to ensure that neither buffer will be improperly accessed (which is not shown here).

\pagebreak

#### Animation
<!-- Discuss the rendering of animations. -->
Animations have a time in milliseconds that states how long each frame of the animation will be displayed on screen. Using this time, and a time-stamp from when the animation started playing, the current frame can be deduced and displayed much the same way as a still bitmap.

~~~C
int time_passed = time_ms() - animated_image.start_time_ms;
if (animated_image.frame_duration_ms == 0) return;
int frames_passed = time_passed / animated_image.frame_duration_ms;
int current_frame = frames_passed % animated_image.frame_count;
int pixels_per_frame = animated_image.width * animated_image.height;
int pixel_offset_to_current_frame = pixels_per_frame * current_frame;
Image frame =
{
    .pixels = animated_image.pixels + pixel_offset_to_current_frame,
    .width  = animated_image.width,
    .height = animated_image.height,
};
draw_image(frame, x, y);
~~~

This method relies on the bitmap holding the frames of animation to store them vertically; with each succeeding frame below the previous. This differs from frames being stored horizontally as when vertically stacked, the *pitch* -- being the number of pixels between the start of each row of pixel data -- and *width* -- being simply the number of pixels to be draw -- are equal. If they were not the pitch would need to be calculated by multiplying the number of frames by the width of a single frame, so it is simply easier if these values are made equal.

#### Text
<!-- Discuss the rendering of text. -->
Text is rendered using a bitmap font; as opposed to generating font geometry on-the-fly. Fonts in this project are stored as a bitmap image holding all of the drawable characters defined in the ASCII standard, packed horizontally, width every character having the same width (mono-space). The location of a character in the image can be calculated using its ASCII code as an offset from the first character:

~~~C
int x = font.char_width * (string[c] - ' ');
~~~

The 'space' character is the first character defined in the font bitmap, and so has an offset of zero. Since all characters are represented as numbers, one can simply subtract the number that represents 'space', producing an index that, when multiplied by the width of a character, gives an x pixel coordinate denoting the top-left corner pixel of that glyph. Given that all of the glyphs are packed horizontally, every character's bitmap starts at a y position of zero. This gives enough information to render the glyph the same way all other bitmaps are rendered. One only needs to keep track of how far left they have moved after drawing each character to place the next letter correctly.

![A font bitmap containing all drawable characters of the ASCII standard.](data/terminus_12.png)

This system is incredibly simple and straightforward to implement. The downside is that it only supports ASCII characters and therefore only the English language.

### Memory
<!-- Discuss the management of memory. -->
While custom memory allocators are often considered a must-have for large high-performance video games\[12], many projects settle for standard memory allocation utilities, and don't consider the possibility of a custom solution. When building a replacement, one must think of how they can improve upon a general purpose allocator's offerings; in performance, in ease of use, or other factors, otherwise it is not worth the effort.

As with any general-purpose or generic approach, there are trade-offs. A general purpose allocator tries to do its best at the problems of wasting as little memory as possible, allocating and deallocating as fast as possible, and avoiding fragmentation. I propose that it can be very easy to beat a general purpose allocator (such as `malloc`) on all of these fronts, simply by constructing a solution that more directly supports the project that is being made.

Here is my assessment of the memory concerns for my project:

**Assets live forever.** Graphics and sound clips are loaded from disk and must be stored for the entire lifetime of them program.

**There can only be one active scene.** The active scene is swapped out at will, and if a scene is returned to, it starts from the beginning; no state is preserved.

**Some things only last one frame.** Some objects are created for rendering purposes, such as dynamic strings, and will simply be discarded after use. One could use stack memory for some of these things, but it would be more reliable to have some way to ensure their allocation.

#### The Solution
The project employs a pool-based approach. It is very simple, with the main allocation function consisting of only 12 lines of code. This allocator follows a principal often employed in high-performance video games: allocate up front, and sub-allocate after that point. There are three pools: the persistent pool, the scene pool, and the frame pool. The persistent pool performs allocations that are never deallocated. The scene pool is emptied when the scene changes, so the next scene has the entire empty pool. The frame pool is emptied every frame after rendering occurs.

~~~C
void * pool_alloc(int pool_index, u64 byte_count)
{
    void * result = NULL;
    byte_count = align_byte_count(byte_count);
    Memory_Pool * pool = &memory_pools[pool_index];
    if (pool->bytes_filled + byte_count <= pool->bytes_available)
    {
        result = pool->memory + pool->bytes_filled;
        pool->bytes_filled += byte_count;
        pool->byte_count_of_last_alloc = byte_count;
    }
    return result;
}
~~~

This is both easier to use (as `free` never needs to be called) and faster than `malloc` as the implementation is vastly simpler. Fragmentation as also impossible as individual allocations cannot be freed and so no empty holes in the pool can appear. The trade off is more memory is allocated than absolutely necessary, but in the final iteration of the memory allocator only 32 megabytes are allocated up front, and that limit has never been reached in my testing.

##### The Only Allocation
There are many options one can choose when allocating memory. There are language-level features, such as the `malloc` family of functions, or `new` in other languages. There are OS API calls, such as `VirtualAlloc` on Windows, or `mmap` on UNIX and other POSIX-compliant systems. Allocations can also be made using stack memory, or static memory. There are trade-offs for each, so I first had to discover what my requirements were to make an educated decision.

As stated earlier in this section, I calculated that a fixed upper limit of 32 megabytes would satisfy the needs of my project. This ruled out stack memory, as the default stack limit in clang and GCC is 8MB (which I found by calling `getrlimit`)\[2], and in MSVC it is 1MB\[5]. While the limits of stack memory can be specified at link time, this would require different actions to build the program on each platform and tool-chain.

I wanted to avoid the overhead of calling `malloc`, as I did not need its internal management of memory. I also wanted to avoid platform-specific functions if possible, but since this case would likely be a single function call I was happy to make an exception. I initially allocated the memory using `mmap`, and only used a subset of its features so that it would be compatible on Linux and macOS. I then put in a compile-time condition to use `VirtualAlloc` if building on Windows. Since I happened to be using the MinGW tool-kit on Windows, `mmap` was also supported, so both options are available.

After some time, I returned to this question. I had noticed that I had not considered another option: static memory. Considering that I had modest requirements, I looked into what the characteristics of static allocation are. On Windows, static data is limited to 2GB on both 32-bit and 64-bit versions of the OS\[1]. On macOS and Linux, I could not so easily find an answer, but in my own testing the program would crash at values higher than 2GB on macOS, but did not crash on Linux at any value. These limits are far beyond what I require, and the implementation is simpler than the previous; both in that it does not require even a function call, and that it is identical on all platforms. This is the implementation that remains in the final iteration of the project.

A compile-time flag allows the use of the previous `mmap` implementation, as it produced better memory debugging information on my macOS machine.

Kind         Limits[^6] Cross-platform Notes
----------   ---------- -------------- ----------------------------------------
Stack        1MB        Yes            Flexible size, if supported by platform.
Static       2GB        Yes            Fixed size only.
malloc       None[^7]   Yes            Internally manages pages.
mmap         None       Some           Allows access control, including execut-
                                       able pages.
VirtualAlloc None       No             Same as above.

[^6]: Lists the most limiting values across the Windows, macOS, and Linux platforms: those targeted by this project.

[^7]: Note that limits marked as 'None' are still limited by the platform, including hardware and decisions made in the OS.

##### Pre-faulting Pages
On most modern operating systems, memory pages are not actually allocated when requested, but when modified; this first access causes a page fault which forces the operating system to map that memory to the process, thus allocating it. If the program never touches the memory, it is not actually allocated. This is counter-acted by the `mmap` argument `MAP_POPULATE`\[6], which pre-faults every allocated page to ensure they are available and there is no overhead when accessing pages for the first time. This argument is unfortunately not cross-platform (it is not available on macOS), so immediately after the initial allocation is made, the game manually accesses the first byte of each page to ensure that all pages are readily available.

##### Thread Safety
One aspect not implemented in the final code-base, but researched, was making the allocator thread-safe. Initially, my implementation used a `mutex` to guarantee that allocations could be made from any thread without interference, but after more of the project was implemented, I found that I never allocated memory from a thread other than the main one, and so this feature was removed as it added cost with no benefit.

### Scenes
Scenes are sets of function pointers that can be swapped out at will. These function pointers are called at specific times, including when the user presses a button, or when a frame of graphics needs to be rendered.

~~~C
typedef void (* Frame_Func)(void * state, f32 delta_time);
typedef void (* Start_Func)(void * state);
typedef void (* Input_Func)(void * state,
    int player, bool pressed, u32 time_stamp_ms);

typedef struct
{
    Start_Func start;
    Frame_Func frame;
    Input_Func input;
    void * state;
}
Scene;
~~~

Each mini-game is implemented in a single scene. When a scene begins, it's `start` function will be called. This will allow it to perform any necessary set-up. Once this has completed the next iteration of the main frame loop will call the `frame` function, which will manipulate the pixel buffer, rendering the scene to the screen. Any time a player presses a button, the `input` function will be called, with the ID of the player that pressed the button, and the time at which they pressed passed. All of these functions have access to the scene's `state`, which is a structure defined along with the scene functions and will hold any state that is required to run the scene. Only one scene is active at a time, and the currently active scene can be changed using the function `set_scene`.

~~~C
bool set_scene(Scene scene)
{
    // Clear scene and frame memory pools.
    flush_pool(SCENE_POOL);
    flush_pool(FRAME_POOL);
    // Set function pointers.
    if (scene.start && scene.frame && scene.input && scene.state)
    {
        current_scene = scene;
        // Call the start function for the new scene.
        current_scene.start(current_scene.state);
        return true;
    }
    return false;
}
~~~

Within the scene functions, the game-play of the game is implemented. Audio can be played using the mixer functions, and graphics displayed using the renderer. Each scene has its own goals, and so calculates and keeps track of how well the players are doing itself. Once the players have achieved the goal of the scene, the `set_scene` function will be called from within the scene code and the next frame iteration will begin whichever scene was passed into that call.

### Building the Binary
For this project, I compile the entire source as a single translation unit. This is sometimes called a 'Unity Build'; achieved by using the `#include` preprocessor directive to combine all source files into a single file and compiling it. It builds faster than more traditional methods which wherein each file becomes a separate translation unit, and can allow the compiler to produce a better optimised binary. This method also makes header files unnecessary, as nothing needs to be forward declared when the entire code base coexists in a translation unit. The trade-off for this is that all root-level symbols once declared are global, so one must guard their naming of functions and types. In practice, this is very achievable -- especially for such a small project -- and the simplicity of not having to forward declare anything is very convenient.

\newpage

# Testing
<!-- Discuss the knowledge gathered about how games are tested. -->
In order to effectively iterate on the project, testing with users was a must. I gathered some information from testing with users here, and describes how the project (mostly the example game) was influenced by the feedback.

The mini-games were tested voluntarily with people at the Goldsmiths, University of London. Participants were a mix of people, some who stated that they play lots of games, and some who don't. Many participants were studying in fields surrounding computing, including game design and development. They were selected by availability (whoever was around during several unrelated public events), and each played the example game for up to five minutes.

Below is an overview of the information gathered from participants who took part in the testing of this project:

No. Input is Responsive Favourite Mini-Game Understood How to Play[^8]
--- ------------------- ------------------- --------------------------
1   Yes                 Heart               Partially
2   Yes                 Digestion           Yes
3   Unsure              Heart               Yes
4   Yes                 Digestion           Partially
5   Yes                 Digestion           No
6   No                  Digestion           Yes
7   Yes                 Lungs               Yes
8   Unsure              Heart               No
9   Unsure              Lungs               Partially
10  Yes                 Heart               Yes
11  No                  Heart               Yes

[^8]: Whether the person needed to ask me how to play the game, or if they understood intuitively.

Both participants 6 and 11 stated that they felt the input was not responsive. Both of these people also identified as being musically trained or having experience with performing music, so were likely used to the responsiveness of a musical instrument and found it hard to use the keyboard buttons as a substitute. Participants 1, 4 and 9 -- who each asked for assistance to understand how to play -- stated that they were not focussed on the sounds the game was making; these participants played a version of the game wherein the assisting UI is always on screen, which informed my decision to partially remove it. Some of the reasons given for favouring a particular mini-game were: "The animation is good."; "I think it is funny."; "I like that sound" (in reference to the 'shaker' sound used in the lungs mini-game).

## Confusion When Understanding the Goals
While one of my desires when designing the example game was to force players to figure out what they need to do, I found that many players with whom I tested the game got confused. In order to improve the experience of these players I added an overlay which appears if the players have spent a long time on a mini-game, but have not made any progress. This overlay attempts to give hints to the players by showing buttons on screen with arrows demonstrating when to press, and a scale which shows their accuracy.

## Testing the Heart Mini-Game
The heart mini-game is the first interaction with the game that players will have. Therefore, I started to test it early on in development. I wanted to avoid upfront explanation and tutorial, so needed to ensure that players could figure out what was happening, and what was expected of them. Here are some of the points that I received, which helped to inform the final mini-game:

### The Timing Bar
This mini-game contains a bar that indicates how accurate the players are tapping on the beat, on a meter of slow to fast with the correct timing in the centre. One piece of feedback that I received was that this bar looks like a 'charge-up' bar used in other video games, causing an interpretation that they should press their button when the dial on the bar is in the green area. They interpreted the bar to be giving instruction, instead of simply displaying state.

Mid-session, I turned off the entire interface, and the player quickly realised that they needed to be focussed on the sound and not the bar. I learned that while I constructed the interface to help players learn how to play, it could increase confusion about what the focus was, and what they were expected to do. This was the inspiration for the assisting overlay mentioned earlier in this section. The final version of this mini-game displays only the heart beating with the metronome sound to highlight that the sound is the focus, and then introduces the interface when the player begins to struggle.

## Testing the Lungs Mini-Game
This mini-game is meant to contradict the previous (Heart) mini-game by having the players do the opposite action; press together instead of alternating. This is confusing to players, but as with the previous, an overlay appears after some time to help guide them. I felt that players were not deterred by the lack of understanding of this mini-game, and enjoyed persevering and learning what they needed to do to succeed.

## Testing the Digestion Mini-Game
This mini-game focusses on the player's ability to keep time in a less common signature. Those who did play this game found it more difficult, some found it too difficult to complete. I added some sound cues to help players understand the order in which they must tap, which helped players know who's turn it was to tap, but not in keeping time correctly. After testing, I felt that this mini-game may be more difficult than the others, especially for players without a musical background.

\newpage

# Evaluation
This section contains an evaluation of a several features and aspects of the project. In general, the technical components are evaluated based on the factors of performance, ease of use, and ease of implementation.

## Rendering Graphics
The rendering system can display various kinds of graphics. It has the capability of drawing still bitmap images, frame-based animated images, rendering text, and drawing some graphical primitives.

#### Performance
I had the target of rendering at a minimum of 60 frames per second. In the final iteration of the project, the game runs at an average of 380FPS on my 2.3GHz laptop with integrated graphics, and runs at an average of 160FPS on the Raspberry Pi Model 3. I consider this good performance, and that my initial goal was met.

#### Ease of Use
The capabilities that the renderer presents to the example game are designed to support it as best as it can. There are several functions, for example the verbosely named `draw_animated_image_frames_and_wait` function allows the caller to specify a range of frames to draw to the screen, and once all of those frames have been displayed will leave the final frame on screen. This is a very specific feature, and it directly supports the desired use of the animation system in several areas of the example game. Considering that this is a specific requirement of the animations used in the game, it is likely that to achieve the same affect with other technologies additional work would need to be done -- it would not be as simple as a single function call.

Rendering text is done in a single function call, which takes in several parameters which affect the presentation of the text; font, colour and position. While passing these as parameters makes usage simple, many calls to the function have the same colour and font arguments. I could have simplified this system, having a way to set font parameters which persist between calls; though this has its own trade-offs.

One key aspect of the font rendering that was very flexible and useful, especially when debugging, is the use of a 'format string'. The C standard library function `printf` allows the caller to print the values of variables with the use of a format string, and this same functionality is supported in the renderer function `draw_text`.

I consider the resulting interface to the graphics renderer to be relatively simple and easy to use for the development of the specific example game. As the project was not intended to become a general purpose game engine, it has fulfilled its role as intended.

#### Ease of Implementation
The graphical rendering system has many features that support the example game. Despite providing a range of functionality, the system itself is quite simple. This can be described more concretely by the following facts: the file `graphics.c`, which contains the entire renderer implementation, is 268 lines long (including white space and comments.) which I consider to be quite short. While the number of lines of any piece of code does not accurately describe its complexity, it can give an idea of an upper ceiling of complexity. Also, the file defines 15 functions, and three data structures, which I would also consider a small number of each.

## Mixing and Playing Audio
The audio mixer can be used to play several pieces of audio at the same time. It has capabilities for managing the loudness of the audio, the stereo balance, and looping.

#### Performance
My initial goal for the performance of the audio mixer was simply to not drop any samples. This was achieved with minimal effort: the first iteration of the mixer successfully performed this. I had intentions to build a system to perform *live* audio synthesis which would have been more taxing on the hardware and may have required some more work to achieve good performance, but this was never implemented.

#### Ease of Use
Most of the use of the audio mixer is via the function `play_sound`, which, given some parameters including gain for each channel, and whether or not to loop the sound, will begin playback of a sound. This is very simple and easy to use. The mixer does not provide extensive functionality; most notably the absence of volume control over time. This was not implemented as it was not required for the example game.

#### Ease of Implementation
The audio mixer has very few components; it keeps a list of active sounds, each of which most importantly having an array of samples which are mixed together into a given buffer when `mix_audio` is called. Given that there is a very simple entry point and exit point for the data, with minimal state kept in between, I consider this system to be simple and the implementation did not occupy much of the development time of the project: approximately one week.

## Managing Memory
The custom memory allocator is used in many aspects of the project. For example, it is called to allocate space for every asset that is loaded, and for temporary memory when dealing with file paths.

#### Performance
The memory allocator performs a successful allocation in 8 instructions:

~~~ASM
pool_alloc:
  movsx rdi, edi                             1
  xor eax, eax                               2
  sal rdi, 5                                 3
  mov r8, QWORD PTR memory_pools[rdi+16]     4
  lea rcx, [r8+rsi]                          5
  cmp rcx, QWORD PTR memory_pools[rdi+8]     6
  ja .L1                                     7
  mov rax, QWORD PTR memory_pools[rdi]
  mov QWORD PTR memory_pools[rdi+16], rcx
  mov QWORD PTR memory_pools[rdi+24], rsi
  add rax, r8
.L1:
  ret                                        8
~~~

A pool deallocation is performed in 5 instructions:

~~~ASM
flush_pool:
  movsx rdi, edi                             1
  sal rdi, 5                                 2
  mov QWORD PTR memory_pools[rdi+16], 0      3
  mov QWORD PTR memory_pools[rdi+24], 0      4
  ret                                        5
~~~

While number of instructions is not a highly accurate measurement of performance, it is a reasonable indicator. I consider this system to be low cost, and therefore has met my performance goals for the allocator.

#### Ease of Use
The function `pool_alloc` is the main way in which the allocator is used. As this function allows the caller to specify which memory pool to draw the allocation from, any function which internally allocates memory via `pool_alloc` can pass this responsibility to its caller. This system allows functions such as `read_image_file` to load data from a file on disk into a memory pool of the caller's choosing. This selection of pools is the alternative to calling `free` on memory when a caller of `malloc` is done with it, as pools are self-managing. If one forgets to call `free` when necessary the program will *leak* memory. In this allocation system, memory leaks are not possible.

\pagebreak

#### Ease of Implementation
While there may not be a large amount of code in the allocation system, it was written taking into account much research. There are many factors, such as byte-boundary alignment, handling the initial allocation (discussed in section 4.2.4, under the subheading [The Only Allocation]), and ensuring the pools are of adequate capacity. The final allocator is simple and features many utilities that allow affective use of memory. One notably missing feature is 'reallocation', wherein memory is copied to a location with a larger capacity. This feature could have been used when loading in files from disk for example, but as the file formats were chosen to state their size in their header this was unnecessary. The basic implementation of the allocator was done in a matter of hours, but many small refinements were made throughout the project as more information on the topic was learned.

## Loading Assets
Assets are files loaded from disk. All asset loading occurs immediately on launching the game. Custom loaders are used to read in and interpret the data in these asset files.

#### Performance
My only concern for performance was that all asset files could be loaded at launch without a perceivable delay. This was achieved, although it is likely that the performance of this action was bound by the time taken to load data from disk to memory, and the code that I had written to perform asset loading had minimal impact on the performance.

#### Ease of Use
As mentioned earlier in section 6.3 ([Managing Memory]), all calls to functions that perform asset loading take in a parameter to select a memory pool to use. This, and the name of the desired file, are all that is needed to load an asset. There is another function, `load_assets`, which handles the loading of the specific assets that the example game requires. This function takes in a path to the folder which contains the assets. This extra functionality makes moving the binary or assets around in the file system much simpler, as one can simply insert the correct path into this function call.

#### Ease of Implementation
The asset formats are simple in that they do not use compression and have simple plain text headers. These aspects of the formats used make them easy to load, and the functions that perform this are not complex. Despite this, there were some details which needed to be handled correctly, including swapping the order of each pixel's channels in the `.pam` image format as they were stored in a different arrangement to what was required by the graphical renderer.

## Implementation of the Example Game
All of these technical systems were designed to support a specific game. This game consists of several mini-games featuring graphics and sound, and uses two buttons -- one for each player. These requirements were met directly by the technical systems: functions for rendering and playing audio were designed to be as simple as possible for use in the game. If the game wanted to perform an action that was not supported by the technical system, it was implemented. Thus, the resulting game and engine fit together. I argue that the resulting piece of software is relatively simple, unlike any game that is implemented in Unity, for example, as Unity is a complex piece of software, and if the game that is being developed with it does not require that complexity, the resulting piece of software as a whole is overly complex and will likely not perform (in CPU, memory, and storage usage) as well as it could.

\newpage

# Conclusion
In this section I will discuss various aspects of the project, including the development process and the resulting software, stating my thoughts on each topic.

## Self Imposed Limitations
To examine the process of developing this project, I begin by looking at the decision to use the C programming language. C is my preferred language, but it has trade-offs as with any language. C's main selling point is that it is *low-level*, providing more direct access to the underlying machine, though this is perhaps becoming less true as technologies change\[13]. In any case, it certainly facilitated my goal of furthering my understanding of how to implement many components of a video game and engine, as C does not provide the assistance that many contemporary languages provide in the form of language features or built-in libraries.

Secondly, I would like to evaluate my use of C standard library features, such as avoiding `malloc`. Implementing a specific-purpose memory allocator was a goal I had set, as purpose built memory allocators are used judiciously in video games\[12]. I felt that it was important to better my understanding of this topic. While I did not use `malloc`, there are other standard library features that I did rely on: `snprintf` was used in several places including formatting of strings for display on screen in the `draw_text` function, and for generating debug output. I considered building a replacement of this, as it is an area of great importance, but considering the scope of the project and that string formatting is less of a priority (though certainly used often) in video games, I left it off the table.

There is another major component of the project that I did not write: the platform layer. In the final iteration of the project, the window management, event queueing, graphical output and audio output are all handled by the SDL2 library. While this library is not used extensively -- there are many of its features that I avoided, including a complete 2D renderer -- the final executable is dependent on it. I initially intended to write platform layers for Windows, macOS and Linux after using SDL2 as a crutch to get the project going, but as it progressed I found that there were aspects of the project that I considered more important. I did design the inputs and outputs of the engine to be relatively simple to re-target, leaving the door open for future development: the renderer outputs a final bitmap in a common format, the audio mixer produces a buffer of any size on request, and user input can be sent in by a simple function call. Platform layers are also often considered boilerplate for video games\[14], as most video games want the same things from each OS's API. Considering this, I felt it acceptable to not delve into the development of a platform layer in this project.

\pagebreak

## The Engine and The Game
I began this document discussing the use of game engines in video game development, expressing that I considered their use subtly harmful to the medium -- a topic worthy of debate. I believe this project serves as a good example of technical solutions tailored to support the design of a specific video game. The example game makes use of every feature of the underlying engine, and this engine does not provide functionality that is not used by the game. As a result, the compiled binary does not contain anything unnecessary. The example game is, however, not especially complex, and it would be an interesting endeavour to construct an engine such as this for a more technically demanding game -- this is something to be done with a longer development time.

## Final Thoughts
To conclude this document, I will reflect on my initial goals for the project.

**To, wherever possible, only use code that I have written.** When stating this goal, I clarified that the reasoning behind it was to force myself to learn new techniques. While the project is not built off the bare metal -- or even atop the OS API, I still constructed many features that are often provided by libraries. This goal is achieved in part, but I am satisfied with the outcome.

**To tailor every aspect of the code to the design of the game.** This point is my personal rally against the widespread practices of building generic code. In the final iteration of the project, I feel that there are few aspects of the code base that do not serve their purpose simply and succinctly, and so am pleased with the results.

**To produce a good quality piece of software.** This goal stated some concrete objectives. Firstly, hitting a frame-rate target of 60FPS: this was done successfully. Secondly, playing audio without hitches or dropped samples: also successful. Finally, to respond to input within 10ms: this objective was is harder to test. Since input is handled within the frame in which it was detected, I can be sure that my own code responds to this detection within this time. Taking my laptop hardware as an example, hitting an average frame-rate of 380FPS means an average frame duration time of 2.6ms, which is well below 10ms. It is possible that there is some amount of delay incurred by the operating system's handling of the input, or the hardware itself, and so the time between depression of a button and output from the game may be longer than 10ms.

**To learn about many areas of game development, and document them.** This is the overarching goal for the project. Being an undergraduate dissertation, I feel that learning is the true outcome to strive for. There are many techniques that I studied intensively in order to produce this project, including memory allocation, 2D rendering, file management, game-play programming, input handling, and more. When I look over the code base, I can see the ways in which I have improved as a games programmer, and I have a strong understanding of the processes going on in the code. To understand how video games work; this was my true goal when starting this degree.

\newpage

# Bibliography
[1]\
Lionel, Steve (Intel); 2011\
*Memory Limits for Applications on Windows*\
https://software.intel.com/en-us/articles/memory-limits-applications-windows\

[2]\
Linux Manual\
*setrlimit(2)*\
https://linux.die.net/man/2/setrlimit\

[3]\
Linux Manual\
*getpagesize(2)*\
https://linux.die.net/man/2/getpagesize\

[4]\
GBATEK\
*Gameboy Advance / Nintendo DS / DSi - Technical Info*\
http://problemkaputt.de/gbatek.htm#gbatechnicaldata\

[5]\
MSDN Documentation\
Compiler Options\
*/F (Set Stack Size)*\
https://msdn.microsoft.com/en-us/library/tdkhxaks.aspx\

[6]\
Linux Manual\
*mmap(2)*\
https://linux.die.net/man/2/mmap\

[7]\
*Made with Unity*\
https://unity.com/madewith\

[8]\
*Unreal Showcase*\
https://www.unrealengine.com/en-US/blog/category/showcase\

[9]\
*Games Made with Love2D*\
https://love2d.org/wiki/Category:Games\

[10]\
*Games Made with SFML*\
http://www.indiedb.com/engines/sfml/games\

[11]\
Green, Berbank; 2005\
*One Button Games*\
https://www.gamasutra.com/view/feature/130728/one_button_games.php\

[12]\
Acton, Mike; 2014\
*CppCon 2014: Mike Acton "Data-Oriented Design and C++"*\
https://youtu.be/rX0ItVEVjHc?t=10m46s\

[13]\
Chisnall, David; 2018\
*C Is Not a Low-level Language*\
https://queue.acm.org/detail.cfm?id=3212479\

[14]\
Muratori, Casey; 2014 (updated 2018)\
*Windows Platform Layer*\
https://www.youtube.com/playlist?list=PLEMXAbCVnmY4ZDrwfTpTdQeFe5iWtKxyb\

[15]\
Muratori, Casey; 2018\
*The Thirty Million Line Problem*\
https://www.youtube.com/watch?v=kZRE7HIO3vk\

[16]\
Simon Peter, Jialin Li, Irene Zhang, Dan R. K. Ports, Doug Woos, Arvind Krishnamurthy, Thomas Anderson -- University Of Washington\
Timothy Roscoe -- Eth Zurich\
2015\
*Arrakis: The Operating System Is the Control Plane*\
https://www.inf.ethz.ch/personal/troscoe/pubs/peter_tocs_15.pdf\

\newpage

# Appendix A
## Thoughts on the Use of Game Engines, and the Preservation of Video Games
Reliance on technologies that you as the developer do not understand and cannot maintain (especially if the source is not available) can shorten the lifespan of the games you make, as they may have dependencies which are not maintained and become non-functioning as the environments that they run in change. As video games grow in cultural relevance, not being able to preserve them becomes a troubling prospect.

One could also argue the opposite. In some cases, the use of libraries becomes a method by which games live on after their original developers have stopped working on them. But, this is only possible under certain conditions. The developer of the original software must allow some way for the library included in their project to be updated or maintained, like using dynamic linking. In this case, the developer of the library in question must have very strict rules about the maintenance of the Application Binary Interface to ensure that a new versions of a static library interoperate with software that has linked with an older version.

There are also situations where the use of libraries and engines does not have an affect on the lifespan of the video games that use them. Video game consoles are traditionally a well defined and unchanging platform; if a game is released on a specific console, it will still work years later on that same console. If a game uses an engine or library that works at the time of release, its unlikely that will change over time. But, releasing a game on a console has other concerns for preservation as consoles may stop being manufactured, and over time the number of working units may decrease. Unless efforts are made to preserve the environment in which the game runs, such as hardware emulation, it may become difficult to play the game at all. Emulation becomes more difficult to pull off as hardware grows more complex and Moore's Law breaks down. And, now that complex operating systems are commonplace on modern consoles, one must obtain or somehow emulate this as well.

For video games to be culturally significant one should be able to examine the history of the medium, as it will hold clues about wider culture through time just as other mediums do. I argue that releasing the source code of games is one of the few ways one can help to secure their existence in the future. This is evidenced by the releasing of the source of many Id Software games; The many of the Doom and Quake series games continue to be played in years after they were released, and people also find value in their source code alone.

Until we as developers stop believing in the idea that we loose when other gain from our knowledge, we will be held back from becoming an undeniable art form. A painter doesn't actively hide their technique from others, nor a musician.

\newpage

# Appendix B
## Original Project Proposal
This section contains the project proposal, handed in on the 15th of December, 2017. This initial proposal differs in many ways from the final iteration of the project. My own goals for the project changed; my interests changed from the creation of a video game to the creation of supporting technologies. Also, note that this document still proposes the development of many custom components.

### Final Project
BSc Games Programming
Year 3, 2017 - 2018
Benedict Henshaw

#### Overview
> Competitive local multi-player one-button rhythm game.

My final project is a video game. It is a rhythm game in the vein of 'Rhythm Tengoku' and the 'WarioWare' series, and will be all about keeping time with music.

The focus is on multi-player mini-games which each have their own spin on keeping time.

#### Mini-Games
I would like to implement at least one of these mini-games by the end of my project.

##### Arm wrestling
Each player's button pressing controls one arm. The arm that pushes the other over wins.

##### Possible mechanics
They must press in time with the beat, but they can choose at what rate they press. The force with with each arm is pressing is based on the accuracy and the rate at which they press. Whichever arm has the highest force will push the other until one arm is bent completely over.

##### Horse racing
Each player's pressing controls a horse in a race. The horse that reaches the end fastest wins.

##### Possible mechanics
To run faster the player must press at different time signatures and rhythms. Their speed is based on accuracy and the rhythm at which they press.

The player must transition into the next rhythm to increase speed.

##### Factory
Each player controls a robotic arm that is taking objects off a conveyor belt in a factory. The robot that has successfully collected the most objects or reaches a target amount wins.

##### Possible mechanics
Both players press once to put the object on the conveyor belt. The object can end up further to the left or right of the belt based on who was most accurate in the first press. Then they must press again to pick up the object.

#### Technology
I want to write the program in C, using SDL2 as a platform layer. I will write a custom audio back end and sequencer for playing custom audio tracks and performing some on-the-fly synthesis. The graphics will be handled by a custom 2D software-based renderer. I will handle memory using some custom allocation techniques, but this game does not require much data throughput so these systems will be relatively simple. Some basic multi-threading will be used; possibly employing a job system. I hope to achieve low-latency input and audio output, and 60 FPS video output.

##### Input
When the user presses a button, that event will be queued. During the next frame, the event queue will be run through and each event handled. This may be done more often if the game does not feel responsive enough. These events are timestamped and I will use that time stamp to check if a player has pressed the button in time with the music. I would like to allow external devices like game controllers to be used in the game too.

##### Audio
I will be employing a custom audio mixer for this project. I will allow for mixing of several 'channels' of PCM audio data. This will allow playback of multiple sounds at once, and may be used to handle individual 'instruments' in an audio track. It will also allow me to know with high accuracy how much music has played and this will help match input to audio output.

A sequencer will handle playing music. The sequencer will have 'instruments' that can produce sound at will with some parameters. There will be at least two kinds of instruments: sample-based and synthesis-based. Sample-based instruments will simply write some pre-recorded sound data into the buffer, while synthesis-based instruments will generate sound data on-the-fly. I would also like to employ some audio effects, such as delay and some filters (low-pass, high-pass, etc.).

I may need to write a secondary tool for authoring audio sequences, though it may just be a system that interprets a plain text format.

##### Graphics
A custom 2D software renderer will fill a buffer with pixels each frame. The most important feature will be a simple sprite copy, but will also have rotation and scaling features. It will handle text using a bitmap font system. Some graphical primitives may also be implemented. Alpha keying (if not alpha blending) will allow for transparency in images. A sprite sheet will contain all graphical elements and the renderer will have a table of locations for each sprite.

##### Data
Graphical data will be handled in the form of '.png' or '.bmp' images. I will opt for '.png' if the total file size of the images becomes inconveniently large using the uncompressed '.bmp'. Sprite sheets will allow lots of graphical components to exist inside one file, which will improve performance when loading data from disk.

Audio sample data will be in '.wav' format unless there is so much of it that a compressed format is needed. In that case I will use '.ogg' as there is a high quality public-domain library available for decoding. 'Audio fonts' will allow lots of separate sounds to share a single file, to improve load performance (and decode performance if used).

A basic (probably plain-text) file format will be used if any player settings or save data needs to be stored.

\newpage

# Appendix C
## Weekly Development Logs
This section contains several blog posts made during the development of the project. They are listed in chronological order, and presented unedited.

### Getting Started (2017-09-30)
I want to build rhythm game for my final year project.

#### Design
My favourite in the genre is Rhythm Tengoku (2006, GBA), which is mini-game based with a comedic theme. This game is single-player, with each mini-game requiring the player to demonstrate some ability to stay in time with music, but each having its own take on what the player should be doing. The game does not use many buttons for this as it is not about reflexes and hyper-awareness so much as a raw sense of rhythm and use of logic.

Dance Dance Revolution (PS, 1998) and Guitar Hero (PS2, 2005) are also rhythm games, but their focus (and theme) are very different. Both are score focused, with competitive elements, where the player has a higher score the more beats they hit successfully, and the player with the highest score wins. These games are also more reflex based, with an emphasis on memorising the patterns of button presses mapped to a song. I find this less interesting, and can be alienating to those who may not have the physical capacity to play them, or the time to memorise songs. This is not the kind of game I would like to make, but I do like the competitive nature of them.

I would like to borrow the aspects of these games that I like for my project, and include new features that the genre has not seen yet.

### Building the Basics (2017-10-15)
I've put together some code that demonstrates the basics that I will need for my game. I have graphics with rectangles and line drawing, and some simple audio output via a callback.

I have been experimenting with getting reasonable input to audio output latency. On macOS I can set my buffer size to 1 sample (which doesn't actually feel different from say, 64 samples), so output latency is not bad. I am almost happy with the latency, but it could be better. I think the first problem to tackle lies with my input handling.

Each frame I check the event queue for input, handling any that have been buffered. The events can come in at any time, but I only act on them once at the start of the frame. Since my graphics code supports v-sync my frame time is a reliable 16ms, I will assume my input latency is at least 16ms. At least, that is the amount of delay I may have control over. If I disable v-sync I do find that I can feel the difference when using keyboard input to control audio out. I like having v-sync (and it drastically reduces CPU/battery usage), so I may have to find a way to check for input more often without being tied to graphics output.

### Program Structure (2017-11-20)
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

### Memory Allocation (2018-02-15)
For my project, I have employed a very simple and fast method of memory management. It is centred around the concept of memory pools, which are stack-like structures that have a fixed amount of memory under their control. When an allocation is made from a pool, it selects some memory from the top of its pool and returns the address of that memory.

The allocated memory is also ensured to be aligned correctly for any type. This can be ensured by making the address of the returned pointer is a multiple of the largest alignment needed. There is a C language type called `maxalign_t`, and taking the size of this type will give you the maximum alignment needed. On two Intel machines I tested this gave 16 bytes. Not aligning correctly will only cause access to that memory to be slower, nothing fatal.

I have three memory pools, the persistent pool, scene pool, and frame pool. Each of which defines a lifetime of the memory it allocates. Any memory allocated by the persistent pool will live for the entire lifetime of the program. Scene pool allocated memory will live until the scene changes. The frame pool is reset after every frame. Individual allocations cannot be arbitrarily freed when allocated by a memory pool, all memory is deallocated when the pool is reset. One simply has to decide how long the allocation should last when allocating, and choose the appropriate pool.

This is a convenient way to manage memory as any functions that would like to allocate memory can ask the caller which pool they would like to use, giving more flexibility to the caller. It is also far faster than `malloc` or `new`, and as it does not have the concept of granular deallocation, and so does not force the overhead of `free`/`delete` on the caller.

The trade-off is that more memory may be allocated to the program than needed; although this is counteracted by the fact that operating systems don't actually allocate memory pages to programs until they attempt to write to them. This adds some overhead, so I may want to touch every allocated memory page at start up to ensure that is ready to go before an allocation is made. Overall this solution is better (faster, simpler), mostly because is is tailored to suit the needs of my game, and general purpose allocators are not.

The fixed amount of underlying memory managed by the pools is small enough (in my case) to comfortably allocate completely in static memory. Some research lead me to believe that 2GB is a reasonable maximum of static memory to assume (https://software.intel.com/en-us/articles/memory-limits-applications-windows).

### Assets, Graphics and Animation (2018-01-10)
Following the theme of do-it-yourself, I have written an image file reader and writer. It operates on Portable Arbitrary Map (`.pam`) files. The format is very simple, with a short plain-text header and uncompressed contents. As this game does not rely on high resolution graphics, it is perfectly reasonable to use uncompressed data for graphical assets.

Now that I can easily import (and export) pixel data, I have fleshed out the rendering system. At its core, my renderer has an internal pixel buffer. This buffer is where everything is rendered. Finally, this entire buffer is displayed on screen. This step can done by sending the pixel data to the OS, or by copying the data to a streaming texture in video memory and displaying with the GPU.

#### Animation
Rendering still images is very useful, but I would like to produce animations that are made up of multiple frames. In order to achieve this, I first need to consider how I will deliver this data to the program.

Bitmap rendering in this project is often concerned with raw pixels. The formula `x + y * width` is employed throughout the renderer to access, set, and copy pixels. `width` is actually not the most correct way to describe this formula; `pitch` is more correct than `width`. The difference is that while you might want to draw an image of width `20px`, The pixels of that image might have come from a much larger image. `pitch` is the width of entire pixel buffer. This is a very common feature of renderers often called a texture atlas (or sprite sheet): an image that holds many images packed together.

I noticed that I could lean on my previous image rendering code if I made my `pitch` and `width` equal. This can be done by packing in images vertically; each consecutive frame is underneath the previous. I would simply have to pass in a pointer to the top-left-most pixel of the frame that I want. It is a simple equation to calculate this:

~~~C
// To render frame 2 of an animation:
int animation_frame = 2;
int pixels_per_frame = width * height;
int pixel_offset_to_current_frame = pixels_per_frame * animation_frame;
u32 * final_pointer = pointer_to_start_of_image + pixel_offset_to_current_frame;
~~~

### Rendering Text (2018-01-13)
While text in most modern programs is rendered on the fly to allow scaling and differing pixel densities, my program has a fixed internal resolution, so the result of this text rendering is known ahead of time. This reason, and the fact that the implementation is far simpler, lead to my decision to render text the same way I handle animations: an atlas of sub-images.

Firstly, I wanted a way to print anything, including the values of variables. I used the standard library function `vsnprintf` to achieve this. The `v` in its name states that it allows the use of a variadic function argument list to provide its arguments; the `s` states that it will write its result to a string; the `n` means that a hard character limit must be specified. Aside from these aspects, it performs the same function as `printf`: it converts data into strings.

Once I have my string, I must find some way to map the characters to bitmap images in my atlas. I could make a list of characters and their associated `x, y` positions in the bitmap, but this is more time consuming than I would like. The method I used leans on the fact that all characters are of course represented by numbers, and these number have a reliable ordering. At this point I made the concious decision to only support the English language in my initial implementation, and to rely on the ASCII standard for character representation.

An important characteristic of the ASCII standard is that all 'drawable' characters are grouped sequentially. All characters from `!` (value 33) to the character `~` (value 126), are drawable, in that they are not white space or a 'control' character. Knowing this, I could use the character's value to calculate an index into my font bitmap.

In order to keep the rendering process simple, I opted for mono-space fonts, where I will always know the width and height of every character, as they are all equal. Using a font bitmap that packs each glyph horizontally, starting at the space ' ' character (value 32), here is how I calculate the index:

I index the string to get the character I want, then subtract ' ' (32) from it to map it to my index which starts from zero.

~~~C
int top_left_x = character_width * (text[c] - ' ')
~~~

This value, and the fact that each glyph is packed horizontally, means that I know where in my texture atlas my glyph is (`top_left_x, 0`), and can render it.

As I mentioned in my previous post, I will also need the `pitch` of my texture atlas to index into it. I could save this separately, but it is derivable from the width of a character. 96 is the difference between `'~'` (126) and `' '` (32) (the total number of characters in the font bitmap), but it starts from zero so -1.

~~~C
int pitch_of_font_bitmap = 95 * character_width;
~~~

This combined with keeping a sum of how far towards the left I move after rendering each character gives me a simple and fast way to render text. I can easily use this render text into a buffer if it is being used frequently.

### The Audio Mixer (2018-01-21)
In the same way that I send a single pixel buffer to the screen, I can only send one audio signal to the sound card. Therefore, in much the same way as graphics, I need a way to combine all of my playing sounds into a single stream of audio. This is commonly called a mixer.

At its most basic, a mixer will add each sample in each stream together:

~~~C
...,  0.3, -0.5,  0.1,  0.2, ...
...,   +     +     +     +   ...
...,  0.1,  0.2, -0.1,  0.3, ...
...,   =     =     =     =   ...
...,  0.4, -0.3,  0.0,  0.5  ...
~~~

The resulting stream of numbers will be the mixed sound, and will appear to contain both sounds.

My implementation of audio mixing is based on `channel`s. Each `channel` can hold one sound (an array of numbers), and a set of parameters for playing the sound, such as how loud it should sound in each ear. These channels also keep track of how much of the sound has been played, and what should be played next. I have a fixed (but quite large) number of channels, as this is both simple and efficient.

Since the frequencies of audio data are much higher than video (48000hz vs 60hz), the data often needs its own thread to ensure that it is being handled without delay. While I investigated handling the audio on the main thread (as it would provide better synchronisation with input events), it was difficult to avoid occasional hitches in the sound as the frame loop is not currently time-locked strongly enough. Thus I conceded to using another thread in the form of an asynchronous callback. This meant that the data shared between threads must be handled carefully, and ideally not modified too often. I used a simple built-in locking mechanism to achieve synchronisation, and set my buffer size to a relatively small value (64 samples) to keep latency low. The performance of this implementation is adequate, but I will keep a close eye on it to see if it must be improved later on.

\newpage

# Appendix D
## Source Code
This section presents the source code of the project in its entirety. It is presented, arranged by source file, in an order designed to assist in the reading of the code.

#### main.c
~~~{.c .numberLines}
//
// main.c
//
// This file contains:
//     - Program entry point
//     - Initialisation for graphics and audio.
//     - Frame loop.
//     - Main audio callback.
//

// Compile time options for the memory allocator.
#define POOL_STATIC_ALLOCATE
#define POOL_STATIC_PERSIST_BYTE_COUNT (32 * 1000 * 1000)

// External includes here:
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <SDL2/SDL.h>

// The entire project is a single compilation unit.
// Everything is included here:
#include "common.c"
#include "memory.c"
#include "graphics.c"
#include "audio.c"
#include "assets.c"
#include "scene.c"

//
// Main audio callback.
//

void audio_callback(void * data, u8 * stream, int byte_count)
{
    Mixer * mixer = data;
    f32 * samples = (f32 *)stream;
    int sample_count = byte_count / sizeof(f32);
    mix_audio(mixer, samples, sample_count);
}

//
// Program entry point.
//

int main(int argument_count, char ** arguments)
{
    // Use unbuffered logging.
    setbuf(stdout, 0);

    //
    // Initialisation.
    //

    if (!init_memory_pools(megabytes(32), megabytes(8), megabytes(4)))
    {
        panic_exit("Could not initialise memory pools.");
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        panic_exit("Could not initialise SDL2.\n%s", SDL_GetError());
    }

    //
    // Init graphics.
    //

    SDL_Window * window = SDL_CreateWindow("",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH * 2, HEIGHT * 2, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        panic_exit("Could not create a window.\n%s", SDL_GetError());
    }

    SDL_SetWindowMinimumSize(window, WIDTH, HEIGHT);

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/);
    if (!renderer)
    {
        panic_exit("Could not create a rendering context.\n%s", SDL_GetError());
    }

    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);
    SDL_RenderSetIntegerScale(renderer, true);

    SDL_Texture * screen_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT);
    if (!screen_texture)
    {
        panic_exit("Could not create the screen texture.\n%s", SDL_GetError());
    }

    pixels = pool_alloc(PERSIST_POOL, WIDTH * HEIGHT * sizeof(u32));
    set_memory(pixels, WIDTH * HEIGHT * sizeof(u32), 0);

    SDL_ShowCursor(false);

    //
    // Init audio.
    //

    mixer = create_mixer(PERSIST_POOL, 64, 1.0);

    SDL_AudioSpec audio_output_spec =
    {
        .freq = 48000,
        .format = AUDIO_F32,
        .channels = 2,
        .samples = 64,
        .callback = audio_callback,
        .userdata = &mixer,
    };

    audio_device = SDL_OpenAudioDevice(NULL, false,
        &audio_output_spec, NULL, 0);
    if (!audio_device)
    {
        panic_exit("Could not open the audio device.\n%s", SDL_GetError());
    }

    SDL_PauseAudioDevice(audio_device, false);

    //
    // Set up the timers.
    //

    u64 previous_counter_ticks = SDL_GetPerformanceCounter();
    f32 counter_ticks_per_second = SDL_GetPerformanceFrequency();

    //
    // Load assets.
    //

    if (!load_assets("../assets/"))
    {
        panic_exit("Could not load all assets.\n%s", strerror(errno));
    }

    //
    // Initialise any connected input devices.
    //

    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        SDL_JoystickOpen(i);
    }

    //
    // Start the game.
    //

    blank_cut(2.0, 0, &heart_scene, NULL);
    play_sound(&mixer, assets.clock_sound, 1.0, 1.0, true);

    while (true)
    {
        // Update timers.
        f32 delta_time = (SDL_GetPerformanceCounter() - previous_counter_ticks)
                            / counter_ticks_per_second;
        previous_counter_ticks = SDL_GetPerformanceCounter();

        // Handle events since last frame.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
#ifdef DEBUG
                print_memory_stats();
#endif
                exit(0);
            }
            else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                if (!event.key.repeat)
                {
                    SDL_Scancode sc = event.key.keysym.scancode;
                    if (sc == SDL_SCANCODE_LSHIFT)
                    {
                        current_scene.input(current_scene.state, 0,
                            event.key.state, event.key.timestamp);
                    }
                    else if (sc == SDL_SCANCODE_RSHIFT)
                    {
                        current_scene.input(current_scene.state, 1,
                            event.key.state, event.key.timestamp);
                    }

                    // DEBUG:
                    if (event.key.state)
                    {
                        if (sc == SDL_SCANCODE_1)
                        {
                            set_scene(heart_scene);
                        }
                        else if (sc == SDL_SCANCODE_2)
                        {
                            set_scene(lungs_scene);
                        }
                        else if (sc == SDL_SCANCODE_3)
                        {
                            set_scene(digestion_scene);
                        }
                        else if (sc == SDL_SCANCODE_I)
                        {
                            if (event.key.state)
                            {
                                heart_state.draw_interface = !heart_state.draw_interface;
                                lungs_state.draw_interface = !lungs_state.draw_interface;
                            }
                        }
                        else if (sc == SDL_SCANCODE_O)
                        {
                            if (event.key.state)
                            {
                                heart_state.target_beats_per_minute += 10;
                            }
                        }
                    }
                }
            }
            else if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP)
            {
                current_scene.input(current_scene.state, event.jbutton.which & 1,
                    event.jbutton.state, event.jbutton.timestamp);
            }
            else if (event.type == SDL_JOYDEVICEADDED)
            {
                SDL_JoystickOpen(event.jdevice.which);
            }
        }

        // Render the scene.
        current_scene.frame(current_scene.state, delta_time);

#ifdef DEBUG
        draw_text(assets.main_font, 270, 226, ~0,
            "FPS: %.0f", 1.0f / delta_time);
#endif

        // Render the internal pixel buffer to the screen.
        SDL_RenderClear(renderer);
        SDL_UpdateTexture(screen_texture, NULL, pixels, WIDTH * sizeof(pixels[0]));
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
}

~~~

#### common.c
~~~{.c .numberLines}
//
// common.c
//
// This file contains:
//     - Primitive type definitions.
//     - Pseudo-random number generator and utilities.
//     - Error reporting functions.
//

//
// Primitive type definitions.
//

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float  f32;
typedef double f64;

typedef _Bool bool;
#define true 1
#define false 0

//
// Common utility functions.
//

#define swap(a, b) { __typeof__(a) tmp = a; a = b; b = tmp; }
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define clamp(low, value, high) max(low, (min(value, high)))

//
// Pseudo-random number generator.
// (Xoroshiro128+)
//

u64 random_seed[2] = { (u64)__DATE__, (u64)__TIME__ };

u64 random_u64()
{
    // Get the next random number.
    u64 s0 = random_seed[0];
    u64 s1 = random_seed[1];
    u64 result = s0 + s1;

    // Increment the generator.
    s1 ^= s0;
    #define LS(x, k) ((x << k) | (x >> (64 - k)))
    random_seed[0] = LS(s0, 55) ^ s1 ^ (s1 << 14);
    random_seed[1] = LS(s1, 36);
    #undef LS

    // Return the number.
    return result;
}

// Set the seed for the pseudo-random number generator.
void set_seed(u64 a, u64 b)
{
    random_seed[0] = a;
    random_seed[1] = b;
    // The first few iterations generate poor results,
    // so run the generator a few times to avoid this.
    for (int i = 0; i < 64; ++i) random_u64();
}

// Get a random f32 between 0.0 and 1.0.
f32 random_f32()
{
    return (f32)random_u64() / (f32)UINT64_MAX;
}

// Get a random f32 between low and high.
f32 random_f32_range(f32 low, f32 high)
{
    f32 d = fabsf(high - low);
    return random_f32() * d + low;
}

// Get a random int between low and high, inclusive.
int random_int_range(int low, int high)
{
    int d = abs(high - low) + 1;
    return random_f32() * d + low;
}

// Get a random boolean.
bool chance(f32 chance_to_be_true)
{
    return random_f32() <= chance_to_be_true;
}

//
// Error reporting.
//

// Print a message and exit the program.
// Also attempts to display a pop-up error message.
void panic_exit(char * message, ...)
{
    va_list args;
    va_start(args, message);
    char buffer[256];
    vsnprintf(buffer, 256, message, args);
    va_end(args);
    puts(buffer);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", buffer, NULL);
    exit(1);
}

// Same as above but does not exit the program.
void issue_warning(char * message, ...)
{
    va_list args;
    va_start(args, message);
    char buffer[256];
    vsnprintf(buffer, 256, message, args);
    va_end(args);
    puts(buffer);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Warning!", buffer, NULL);
}

//
// Generic type printer. (Requires C11)
//

#define GFMT(x) _Generic((x),           \
    bool:                     "%d\n",   \
    char:                     "%c\n",   \
    signed char:              "%hhd\n", \
    unsigned char:            "%hhu\n", \
    signed short:             "%hd\n",  \
    unsigned short:           "%hu\n",  \
    signed int:               "%d\n",   \
    unsigned int:             "%u\n",   \
    long int:                 "%ld\n",  \
    unsigned long int:        "%lu\n",  \
    long long int:            "%lld\n", \
    unsigned long long int:   "%llu\n", \
    float:                    "%f\n",   \
    double:                   "%f\n",   \
    long double:              "%Lf\n",  \
    char *:                   "%s\n",   \
    signed char *:            "%p\n",   \
    unsigned char *:          "%p\n",   \
    signed short *:           "%p\n",   \
    unsigned short *:         "%p\n",   \
    signed int *:             "%p\n",   \
    unsigned int *:           "%p\n",   \
    long int *:               "%p\n",   \
    unsigned long int *:      "%p\n",   \
    long long int *:          "%p\n",   \
    unsigned long long int *: "%p\n",   \
    float *:                  "%p\n",   \
    double *:                 "%p\n",   \
    long double *:            "%p\n",   \
    void *:                   "%p\n",   \
    default:                  "")
#define put(x) printf(GFMT(x),x)

~~~

#### memory.c
~~~{.c .numberLines}
//
// memory.c
//
// This file contains:
//     - Memory pool allocators.
//     - Memory utility functions.
//

//
// General memory related functions.
//

static inline u64 megabytes(u64 count)
{
    return count * 1024 * 1024;
}

// NOTE: There is evidence to suggest that alignment may be unnecessary on more
// recent x86 hardware: http://www.agner.org/optimize/blog/read.php?i=142

// All memory allocations align to the platform's maximum primitive size.
#define MAX_ALIGNMENT_BYTES (sizeof(max_align_t))

// Round byte_count up to the next multiple of the desired alignment.
static inline u64 align_byte_count(u64 byte_count)
{
    byte_count += (MAX_ALIGNMENT_BYTES - 1);
    byte_count &= ~(MAX_ALIGNMENT_BYTES - 1);
    return byte_count;
}

void set_memory(void * memory, u64 byte_count, u8 value)
{
    while (byte_count > 0)
    {
        *( u8 *)memory++ = value;
        --byte_count;
    }
}

// Perform a copy of the data at src to dest of byte_count bytes.
void copy_memory(void * src, void * dest, u64 byte_count)
{
    if (byte_count & (~(u64)7))
    {
        // Copy 8 bytes at a time.
        u64 * src64 = src;
        u64 * dest64 = dest;
        u64 count = byte_count / 8;
        for (u64 i = 0; i < count; ++i) *dest64++ = *src64++;
    }
    else if (byte_count & (~(u64)3))
    {
        // Copy 4 bytes at a time.
        u32 * src32 = src;
        u32 * dest32 = dest;
        u64 count = byte_count / 4;
        for (u64 i = 0; i < count; ++i) *dest32++ = *src32++;
    }
    else if (byte_count & (~(u64)1))
    {
        // Copy 2 bytes at a time.
        u16 * src16 = src;
        u16 * dest16 = dest;
        u64 count = byte_count / 2;
        for (u64 i = 0; i < count; ++i) *dest16++ = *src16++;
    }
    else
    {
        // Fall back to byte-by-byte copy.
        u8 * src8 = src;
        u8 * dest8 = dest;
        while (byte_count-- > 0) *dest8++ = *src8++;
    }
}

// Returns true if the given memory is byte-for-byte equivalent.
bool equal(void * a, void * b, u64 byte_count)
{
    if (a == b) return true;
    while (byte_count && *(u8 *)a++ == *(u8 *)b++) --byte_count;
    return byte_count == 0;
}

//
// Memory Pools.
//
// Memory pools are simple stack-like structures that have a fixed amount of
// memory under their control. When an allocation is made from a pool, it
// selects some memory from the top of its pool and returns that pointer.
//
// All allocations are guaranteed to be aligned to the multiple for the largest
// type available on the machine.
//

typedef struct
{
    u8 * memory;
    u64 bytes_available;
    u64 bytes_filled;
    u64 byte_count_of_last_alloc;
} Memory_Pool;

#define PERSIST_POOL 0
#define SCENE_POOL 1
#define FRAME_POOL 2

Memory_Pool memory_pools[] = {
    [PERSIST_POOL] = { NULL, 0, 0, 0 },
    [SCENE_POOL]   = { NULL, 0, 0, 0 },
    [FRAME_POOL]   = { NULL, 0, 0, 0 },
};

// Allocates a number of bytes from the given memory pool.
// Returns NULL if the allocation was unsuccessful.
void * pool_alloc(int pool_index, u64 byte_count)
{
    void * result = NULL;
    byte_count = align_byte_count(byte_count);
    Memory_Pool * pool = &memory_pools[pool_index];
    if (pool->bytes_filled + byte_count <= pool->bytes_available)
    {
        result = pool->memory + pool->bytes_filled;
        pool->bytes_filled += byte_count;
        pool->byte_count_of_last_alloc = byte_count;
    }
    return result;
}

// Deallocate the last item that was allocated.
// Useful for very temporary usage of memory.
void pool_unalloc(int pool_index)
{
    Memory_Pool * pool = &memory_pools[pool_index];
    pool->bytes_filled -= pool->byte_count_of_last_alloc;
    pool->byte_count_of_last_alloc = 0;
}

void flush_pool(int pool_index)
{
    memory_pools[pool_index].bytes_filled = 0;
    memory_pools[pool_index].byte_count_of_last_alloc = 0;
}

// Returns true if the pointer points to memory inside the given pool.
bool was_allocated_by_pool(int pool_index, void * pointer)
{
    u64 p = (u64)pointer;
    u64 low = (u64)memory_pools[pool_index].memory;
    u64 high = (u64)((u8 *)low + memory_pools[pool_index].bytes_available);
    return p >= low && p < high;
}

// Create an allocated copy of any memory.
void * clone_memory(int pool_index, void * src, u64 byte_count)
{
    void * memory = pool_alloc(pool_index, byte_count);
    if (memory) copy_memory(src, memory, byte_count);
    return memory;
}

// Allocates the memory pools.
// Returns false if the allocation did not succeed.
// persist_byte_count is ignored if POOL_STATIC_ALLOCATE is defined.
bool init_memory_pools(u64 persist_byte_count,
    u64 scene_byte_count, u64 frame_byte_count)
{
#ifdef POOL_STATIC_ALLOCATE
    persist_byte_count = POOL_STATIC_PERSIST_BYTE_COUNT;
#endif

    // Scene and frame pools must fit inside the persistent pool.
    if (scene_byte_count + frame_byte_count > persist_byte_count) return false;

#ifdef POOL_STATIC_ALLOCATE
    // If the storage is not too large, it is totally safe to statically allocate
    // this memory. 2GB is a reasonable maximum to assume.
    // https://software.intel.com/en-us/articles/memory-limits-applications-windows
    static u8 static_memory[POOL_STATIC_PERSIST_BYTE_COUNT];
    void * memory = static_memory;
#else
    // Allocate the memory at run time.
    // mmap is preferred to malloc as it will not maintain internal storage,
    // all memory allocated by mmap will be under our control.
    void * memory = mmap(0,
        persist_byte_count,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        0, 0);
#endif

    if (!memory) return false;

    // Access each page of the allocated memory to ensure the pages are
    // accessible. 4096 is a common memory page size.
    u8 * bytes = memory;
    for (int byte_index = 0; byte_index < persist_byte_count; byte_index += 4096)
    {
        bytes[byte_index] = 0;
    }

    // The persistent pool handles this allocated memory.
    memory_pools[PERSIST_POOL].memory = memory;
    memory_pools[PERSIST_POOL].bytes_available = persist_byte_count;
    memory_pools[PERSIST_POOL].bytes_filled = 0;

    // The scene and frame pools are allocated from the persistent pool.
    memory_pools[SCENE_POOL].memory = pool_alloc(PERSIST_POOL, scene_byte_count);
    memory_pools[SCENE_POOL].bytes_available = scene_byte_count;
    memory_pools[SCENE_POOL].bytes_filled = 0;

    memory_pools[FRAME_POOL].memory = pool_alloc(PERSIST_POOL, frame_byte_count);
    memory_pools[FRAME_POOL].bytes_available = frame_byte_count;
    memory_pools[FRAME_POOL].bytes_filled = 0;

    return true;
}

//
// DEBUG:
//

void print_memory_stats()
{
    printf("Memory Pool Stats:\n");

    printf("Persist: %8llu / %8llu (%02.0f%%), %8llu\n",
        memory_pools[PERSIST_POOL].bytes_filled,
        memory_pools[PERSIST_POOL].bytes_available,
        memory_pools[PERSIST_POOL].bytes_filled /
            (f32)memory_pools[PERSIST_POOL].bytes_available * 100,
        memory_pools[PERSIST_POOL].byte_count_of_last_alloc);

    printf("Scene:   %8llu / %8llu (%02.0f%%), %8llu\n",
        memory_pools[SCENE_POOL].bytes_filled,
        memory_pools[SCENE_POOL].bytes_available,
        memory_pools[SCENE_POOL].bytes_filled /
            (f32)memory_pools[SCENE_POOL].bytes_available * 100,
        memory_pools[SCENE_POOL].byte_count_of_last_alloc);

    printf("Frame:   %8llu / %8llu (%02.0f%%), %8llu\n",
        memory_pools[FRAME_POOL].bytes_filled,
        memory_pools[FRAME_POOL].bytes_available,
        memory_pools[FRAME_POOL].bytes_filled /
            (f32)memory_pools[FRAME_POOL].bytes_available * 100,
        memory_pools[FRAME_POOL].byte_count_of_last_alloc);
}

~~~

#### graphics.c
~~~{.c .numberLines}
//
// graphics.c
//
// This file contains:
//     - Pixel manipulation utilities.
//     - Graphical primitive rendering.
//     - Bitmap rendering.
//     - Animated bitmap handling and rendering.
//     - Bitmap font rendering.
//

// The fixed resolution of the internal pixel buffer.
#define WIDTH 320
#define HEIGHT 240

// Points to the internal pixel buffer.
u32 * pixels;

// Pack an RGBA pixel from its components.
static inline u32 rgba(u8 r, u8 g, u8 b, u8 a)
{
    return (r << 24u) | (g << 16u) | (b << 8u) | a;
}

// Access individual components of an RGBA pixel.
static inline u32 get_red(u32 colour)   { return (colour & 0xff000000) >> 24; }
static inline u32 get_blue(u32 colour)  { return (colour & 0x00ff0000) >> 16; }
static inline u32 get_green(u32 colour) { return (colour & 0x0000ff00) >> 8; }
static inline u32 get_alpha(u32 colour) { return (colour & 0x000000ff) >> 0; }

// Set every pixel of the internal buffer to a colour.
void clear(u32 colour)
{
    for (int y = 0; y < HEIGHT; ++y)
    {
        for (int x = 0; x < WIDTH; ++x)
        {
            pixels[x + y * WIDTH] = colour;
        }
    }
}

// Draws noise over the entire screen.
void draw_noise(float intensity)
{
    for (int y = 0; y < HEIGHT; ++y)
    {
        for (int x = 0; x < WIDTH; ++x)
        {
            int r = random_int_range(0, intensity * 255);
            pixels[x + y * WIDTH] = rgba(r, r, r, 255);
        }
    }
}

// Returns false if the given coordinates are off screen.
static inline bool set_pixel(int x, int y, u32 colour)
{
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
    {
        pixels[x + y * WIDTH] = colour;
        return true;
    }
    return false;
}

// Draw a line using Bresenham's line algorithm.
void draw_line(int ax, int ay, int bx, int by, u32 colour)
{
    int delta_x = abs(bx - ax);
    int delta_y = abs(by - ay);
    int step_x  = ax < bx ? 1 : -1;
    int step_y  = ay < by ? 1 : -1;
    int error   = (delta_x > delta_y ? delta_x : -delta_y) / 2;

    // Stop drawing if pixel is off screen or we have reached the end of the line.
    while (set_pixel(ax, ay, colour) && !(ax == bx && ay == by))
    {
        int e = error;
        if (e > -delta_x) error -= delta_y, ax += step_x;
        if (e <  delta_y) error += delta_x, ay += step_y;
    }
}

//
// Image.
//
// An Image is just a rectangular chunk of pixels with a width and height.
//

typedef struct
{
    u32 * pixels;
    int width;
    int height;
}
Image;

// Draw a bitmap image to the internal buffer.
void draw_image(Image image, int x, int y)
{
    int max_x = min(x + image.width, WIDTH);
    int max_y = min(y + image.height, HEIGHT);
    for (int sy = y, iy = 0; sy < max_y; ++sy, ++iy)
    {
        for (int sx = x, ix = 0; sx < max_x; ++sx, ++ix)
        {
            u32 p = image.pixels[ix + iy * image.width];
            if (get_alpha(p) != 0)
            {
                set_pixel(sx, sy, p);
            }
        }
    }
}

//
// Animated Images.
//
// Animated images are an extension of normal images. They are a collection
// of frames, stored vertically in the same image. Some simple timing is used
// to determine which frame should be displayed, or individual frames can be
// displayed manually. By default, animations loop.
//

typedef struct
{
    u32 * pixels;
    int width;
    int height;
    int frame_count;
    int frame_duration_ms;
    int start_time_ms;
}
Animated_Image;

// Draw an animated image to the internal buffer. This function expects that
// the object passed in has appropriate numbers in each of its fields.
void draw_animated_image(Animated_Image animated_image, int x, int y)
{
    int time_passed = SDL_GetTicks() - animated_image.start_time_ms;
    if (animated_image.frame_duration_ms == 0) return;
    int frames_passed = time_passed / animated_image.frame_duration_ms;
    int current_frame = frames_passed % animated_image.frame_count;
    int pixels_per_frame = animated_image.width * animated_image.height;
    int pixel_offset_to_current_frame = pixels_per_frame * current_frame;
    Image frame =
    {
        .pixels = animated_image.pixels + pixel_offset_to_current_frame,
        .width  = animated_image.width,
        .height = animated_image.height,
    };
    draw_image(frame, x, y);
}

// Draw a single frame of an animation to the internal buffer.
void draw_animated_image_frame(Animated_Image animated_image,
    int animation_frame, int x, int y)
{
    int pixels_per_frame = animated_image.width * animated_image.height;
    int pixel_offset_to_current_frame = pixels_per_frame * animation_frame;
    Image frame =
    {
        .pixels = animated_image.pixels + pixel_offset_to_current_frame,
        .width  = animated_image.width,
        .height = animated_image.height,
    };
    draw_image(frame, x, y);
}

// Draw a selected range of frames of animation, instead of all frames.
void draw_animated_image_frames(Animated_Image animated_image,
    int start_frame, int end_frame, int x, int y)
{
    int time_passed = SDL_GetTicks() - animated_image.start_time_ms;
    int frames_passed = time_passed / animated_image.frame_duration_ms;
    int frame_count = (end_frame - start_frame) + 1;
    int current_frame = start_frame + (frames_passed % frame_count);
    int pixels_per_frame = animated_image.width * animated_image.height;
    int pixel_offset_to_current_frame = pixels_per_frame * current_frame;
    Image frame =
    {
        .pixels = animated_image.pixels + pixel_offset_to_current_frame,
        .width  = animated_image.width,
        .height = animated_image.height,
    };
    draw_image(frame, x, y);
}

// Same as above but don't loop, stop and display the final frame once it is complete.
bool draw_animated_image_frames_and_wait(Animated_Image animated_image,
    int start_frame, int end_frame, int x, int y)
{
    bool waiting = false;
    int time_passed = SDL_GetTicks() - animated_image.start_time_ms;
    int frames_passed = time_passed / animated_image.frame_duration_ms;
    int frame_count = (end_frame - start_frame) + 1;
    int current_frame = start_frame + (frames_passed % frame_count);
    if (start_frame + frames_passed > end_frame)
    {
        current_frame = end_frame;
        waiting = true;
    }
    int pixels_per_frame = animated_image.width * animated_image.height;
    int pixel_offset_to_current_frame = pixels_per_frame * current_frame;
    Image frame =
    {
        .pixels = animated_image.pixels + pixel_offset_to_current_frame,
        .width  = animated_image.width,
        .height = animated_image.height,
    };
    draw_image(frame, x, y);
    return waiting;
}

//
// Text rendering.
//
// This text rendering system uses mono-space bitmap fonts. The font data is
// loaded as an image with each drawable ASCII character in it, in ascending
// order from left to right (starting with the ' ' character).
//

typedef struct
{
    u32 * pixels;
    int char_width;
    int char_height;
}
Font;

void draw_text(Font font, int x, int y, u32 colour, char * text, ...)
{
#define TEXT_MAX 64
    char formatted_text[TEXT_MAX];
    va_list args;
    va_start(args, text);
    int char_count = vsnprintf(formatted_text, TEXT_MAX, text, args);
    char_count = min(char_count, TEXT_MAX);
    va_end(args);
    // 95 is the number of drawable characters including the single space.
    int total_width = 95 * font.char_width;
    int x_offset = 0;
    for (int c = 0; c < char_count; ++c)
    {
        if (formatted_text[c] >= ' ' && formatted_text[c] <= '~')
        {
            int text_start_x = font.char_width * (formatted_text[c] - ' ');
            int max_x = min(x + font.char_width, WIDTH);
            int max_y = min(y + font.char_height, HEIGHT);
            for (int sy = y, iy = 0; sy < max_y; ++sy, ++iy)
            {
                for (int sx = x, ix = text_start_x; sx < max_x; ++sx, ++ix)
                {
                    if (font.pixels[ix + iy * total_width])
                    {
                        set_pixel(sx + x_offset, sy, colour);
                    }
                }
            }
            x_offset += font.char_width;
        }
    }
}

~~~

#### audio.c
~~~{.c .numberLines}
//
// audio.c
//
// This file contains:
//     - Audio Mixer.
//

// The index of the audio device, assigned at program initialisation.
u32 audio_device = 0;

//
// Sound.
//
// A single sound is a block of mono single-precision floating point PCM samples.
//

typedef struct
{
    f32 * samples;
    int sample_count;
}
Sound;

//
// Audio Mixer.
//
// The mixer consists of a number of channels, each of which can hold a sound
// along with a set of parameters to control it's playback.
//

typedef struct
{
    f32 * samples;    // The audio data itself.
    int sample_count;   // Number of samples in the data.
    int sample_index;   // Index of the last sample written.
    f32 left_gain;    // How loud to play the sound in the left channel.
    f32 right_gain;   // Same for the right channel.
    bool loop;          // If the sound should repeat.
    bool playing;       // If the sound is playing right now or not.
}
Mixer_Channel;

typedef struct
{
    Mixer_Channel * channels;
    int channel_count;
    f32 gain;
}
Mixer;

Mixer mixer;

// Initialise the audio mixer.
Mixer create_mixer(int pool_index, int channel_count, f32 gain)
{
    Mixer mixer = {};
    mixer.channels = pool_alloc(pool_index,
        channel_count * sizeof(Mixer_Channel));
    if (mixer.channels)
    {
        mixer.channel_count = channel_count;
        mixer.gain = gain;
    }
    return mixer;
}

// The main audio mixing function. This will likely be called on a separate thread,
// so some care should be taken when considering input and output.
void mix_audio(Mixer * mixer, void * stream, int samples_requested)
{
    f32 * samples = stream;

    // Zero the entire buffer first.
    for (int sample_index = 0;
        sample_index < samples_requested;
        ++sample_index)
    {
        samples[sample_index] = 0.0f;
    }

    // Mix all of the data from a channel into the buffer,
    // then move on to the next channel.
    for (int channel_index = 0;
        channel_index < mixer->channel_count;
        ++channel_index)
    {
        Mixer_Channel * channel = &mixer->channels[channel_index];
        if (channel->samples && channel->playing)
        {
            for (int sample_index = 0;
                 sample_index < samples_requested &&
                 channel->sample_index < channel->sample_count;
                 ++sample_index)
            {
                // Load a mono sample from the channel.
                f32 new_left  = channel->samples[channel->sample_index];
                f32 new_right = channel->samples[channel->sample_index];

                // Apply the left and right channel gains.
                new_left  *= channel->left_gain;
                new_left  *= mixer->gain;
                new_right *= channel->right_gain;
                new_right *= mixer->gain;

                // Mix the adjusted sample into the stereo output buffer.
                samples[sample_index] += new_left;
                ++sample_index;
                samples[sample_index] += new_right;

                // Move to the next sample in the channel.
                channel->sample_index += 1;
            }

            // If we have read all of the samples, end the sound,
            // or restart it if it is set to loop.
            if (channel->sample_index >= channel->sample_count)
            {
                if (channel->loop)
                {
                    channel->sample_index = 0;
                }
                else
                {
                    *channel = (Mixer_Channel){};
                }
            }
        }
    }
}

//
// Playback control.
//
// Each of these functions should be called only when a synchronisation lock
// has been attained (assuming the audio is being handled on another thread).
//

// Immediately start playing a sound.
// Returns the index of the channel that holds the sound,
// or -1 if no channel was available.
int play_sound(Mixer * mixer, Sound sound,
    f32 left_gain, f32 right_gain, int loop)
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

// Load a channel with a sound and set its parameters.
// Returns the index of the channel that holds the sound,
// or -1 if no channel was available.
int queue_sound(Mixer * mixer, Sound sound,
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
            mixer->channels[i].playing      = false;
            SDL_UnlockAudioDevice(audio_device);
            return i;
        }
    }
    return -1;
}

// Tell a loaded channel to start playing.
// Returns true if the given channel was successfully told to play.
bool play_channel(Mixer * mixer, int channel_index)
{
    if (channel_index >= 0 && channel_index <= mixer->channel_count &&
        mixer->channels[channel_index].samples)
    {
        SDL_LockAudioDevice(audio_device);
        mixer->channels[channel_index].playing = true;
        SDL_UnlockAudioDevice(audio_device);
        return true;
    }
    return false;
}

// Tell a loaded channel to stop playing (but stay loaded).
// Returns true if the given channel was successfully told to pause.
bool pause_channel(Mixer * mixer, int channel_index)
{
    if (channel_index >= 0 &&
        channel_index <= mixer->channel_count &&
        mixer->channels[channel_index].samples)
    {
        SDL_LockAudioDevice(audio_device);
        mixer->channels[channel_index].playing = false;
        SDL_UnlockAudioDevice(audio_device);
        return true;
    }
    return false;
}

// Stop all instances of a sound that are playing.
bool stop_sound(Mixer * mixer, Sound sound)
{
    for (int i = 0; i < mixer->channel_count; ++i)
    {
        if (mixer->channels[i].samples == sound.samples)
        {
            mixer->channels[i] = (Mixer_Channel){};
            return true;
        }
    }
    return false;
}

// Returns true if the given sound is playing in a channel.
bool sound_is_playing(Mixer * mixer, Sound sound)
{
    for (int i = 0; i < mixer->channel_count; ++i)
    {
        if (mixer->channels[i].samples == sound.samples)
        {
            return true;
        }
    }
    return false;
}

~~~

#### assets.c
~~~{.c .numberLines}
//
// assets.c
//
// This file contains:
//     - Image loader and writer.
//     - Sound loader and writer.
//     - Asset loading and storing.
//

//
// Image files.
//
// The Portable Arbitrary Map image file format is used. It is one of the simplest
// image formats that allows the RGBA pixel format. It stores the pixels
// uncompressed, with a simple header.
//
// An example of the .pam header:
// P7                     <-- Magic number.
// WIDTH 256              <-- Image width (in pixels).
// HEIGHT 256             <-- Image height (in pixels).
// DEPTH 4                <-- Number of channels per pixel.
// MAXVAL 255             <-- Highest value a channel can be.
// TUPLTYPE RGB_ALPHA     <-- The pixel format. (Can also be grayscale, etc.)
// ENDHDR                 <-- Marker for the end of the header.
//

// Reorder the channels of a four channel pixel.
void agbr_to_rgba(void * pixels, int pixel_count)
{
    for (int pixel_index = 0; pixel_index < pixel_count; ++pixel_index)
    {
        u32 p = ((u32 *)pixels)[pixel_index];
        ((u32 *)pixels)[pixel_index] = rgba(
            // get_alpha(p), get_blue(p), get_green(p), get_red(p));
            get_alpha(p), get_green(p), get_blue(p), get_red(p));
    }
}

// Load an RGBA format .pam file into an Image.
// Returns a zero'd Image if unsuccessful.
Image read_image_file(int pool_index, char * file_name)
{
    FILE * file = fopen(file_name, "r");
    if (!file) return (Image){};
    int width = 0, height = 0;
    fscanf(file, "P7\n"
        "WIDTH %d\n"
        "HEIGHT %d\n"
        "DEPTH 4\n"
        "MAXVAL 255\n"
        "TUPLTYPE RGB_ALPHA\n"
        "ENDHDR\n",
        &width, &height);
    if (width && height)
    {
        int pixel_count = width * height;
        u32 * pixels = pool_alloc(pool_index, width * height * sizeof(u32));
        int pixels_read = fread(pixels, sizeof(u32), pixel_count, file);
        fclose(file);
        if (pixels_read == pixel_count)
        {
            agbr_to_rgba(pixels, pixel_count);
            return (Image){ pixels, width, height };
        }
    }
    return (Image){};
}

// Writes an Image to a .pam file.
// Returns true if the entire Image was successfully written.
bool write_image_file(Image image, char * file_name)
{
    FILE * file = fopen(file_name, "w");
    if (!file) return false;
    fprintf(file,
        "P7\nWIDTH %d\nHEIGHT %d\nDEPTH 4\n"
        "MAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n",
        image.width, image.height);
    int byte_count = image.width * image.height * sizeof(image.pixels[0]);
    int bytes_written = fwrite(image.pixels, 1, byte_count, file);
    fclose(file);
    return byte_count == bytes_written;
}

//
// Audio files.
//
// A simple custom file format is used to store audio data. It is derived from
// the .pam format described above.
//
// An example of the file header:
// SND                    <-- Magic number.
// SAMPLE_COUNT 48000     <-- The number of samples in the file.
// ENDHDR                 <-- Marker for the end of the header.
//

// Reads a mono f32 format audio file.
// Returns a zero'd Sound if unsuccessful.
Sound read_sound_file(int pool_index, char * file_name)
{
    FILE * file = fopen(file_name, "r");
    if (!file) return (Sound){};
    int sample_count = 0;
    fscanf(file, "SND\nSAMPLE_COUNT %d\nENDHDR\n", &sample_count);
    if (sample_count)
    {
        f32 * samples = pool_alloc(pool_index, sample_count * sizeof(f32));
        int samples_read = fread(samples, sizeof(f32), sample_count, file);
        fclose(file);
        if (samples_read == sample_count)
        {
            return (Sound){ samples, sample_count };
        }
    }
    return (Sound){};
}

// Writes a sound into a file.
// Returns true if the entire Sound was successfully written.
bool write_sound_file(Sound sound, char * file_name)
{
    FILE * file = fopen(file_name, "w");
    if (!file) return false;
    fprintf(file, "SND\nSAMPLE_COUNT %d\nENDHDR\n", sound.sample_count);
    int byte_count = sound.sample_count * sizeof(sound.samples[0]);
    int bytes_written = fwrite(sound.samples, 1, byte_count, file);
    fclose(file);
    return byte_count == bytes_written;
}

// Read raw (headerless) mono f32 pcm data from a file.
Sound read_raw_sound(int pool, char * file_name)
{
    Sound s = {};
    FILE * file = fopen(file_name, "r");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        int byte_count = ftell(file);
        fseek(file, 0, SEEK_SET);
        s.samples = pool_alloc(pool, byte_count);
        int bytes_read = 0;
        bytes_read = fread(s.samples, 1, byte_count, file);
        fclose(file);
        SDL_assert(byte_count == bytes_read);
        s.sample_count = bytes_read / sizeof(f32);
    }
    return s;
}

//
// Loading assets.
//
// All assets are loaded into a single struct, with references to them taken
// when needed. load_assets should be called at launch with a suitable path.
//

struct
{
    Animated_Image button_animation;
    Animated_Image heart_animation;
    Animated_Image left_lung_animation;
    Animated_Image right_lung_animation;
    Animated_Image digestion_animation;
    Font main_font;
    Font scream_font;
    Image heart_icon;
    Image morse_background;
    Image relaxed_skeleton;
    Sound wood_block_sound;
    Sound yay_sound;
    Sound shaker_sound;
    Sound tap_sound;
    Sound clock_sound;
    Sound brown_sound;
}
assets;

// Load all assets, given a path relative to the location of the executable
// (or app bundle).
bool load_assets(char * assets_dir)
{
    char * full_dir = pool_alloc(FRAME_POOL, 512);
    char * base_path = SDL_GetBasePath();
    snprintf(full_dir, 512, "%s%s", base_path, assets_dir ? assets_dir : "");
    SDL_free(base_path);
    chdir(full_dir);

    assets.relaxed_skeleton = read_image_file(PERSIST_POOL, "relaxed_skeleton.pam");
    if (!assets.relaxed_skeleton.pixels) return false;

    assets.heart_icon = read_image_file(PERSIST_POOL, "heart_icon.pam");
    if (!assets.heart_icon.pixels) return false;

    {
        Image main_font_image = read_image_file(PERSIST_POOL, "font.pam");
        if (!main_font_image.pixels) return false;
        assets.main_font = (Font)
        {
            .pixels = main_font_image.pixels,
            .char_width = 6,
            .char_height = 12,
        };
    }

    {
        Image scream_font_image = read_image_file(PERSIST_POOL, "scream.pam");
        if (!scream_font_image.pixels) return false;
        assets.scream_font = (Font)
        {
            .pixels = scream_font_image.pixels,
            .char_width = 9,
            .char_height = 8,
        };
    }

    {
        Image button_image = read_image_file(PERSIST_POOL, "button.pam");
        assets.button_animation = (Animated_Image)
        {
            .pixels = button_image.pixels,
            .width = button_image.width,
            .height = button_image.height / 2,
            .frame_count = 2,
            .frame_duration_ms = 10,
        };
    }

    {
        Image heart_animation_image = read_image_file(PERSIST_POOL, "heart.pam");
        if (!heart_animation_image.pixels) return false;
        assets.heart_animation = (Animated_Image)
        {
            .pixels = heart_animation_image.pixels,
            .width = 320,
            .height = 200,
            .frame_count = 7,
        };
    }

    {
        Image left_lung_animation_image = read_image_file(PERSIST_POOL, "left_lung.pam");
        if (!left_lung_animation_image.pixels) return false;
        assets.left_lung_animation = (Animated_Image)
        {
            .pixels = left_lung_animation_image.pixels,
            .width = 85,
            .height = 167,
            .frame_count = 8,
        };
    }

    {
        Image right_lung_animation_image = read_image_file(PERSIST_POOL, "right_lung.pam");
        if (!right_lung_animation_image.pixels) return false;
        assets.right_lung_animation = (Animated_Image)
        {
            .pixels = right_lung_animation_image.pixels,
            .width = 90,
            .height = 167,
            .frame_count = 8,
        };
    }

    {
        Image digestion_animation_image = read_image_file(PERSIST_POOL, "digestion.pam");
        if (!digestion_animation_image.pixels) return false;
        assets.digestion_animation = (Animated_Image)
        {
            .pixels = digestion_animation_image.pixels,
            .width = 85,
            .height = 200,
            .frame_count = 7,
        };
    }

    assets.shaker_sound = read_raw_sound(PERSIST_POOL, "shaker.f32");
    if (!assets.shaker_sound.samples) return false;

    assets.wood_block_sound = read_raw_sound(PERSIST_POOL, "woodblock.f32");
    if (!assets.wood_block_sound.samples) return false;

    assets.tap_sound = read_raw_sound(PERSIST_POOL, "tap.f32");
    if (!assets.tap_sound.samples) return false;

    assets.yay_sound = read_raw_sound(PERSIST_POOL, "yay.f32");
    if (!assets.yay_sound.samples) return false;

    assets.clock_sound = read_raw_sound(PERSIST_POOL, "clock.f32");
    if (!assets.clock_sound.samples) return false;

    assets.brown_sound = read_raw_sound(PERSIST_POOL, "brown.f32");
    if (!assets.brown_sound.samples) return false;

    return true;
}

~~~

#### scene.c
~~~{.c .numberLines}
//
// scene.c
//
// This file contains:
//     - Scene handling.
//     - Scene definitions (e.g. the mini-games).
//

//
// Scene.
//
// A scene is a set of functions and a struct of state variables that can be
// swapped out at will. The start function is called when the scene is entered,
// the frame function is called when the frame needs to be redrawn, and the
// input function is called when a player presses a button.
//

typedef void (* Frame_Func)(void * state, f32 delta_time);
typedef void (* Start_Func)(void * state);
typedef void (* Input_Func)(void * state, int player, bool pressed, u32 time_stamp_ms);

typedef struct
{
    Start_Func start;
    Frame_Func frame;
    Input_Func input;
    void * state;
}
Scene;

// TODO: Where should the scenes live?
Scene current_scene;
extern Scene heart_scene;

// Change the current scene.
// Will call the start function for that scene.
// Returns true if successful.
bool set_scene(Scene scene)
{
    // Clear scene and frame memory pools.
    flush_pool(SCENE_POOL);
    flush_pool(FRAME_POOL);
    // Set function pointers.
    if (scene.start && scene.frame && scene.input && scene.state)
    {
        current_scene = scene;
        // Call the start function for the new scene.
        current_scene.start(current_scene.state);
        return true;
    }
    return false;
}

//
// Blank scene.
//
// Display a blank screen of a given colour, for a given amount of time.
// Can be used for transitions between scenes.
//

typedef struct
{
    u32 end_time;
    u32 colour;
    f32 time_in_seconds;
    Scene * next_scene;
    Sound end_sound;
}
Blank_State;

Blank_State blank_state;

void blank_frame(void * state, f32 delta_time)
{
    Blank_State * s = state;
    if (s->end_time < SDL_GetTicks())
    {
        if (s->end_sound.samples)
        {
            play_sound(&mixer, s->end_sound, 1.0, 1.0, false);
        }
        set_scene(*s->next_scene);
    }
    clear(s->colour);
}

// Stub functions, as nothing needs to be done for this scene.
void blank_start(void * state)
{
    Blank_State * s = state;
    s->end_time = SDL_GetTicks() + (1000 * s->time_in_seconds);
}

void blank_input(void * state, int player, bool pressed, u32 time_stamp_ms) {}

Scene blank_scene =
{
    .start = blank_start,
    .frame = blank_frame,
    .input = blank_input,
    .state = &blank_state,
};

void prepare_blank_cut(f32 time_in_seconds, u32 colour,
    Scene * next_scene, Sound * end_sound)
{
    blank_state = (Blank_State)
    {
        .time_in_seconds = time_in_seconds,
        .next_scene = next_scene,
        .colour = colour,
        .end_sound = end_sound ? *end_sound : (Sound){},
    };

    set_scene(blank_scene);
}

void blank_cut(f32 time_in_seconds, u32 colour,
    Scene * next_scene, Sound * end_sound)
{
    prepare_blank_cut(time_in_seconds, colour, next_scene, end_sound);
    set_scene(blank_scene);
}

//
// Accuracy interface.
//
// A tutorial interface which helps players improve their accuracy.
//
void draw_accuracy_interface(f32 accuracy,
    f32 range, f32 bpm,
    bool draw_left_arrow, bool draw_right_arrow,
    bool left_state, bool right_state)
{
    f32 yellow_range = range * 5.0;
    f32 red_range = range * 10.0f;
    f32 scale = 100.0 / red_range;

    int y = 10;
    draw_line(WIDTH / 2 - red_range * scale, HEIGHT - y,
        WIDTH / 2 + red_range * scale, HEIGHT - y,
        0xff0000ff);
    draw_line(WIDTH / 2 - yellow_range * scale, HEIGHT - y,
        WIDTH / 2 + yellow_range * scale, HEIGHT - y,
        0xffff00ff);
    draw_line(WIDTH / 2 - range * scale, HEIGHT - y,
        WIDTH / 2 + range * scale, HEIGHT - y,
        0x00ff00ff);
    draw_line(WIDTH / 2 - range * scale, HEIGHT - (y - 1),
        WIDTH / 2 - range * scale, HEIGHT - (y + 1),
        0x00ff00ff);
    draw_line(WIDTH / 2 + range * scale, HEIGHT - (y - 1),
        WIDTH / 2 + range * scale, HEIGHT - (y + 1),
        0x00ff00ff);

    draw_line(WIDTH / 2 + accuracy * scale, HEIGHT - (y - 1),
        WIDTH / 2 + accuracy * scale, HEIGHT - (y + 1),
        ~0);
    draw_line(WIDTH / 2 + accuracy * scale - 1, HEIGHT - (y - 1),
        WIDTH / 2 + accuracy * scale - 1, HEIGHT - (y + 1),
        ~0);
    draw_line(WIDTH / 2 + accuracy * scale + 1, HEIGHT - (y - 1),
        WIDTH / 2 + accuracy * scale + 1, HEIGHT - (y + 1),
        ~0);

    draw_text(assets.main_font,
        WIDTH / 2 - red_range * scale - 30,
        HEIGHT - (y + 6),
        ~0,
        "slow");
    draw_text(assets.main_font,
        WIDTH / 2 + red_range * scale + 5,
        HEIGHT - (y + 6),
        ~0,
        "fast");

    y = 80 + sinf((M_PI*2.0) * SDL_GetTicks() * 0.001 * (bpm / 60.0)) * 5;
    if (draw_left_arrow)
    {
        draw_line(44, y,      44,     y + 10, ~0);
        draw_line(44, y + 10, 44 - 5, y +  5, ~0);
        draw_line(44, y + 10, 44 + 5, y +  5, ~0);
    }

    if (draw_right_arrow)
    {
        draw_line(274, y,      274,     y + 10, ~0);
        draw_line(274, y + 10, 274 - 5, y +  5, ~0);
        draw_line(274, y + 10, 274 + 5, y +  5, ~0);
    }

    draw_animated_image_frame(assets.button_animation, left_state,   15, 110);
    draw_animated_image_frame(assets.button_animation, right_state, 245, 110);
}

//
// Heart scene.
//
// In this scene, players will need to work together to get a heart pumping at
// a resonable rate. One player controls expansion of the heart, while the other
// controls contraction.
//

typedef struct
{
    Animated_Image heart;
    bool player_states[2];
    int time_stamps[2];
    int delta_ms;
    f32 target_beats_per_minute;
    f32 accuracy;
    bool expanding;
    bool draw_interface;
}
Heart_State;

Heart_State heart_state;

void heart_start(void * state)
{
    Heart_State * s = state;
    *s = (Heart_State){};
    s->heart = assets.heart_animation;
    s->heart.frame_duration_ms = 30;
    s->target_beats_per_minute = 60.0;
    if (!sound_is_playing(&mixer, assets.brown_sound))
    {
        play_sound(&mixer, assets.brown_sound, 0.05, 0.05, true);
    }
}

void heart_frame(void * state, f32 delta_time)
{
    Heart_State * s = state;

    draw_noise(0.15);

    if (s->expanding)
    {
        draw_animated_image_frames_and_wait(s->heart, 0, 3, 0, 20);
    }
    else
    {
        draw_animated_image_frames_and_wait(s->heart, 4, 6, 0, 20);
    }

    f32 range = 5.0;
    if (s->time_stamps[0] && s->time_stamps[1])
    {
        f32 beats_per_minute = 60.0f / (s->delta_ms * 0.001f);
        f32 d = s->target_beats_per_minute - beats_per_minute;
        d = clamp(-range * 10.0, -d, range * 10.0);
        s->accuracy += (d - s->accuracy) * 0.05;
    }

    if (s->draw_interface)
    {
        draw_accuracy_interface(s->accuracy, range,
            s->target_beats_per_minute,
            s->expanding, !s->expanding,
            s->player_states[0], s->player_states[1]);
    }
}

void heart_input(void * state, int player, bool pressed, u32 time_stamp_ms)
{
    Heart_State * s = state;
    s->player_states[player] = pressed;
    if (pressed)
    {
        play_sound(&mixer, assets.wood_block_sound,
            player ? 0.1 : 1.0,
            player ? 1.0 : 0.1,
            false);
        if (s->expanding != player)
        {
            s->time_stamps[player] = time_stamp_ms;
            s->heart.start_time_ms = time_stamp_ms;
            int a = s->time_stamps[0];
            int b = s->time_stamps[1];
            if (a && b) s->delta_ms = abs(a - b);
            s->expanding = player;
        }
    }
}

Scene heart_scene =
{
    .frame = heart_frame,
    .start = heart_start,
    .input = heart_input,
    .state = &heart_state,
};

//
// Lungs scene.
//
// In this scene, players hold and release their buttons in time with
// eachother and the beat.
//

typedef struct
{
    Animated_Image left_lung;
    Animated_Image right_lung;
    f32 target_beats_per_minute;
    f32 accuracy;
    int delta_ms[2];
    int time_stamps[2][2];
    int current_stamp[2];
    bool player_states[2];
    bool draw_interface;
}
Lungs_State;

Lungs_State lungs_state;

void lungs_start(void * state)
{
    Lungs_State * s = state;
    *s = (Lungs_State){};
    s->target_beats_per_minute = 60.0;
    s->left_lung = assets.left_lung_animation;
    s->right_lung = assets.right_lung_animation;
    s->left_lung.frame_duration_ms = 60.0;
    s->right_lung.frame_duration_ms = 60.0;
}

void lungs_frame(void * state, f32 delta_time)
{
    Lungs_State * s = state;

    draw_noise(0.15);

    if (s->player_states[0])
    {
        draw_animated_image_frames_and_wait(s->left_lung, 0, 3, 76, 40);
    }
    else
    {
        draw_animated_image_frames_and_wait(s->left_lung, 5, 7, 76, 40);
    }

    if (s->player_states[1])
    {
        draw_animated_image_frames_and_wait(s->right_lung, 0, 3, 76 + 75, 40);
    }
    else
    {
        draw_animated_image_frames_and_wait(s->right_lung, 5, 7, 76 + 75, 40);
    }

    f32 range = 5.0;
    if (s->time_stamps[0] && s->time_stamps[1])
    {
        f32 beats_per_minute[2];
        beats_per_minute[0] = 60.0f / (s->delta_ms[0] * 0.001f);
        beats_per_minute[1] = 60.0f / (s->delta_ms[1] * 0.001f);
        f32 target_delta = s->target_beats_per_minute - beats_per_minute[0];
        target_delta += s->target_beats_per_minute - beats_per_minute[1];
        target_delta /= 2.0;
        target_delta = clamp(-range * 10.0, -target_delta, range * 10.0);
        s->accuracy += (target_delta - s->accuracy) * 0.05;
    }

    if (s->draw_interface)
    {
        draw_accuracy_interface(s->accuracy, range,
            s->target_beats_per_minute,
            true, true,
            s->player_states[0], s->player_states[1]);
    }
}

void lungs_input(void * state, int player, bool pressed, u32 time_stamp_ms)
{
    Lungs_State * s = state;
    s->player_states[player] = pressed;
    s->time_stamps[player][s->current_stamp[player]] = time_stamp_ms;
    int a = s->time_stamps[player][0];
    int b = s->time_stamps[player][1];
    if (a && b) s->delta_ms[player] = abs(a - b);

    s->current_stamp[player] = !s->current_stamp[player];

    if (player == 0)
    {
        s->left_lung.start_time_ms = SDL_GetTicks();
        play_sound(&mixer, assets.shaker_sound, 0.4, 0.04, false);
    }
    else
    {
        s->right_lung.start_time_ms = SDL_GetTicks();
        play_sound(&mixer, assets.shaker_sound, 0.04, 0.4, false);
    }
}

Scene lungs_scene =
{
    .frame = lungs_frame,
    .start = lungs_start,
    .input = lungs_input,
    .state = &lungs_state,
};

//
// Digestion scene.
//
// In this scene players need to tap out a rhythm in 5/4 with one player
// tapping on the final beat of the bar, and one tapping the rest.
//

typedef struct
{
    Animated_Image digestion;
    int current_beat;
}
Digestion_State;

Digestion_State digestion_state;

void digestion_start(void * state)
{
    Digestion_State * s = state;
    *s = (Digestion_State){};
    s->digestion = assets.digestion_animation;
    s->digestion.frame_duration_ms = 30;
}

void digestion_frame(void * state, f32 delta_time)
{
    Digestion_State * s = state;
    draw_noise(0.15);
    if (s->current_beat == 5)
    {
        bool waiting = draw_animated_image_frames_and_wait(s->digestion, 4, 6, 117, 40);
        if (waiting) s->current_beat = 0;
    }
    else
    {
        draw_animated_image_frame(s->digestion, s->current_beat, 117, 40);
    }
}

void digestion_input(void * state, int player, bool pressed, u32 time_stamp_ms)
{
    Digestion_State * s = state;
    if (pressed)
    {
        if ((s->current_beat < 4 && player == 0) || (s->current_beat == 4 && player == 1))
        {
            s->current_beat = (s->current_beat + 1) % 6;
            s->digestion.start_time_ms = time_stamp_ms;
            if (s->current_beat == 5)
            {
                play_sound(&mixer, assets.wood_block_sound, 0.3, 1.0, false);
            }
            else
            {
                play_sound(&mixer, assets.tap_sound, 1.0, 0.3, false);
            }
        }
    }
}

Scene digestion_scene =
{
    .frame = digestion_frame,
    .start = digestion_start,
    .input = digestion_input,
    .state = &digestion_state,
};

~~~

\newpage

## Build System
This section presents the short and simple build system used to construct the project.

#### `build.sh`
~~~ {.sh .numberLines}
# Common flags.
FLAGS="main.c -o rhythm -Wall"

# macOS (clang)
clang $FLAGS -framework SDL2

# windows (MinGW)
gcc $FLAGS -mwindows -lmingw32 -lSDL2main -lSDL2

# Run on successful build.
if [[ $? -eq 0 ]]; then
    ./rhythm
fi
~~~
