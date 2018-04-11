*_DRAFT_*
*Each underlined sentence is a rough description of what the next paragraph will discuss. The majority of those paragraphs are incomplete.*

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
*_Rhythm game with supporting systems built from scratch..._*

## Introduction
*_Discuss the game idea as it relates to the games that inspired it._*
This document discusses the process of developing a video game based on the concept of rhythm and the interactions of two players as they work together and compete in a series of mini-games. Each mini-game will attempt to challenge the players in different ways.

The game is inspired by the video games 'Rhythm Heaven (リズム天国)' (2006) and 'WarioWare' (2003), both of which were released on the Nintendo Game Boy Advance. Both of these games feature mini-games and have a humorous theme, represented in their graphics and mechanics.

*_State that the project has a focus on DIY for educational purposes, and argue that it also creates better software._*
I created this project in the C programming language, implementing many of the technical systems that support the game. This includes graphics, audio, memory management, and input. This was done to further my own understanding of these aspects of game programming, and with the goal of creating a lightweight and efficient program (when compared to using pre-existing libraries and game engines).

*_Discuss the structure of the this report._*
This report will cover in detail the conceptual development of the project, as well as the planning, implementation, testing, and evaluation of the software.

## Background
*_Discuss the high-level design of the game as it relates to other games, emphasising what is novel about it._*
In order to differentiate my project from games that have come before it, I sought to find an aspect of the game-play that I could innovate on. I decided to explore the concept of multi-player.

*_Discuss the technical sub-systems of the project, with examples and critiques of how they are implemented in other games. (Several paragraphs.)_*

## Specification
*_Discuss the range of interactions the mini-games will explore._*

*_Discuss desires for what the player will experience when playing._*

*_Discuss technical performance goals, and how they support design goals._*

## Design and Implementation
*_Discuss the overall design of the game describing the mini-games and their characteristics. (Several paragraphs.)_*

### Overview
*_Discuss the high-level design and structure of the software._*
The project has five major sub-systems, and a core that ties them together. These sub-systems are Assets, Audio, Graphics, Memory, and Scenes. Each sub-system resides in a single C source file.

There are two major kinds of assets in this project: bitmap graphics, and PCM audio. Graphics are stored using a file format called Portable Arbitrary Map, which contains a brief ASCII text header followed by a block of uncompressed pixel data. Audio is stored in a custom file format designed to match the PAM format used for graphics, following the same principal of plain-text header and uncompressed raw data.

Audio is outputted to the sound hardware via a callback supplied by the platform (initially, the SDL2 library). This callback is run on a separate high-priority thread, meaning it is given more CPU time by the OS scheduler. In this callback, a custom audio mixer creates a single stream of floating-point PCM data and copies it into the output buffer. The mixer holds several separate sounds along with some parameters for how it should be played.

Graphics, much like sound, is handled by a custom renderer that produces a single final bitmap to be displayed on screen. While there is support for some graphical primitive rendering, the core functionality of the renderer is drawing bitmap graphics to the screen. These bitmap graphics can be any size, supporting transparency. They can also also be animated (composed of several bitmaps), or text, generated from a set of character bitmaps.

Memory is handled simply and efficiently by a custom memory allocator. The allocator holds a large pool of memory and sub-allocates from the pool on request. There are several sub-pools each allocating memory with a certain lifetime allowing freeing of memory to be automatic and efficient.

Scenes are used to encapsulate different pieces of the game. Each scene contains some state and a set of functions that handle input and output. This is where game-play code is written. Transitions between scenes can be made at will from anywhere in the program.

### Assets
*_Discuss the management of assets and custom file formats._*
All assets are loaded by custom file readers.

### Audio
*_Discuss the playback of audio and the mixer._*
Audio is output by a high-frequency callback. This callback requests a number of samples, which is produced at will by the custom audio mixer. This audio mixer has a list of all playing sounds and their current state, and uses this to mix together a single stream of audio for playback.

*_Discuss management audio synthesis and effects._*

### Graphics
*_Discuss general graphics info._*
All of the graphics in this project are created using a custom built software renderer. The software renderer produces a single final bitmap which is displayed on screen.

*_Discuss the rendering of still bitmaps._*
Each singular bitmap is rendered by copying an array of pixels into the main pixel buffer.

*_Discuss the rendering of animations._*
Animations have a time in milliseconds that states how long each frame of the animation will be displayed on screen. Using this time, and a time-stamp from when the animation started playing, the current frame can be deduced and displayed much the same way as a still bitmap.

*_Discuss the rendering of text._*
Text is rendered using a bitmap font; as opposed to generating font geometry on-the-fly. A bitmap image holds all of the drawable characters defined in the ASCII standard. The location of a character in the image can be calculated using its ASCII code as an offset from the first character.

### Memory
*_Discuss the management of memory._*
The project employs a custom memory allocator. It is very simple, with the main allocation function consisting of only 12 lines of code. This allocator follows a principal often employed in high-performance video games: allocate up front, and sub-allocate after that point.

### Scenes
*_Discuss the management of scenes and mini-games._*
Scenes are sets of function pointers that can be swapped out at will. These function pointers are called at specific times, including when the user presses a button, or when a frame of graphics needs to be rendered.

Each mini-game is a single scene, and each menu is also a separate scene.

## Testing and Evaluation
*_Discuss the knowledge gathered about how games are tested._*

*_Discuss the methods chosen, and specifics about how testing was carried out._*

*_Show the information collected during testing._*

*_Show how testing information was used to improve the project._*

*_Discuss the cycles of iteration._*

## Conclusion
*_Evaluate the methods used to construct the game._*

*_Evaluate the software performance and responsiveness with timings and user feedback._*

*_Discuss the topics learned, evaluating their importance and difficulty._*

*_Evaluate the final game produced, stating my opinions and how the outcome relates to the initial goals._*

## Bibliography
+ Handmade Hero (https://handmadehero.org/)
+ ...

## Appendix A
*_Program source code._*

## Appendix B
*_Custom tools and build management source code._*

## Appendix C
*_Copy of the original proposal._*

*_Copy of weekly logs._*
