# Preliminary Report
The preliminary report for the final year project of Benedict Henshaw, written in February of 2018.
Source: http://gitlab.doc.gold.ac.uk/myk-project-students-20172018/benedict/
Demo: https://youtu.be/CIczNU_G6Eo

## Introduction
My project is a two-player rhythm-based video game, written 'from scratch' in C. It is highly influenced by the games Rhythm Tengoku (Nintendo, 2006) and WarioWare (Nintendo, 2003), both released on the GameBoy Advance. Like these games, my project will feature mini-games that will challenge two players on their ability to understand, follow, and set rhythms. It will also feature simple, low-resolution graphics, and live synthesised audio.

![Screenshots of Rhythm Tengoku](data/rhythm_tengoku_shots.png)

This document outlines everything I would like to achieve with my project, how I will achieve it, and what I have done so far.

## Aims and Objectives
My goals in the creation of this project are not only focused on the final product, but are highly influenced by the skills that I want to learn and practice. Here are some of the aims that I have for my project, each followed by some more concrete objectives.

### Build a Software Renderer
All the graphics in this project will be rendered by transforming pixel data in software. While this may not allow for the best performance on many modern machines (though it will definitely run fast enough), it will allow the software to be easily ported to platforms with differing graphics hardware and software, as well as being a great opportunity to learn more about the intricate details of graphics code.

+ Display bitmap graphics on screen.
+ Bitmap rotation and scaling.
+ Run at a high frame rate; at least 60 frames per second.

### Build a Custom Audio Mixer and Synthesis Tools
My game has a large focus on audio, especially the timing of that audio. I want to construct an audio mixer and back end that supports my game better than any currently available mixer libraries. I would also like to take this opportunity to learn more about audio synthesis, as I know far less about it than graphical synthesis.

+ Output sound.
+ Mix multiple sounds together.
+ Synthesise sounds.

### Implement Several Mini-Games
I feel that the for the project to be most enjoyable, some variety would be ideal. To achieve this I would like to implement several mini-games that have different player goals.

+ Implement two or more mini-games.
+ Design a system for transitioning between mini-games.

### Have Input that Feels Responsive
Rhythm games place far higher importance on the timing of input that many other games. In order for the player's experience to be enjoyable, their input must feel responsive.

+ Low time between button press and auditory / graphical response; ideally less than 10ms.
+ Controller support.
+ Easy-to-use button remapping.

### Write the Entire Project in C, without Libraries
I want to learn as much as I can from this project, therefore I would like to implement as many of the features of my program as I feasibly can. C is my language of choice, and is suitable for this project for its performance characteristics.

+ Implement as many of the features of the project as I can (i.e. avoid libraries).
+ Build custom memory allocators.
+ Design custom asset file formats.

### Make it Fun
Despite the technical implementation being a strong factor in the design of my project, I would like to create a game that is enjoyable to play.

+ Make someone smile.

## Methods
Here are some ways in which I will tackle the problems that I have laid out in my aims and objectives.

### Build a Software Renderer
There are several key components of a software renderer. The most important is some way to set pixels on the display. This most often comes in the form of a bitmap that can be handed to the operating system to be displayed in the window. Also common is a way to select portions of the window to be updated individually, which can improve performance and avoid the need to have a program-side copy of the entire window's pixels. I will most likely choose the former method, as my program will often be modifying many pixels on screen every frame.

Another key component of a software renderer is the data format. The majority of pixel formats are packed integers, usually one byte per channel; so RGB pixels are three bytes, RGBA are four. The order in which the channels are arranged varies wildly. As I will have my own program-side pixel buffer that I will manipulate and send off each frame, I will choose a single pixel format to use during the composition of the graphics, and then convert it to the appropriate format (if needed) before displaying. I may even choose to use a pallet-based format, where bitmaps hold indices into a table of colours.

Most, if not all, of my graphics will be composed from bitmap images. In order to display these bitmaps all that needs to be done is to copy their data into a portion of the screen pixel buffer. If I find a need for rotation and scaling I will implement these features using a nearest neighbour based algorithm, as this fits with the aesthetic style of the game and is the easiest method.

Several things will allow my software renderer to run at a high frame rate. Firstly, I have chosen to run at a low resolution (320px by 200px). This is a very low number of pixels to be calculated per frame by modern standards. I will also be implementing a custom memory management system, which will allow no system memory allocation or deallocation to occur during the frame loop of the program; this is something that slows down many modern programs. Outside of these efforts, I will simply keep any eye on the performance of the program, and will avoid adding any unnecessary features that may slow it down too much. This will require testing on multiple machines and platforms to ensure that the game runs well on each. I would like the game to support a cheap computer like the Raspberry Pi.

### Build a Custom Audio Mixer and Synthesis Tools
To play more than one audio stream an audio mixer is needed. I have decided that the mixer will operate on floating-point data only. The mixer will have several streams of audio, each having adjustable stereo volume.

To perform audio synthesis, I will construct a set of virtual instruments. There will be two kinds: Pure synthesis and sample based. Pure synthesis instruments will generate sound given some parameters, and sample based instruments will generate sound by copying from samples of audio data. The former could be used for melody, and the latter, percussion.

To make music from my instruments I will implement a sequencer. It will hold a set of timings paired with parameters (such as pitch), that will be used to trigger generation of audio. I would like to use the fact that the music is generated live to incorporate more interactive elements in the composition.

### Implement Several Mini-Games
I will implement a 'scene' system to support separate mini-games. Each scene will have its own state and functions to affect that state, including rendering graphics, handling input, and playing audio. This will be done by using a set of function pointers that can be swapped out based on which scene is currently active.

The first mini-game I plan to implement is themed around the beating of a heart. One player's button will expand the heart, the other's will contract it. They will have to work together to maintain a reasonable heart rate. If the rate falls too low for too long they will loose. To win, they must keep a good rate for a certain amount of time.

The second mini-game is competitive. Each player will use their button presses to control an arm in an arm wrestle. They must tap fast to push harder. The player that pushes the opponents hand over wins.

I hope to flesh out some more ideas during the development of the game.

### Have Input that Feels Responsive
The major concern for this issue is the time between button press and output response. There are several factors that add to this time: how often events are handled, how long after that a response is represented in the program's output, and any delay incurred by the hardware or the driver or OS. The final factor is mostly out of my control, but the first two are where some ground can be made.

Input events must be handled frequently. This is often done once per frame. This may be enough for this project, but I will have to do some testing to figure that out. If not, I may use a separate thread to handle input.

To lower audio latency, I must send off smaller chunks of audio data, but more frequently. This allows changes to the audio state to be made more often, allowing the results of user input to affect the audio output faster. This can be done by running at a high frame rate and pushing smaller amounts of audio each frame, or by lowering the size of the buffer used in the audio thread.

### Write the Entire Project in C, without Libraries
As I want to learn as much as possible during the construction of this project, I would like to build many major systems to support the game. This includes an asset management system, supporting asset loading and ideally a packed asset file which would allow all the game assets to be transported as a single file. If I have the time, I would also like to write some basic compression algorithms to reduce the size of this file.

I would also like to build a memory management system that supports my game better than the `malloc`/`free` model often employed in C. My system will use memory pools, allocated at the start of the program (possibly statically), and handed out when needed. There will be a pool for each major memory lifetime group, and deallocation will occur once that lifetime has been exceeded. For example, some allocations are made during the frame loop and their lifetime is only for that frame, so they can be allocated from a pool that is emptied each frame. Together, this will allow memory allocation to only take up a few cycles each time.

### Make it Fun
User testing will help me keep on track to building a fun game. I will allow people to play the game for a short period of time, and then ask them to complete a survey. This survey will ask them about their thoughts on the game, which parts they found most or lest enjoyable, and more. I will also watch players as they play the game to help identify what aspects of the game are causing problems.

## Project Plan
This is the overview of my project development time line. Goals that have been completed are struck through.

#### January

+ ~~Create software structure plan~~ *(1 week)*
+ ~~Implement basic bitmap renderer~~ *(3 days)*
+ ~~Implement basic audio output~~ *(3 days)*
+ ~~Implement memory allocation system~~ *(2 days)*

#### February

+ Build one mini-game *(~2 weeks)*
    - ~~Fully design a mini-game~~ *(3 days)*
    - Create assets for the mini-game *(1 day)*
    - Build basic bitmap animation support *(1 day)*
    - Build the mini-game *(1 week)*
+ Collect some feedback from user testing *(1 week)*
+ Build a framework to support multiple mini-games *(3 days)*

#### March

+ Build a better audio subsystem *(1 week)*
+ Improve the asset subsystem *(1 week)*
+ Design another mini-game *(3 days)*

#### April

+ Build another mini-game *(2 weeks)*
+ Create/source assets *(1 weeks)*
+ Collect more user testing feedback *(3 days)*

#### May

+ Polish based on feedback *(1 week)*
+ Submit Project

## Progress
So far, I have implemented the following:

+ Displaying pixels
+ Bitmap rendering
+ Bitmap scaling
+ Audio output
+ Memory allocator
+ Bitmap file loading
+ Receiving input from keyboard and controllers

## Planned Work
Here is a high-level overview of all things that are needed to complete the game:

+ Graphics
    + Bitmap rendering
+ Audio
    + Sample output
    + Synthesis
    + Sequencer
    + Mixer
+ Input
    + Event time-stamping
    + Timing comparison
+ Assets
    + Bitmap loading
    + Sound loading
+ Memory allocator
+ Create/source assets

## References
Wikipedia articles for mentioned video-games.
https://en.wikipedia.org/wiki/Rhythm_Tengoku
https://en.wikipedia.org/wiki/WarioWare,_Inc.:_Mega_Microgames!

Some numerical evidence to suggest that C is a 'fast' language.
http://benchmarksgame.alioth.debian.org/u64q/which-programs-are-fastest.html
