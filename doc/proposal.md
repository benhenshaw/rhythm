# Final Project
BSc Games Programming</br>
Year 3, 2017 - 2018</br>
Benedict Henshaw</br>

## Overview
> Competitive local multi-player one-button rhythm game.

My final project is a video game. It is a rhythm game in the vein of 'Rhythm Tengoku' and the 'WarioWare' series, and will be all about keeping time with music.

The focus is on multi-player mini-games which each have their own spin on keeping time.

## Mini-Games
I would like to implement at least one of these mini-games by the end of my project.

### Arm wrestling
Each player's button pressing controls one arm. The arm that pushes the other over wins.

#### Possible mechanics
They must press in time with the beat, but they can choose at what rate they press. The force with with each arm is pressing is based on the accuracy and the rate at which they press. Whichever arm has the highest force will push the other until one arm is bent completely over.

### Horse racing
Each player's pressing controls a horse in a race. The horse that reaches the end fastest wins.

#### Possible mechanics
To run faster the player must press at different time signatures and rhythms. Their speed is based on accuracy and the rhythm at which they press.

The player must transition into the next rhythm to increase speed.

### Factory
Each player controls a robotic arm that is taking objects off a conveyor belt in a factory. The robot that has successfully collected the most objects or reaches a target amount wins.

#### Possible mechanics
Both players press once to put the object on the conveyor belt. The object can end up further to the left or right of the belt based on who was most accurate in the first press. Then they must press again to pick up the object.

## Technology
I want to write the program in C, using SDL2 as a platform layer. I will write a custom audio back end and sequencer for playing custom audio tracks and performing some on-the-fly synthesis. The graphics will be handled by a custom 2D software-based renderer. I will handle memory using some custom allocation techniques, but this game does not require much data throughput so these systems will be relatively simple. Some basic multi-threading will be used; possibly employing a job system. I hope to achieve low-latency input and audio output, and 60 FPS video output.

### Input
When the user presses a button, that event will be queued. During the next frame, the event queue will be run through and each event handled. This may be done more often if the game does not feel responsive enough. These events are timestamped and I will use that time stamp to check if a player has pressed the button in time with the music. I would like to allow external devices like game controllers to be used in the game too.

### Audio
I will be employing a custom audio mixer for this project. I will allow for mixing of several 'channels' of PCM audio data. This will allow playback of multiple sounds at once, and may be used to handle individual 'instruments' in an audio track. It will also allow me to know with high accuracy how much music has played and this will help match input to audio output.

A sequencer will handle playing music. The sequencer will have 'instruments' that can produce sound at will with some parameters. There will be at least two kinds of instruments: sample-based and synthesis-based. Sample-based instruments will simply write some pre-recorded sound data into the buffer, while synthesis-based instruments will generate sound data on-the-fly. I would also like to employ some audio effects, such as delay and some filters (low-pass, high-pass, etc.).

I may need to write a secondary tool for authoring audio sequences, though it may just be a system that interprets a plain text format.

### Graphics
A custom 2D software renderer will fill a buffer with pixels each frame. The most important feature will be a simple sprite copy, but will also have rotation and scaling features. It will handle text using a bitmap font system. Some graphical primitives may also be implemented. Alpha keying (if not alpha blending) will allow for transparency in images. A sprite sheet will contain all graphical elements and the renderer will have a table of locations for each sprite.

### Data
Graphical data will be handled in the form of '.png' or '.bmp' images. I will opt for '.png' if the total file size of the images becomes inconveniently large using the uncompressed '.bmp'. Sprite sheets will allow lots of graphical components to exist inside one file, which will improve performance when loading data from disk.

Audio sample data will be in '.wav' format unless there is so much of it that a compressed format is needed. In that case I will use '.ogg' as there is a high quality public-domain library available for decoding. 'Audio fonts' will allow lots of separate sounds to share a single file, to improve load performance (and decode performance if used).

A basic (probably plain-text) file format will be used if any player settings or save data needs to be stored.
