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
