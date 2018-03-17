# The Audio Mixer (2018-01-21)
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
