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

typedef struct {
    f32 * samples;
    int sample_count;
} Sound;

//
// Audio Mixer.
//
// The mixer consists of a number of channels, each of which can hold a sound
// along with a set of parameters to control it's playback.
//

typedef struct {
    float * samples;    // The audio data itself.
    int sample_count;   // Number of samples in the data.
    int sample_index;   // Index of the last sample written.
    float left_gain;    // How loud to play the sound in the left channel.
    float right_gain;   // Same for the right channel.
    bool loop;          // If the sound should repeat.
    bool playing;       // If the sound is playing right now or not.
} Mixer_Channel;

typedef struct {
    Mixer_Channel * channels;
    int channel_count;
    float gain;
} Mixer;

// TODO: Where should the mixer live?
Mixer mixer;

// Initialise the audio mixer.
Mixer create_mixer(int pool_index, int channel_count, float gain) {
    Mixer mixer = {};
    mixer.channels = pool_alloc(pool_index, channel_count * sizeof(Mixer_Channel));
    if (mixer.channels) {
        mixer.channel_count = channel_count;
        mixer.gain = gain;
    }
    return mixer;
}

// The main audio mixing function. This will likely be called on a separate thread,
// so some care should be taken when considering input and output.
void mix_audio(Mixer * mixer, void * stream, int samples_requested) {
    float * samples = stream;

    // Zero the entire buffer first.
    for (int sample_index = 0; sample_index < samples_requested; ++sample_index) {
        samples[sample_index] = 0.0f;
    }

    // Mix all of the data from a channel into to buffer,
    // then move on to the next channel.
    for (int channel_index = 0; channel_index < mixer->channel_count; ++channel_index) {
        Mixer_Channel * channel = &mixer->channels[channel_index];
        if (channel->samples && channel->playing) {
            for (int sample_index = 0;
                 sample_index < samples_requested &&
                 channel->sample_index < channel->sample_count;
                 ++sample_index) {
                // Load a mono sample from the channel.
                float new_left  = channel->samples[channel->sample_index];
                float new_right = channel->samples[channel->sample_index];

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
            if (channel->sample_index >= channel->sample_count) {
                if (channel->loop) {
                    channel->sample_index = 0;
                } else {
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
int play_sound(Mixer * mixer, Sound sound, float left_gain, float right_gain, int loop) {
    for (int i = 0; i < mixer->channel_count; ++i) {
        if (mixer->channels[i].samples == NULL) {
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
int queue_sound(Mixer * mixer, Sound sound, float left_gain, float right_gain, int loop) {
    for (int i = 0; i < mixer->channel_count; ++i) {
        if (mixer->channels[i].samples == NULL) {
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
bool play_channel(Mixer * mixer, int channel_index) {
    if (channel_index >= 0 && channel_index <= mixer->channel_count &&
        mixer->channels[channel_index].samples) {
        SDL_LockAudioDevice(audio_device);
        mixer->channels[channel_index].playing = true;
        SDL_UnlockAudioDevice(audio_device);
        return true;
    }
    return false;
}

// Tell a loaded channel to stop playing (but stay loaded).
// Returns true if the given channel was successfully told to pause.
bool pause_channel(Mixer * mixer, int channel_index) {
    if (channel_index >= 0 && channel_index <= mixer->channel_count &&
        mixer->channels[channel_index].samples) {
        SDL_LockAudioDevice(audio_device);
        mixer->channels[channel_index].playing = false;
        SDL_UnlockAudioDevice(audio_device);
        return true;
    }
    return false;
}
