//
// audio.c
//
// This file contains:
//     - Audio Mixer.
//

typedef struct {
    float * samples;    // The audio data itself.
    int sample_count;   // Number of samples in the data.
    int sample_index;   // Index of the last sample written.
    float left_gain;    // How loud to play the sound in the left channel.
    float right_gain;   // Same for the right channel.
    int loop;           // If the sound should repeat.
} Mixer_Channel;

typedef struct {
    Mixer_Channel * channels;
    int channel_count;
    float gain;
} Mixer;

Mixer create_mixer(int channel_count, float gain) {
    Mixer mixer = {};
    mixer.channels = calloc(channel_count, sizeof(Mixer_Channel));
    if (mixer.channels) {
        mixer.channel_count = channel_count;
        mixer.gain = gain;
    }
    return mixer;
}

void mix_audio(Mixer * mixer, void * stream, int samples_requested) {
    float * samples = (float *)stream;

    for (int sample_index = 0; sample_index < samples_requested; ++sample_index) {
        samples[sample_index] = 0.0f;
    }

    for (int channel_index = 0; channel_index < mixer->channel_count; ++channel_index) {
        Mixer_Channel * channel = &mixer->channels[channel_index];
        if (channel->samples) {
            for (int sample_index = 0;
                 sample_index < samples_requested && channel->sample_index < channel->sample_count;
                 ++sample_index) {
                float new_left  = channel->samples[channel->sample_index];
                float new_right = channel->samples[channel->sample_index];

                new_left  *= channel->left_gain;
                new_left  *= mixer->gain;
                new_right *= channel->right_gain;
                new_right *= mixer->gain;

                samples[sample_index] += new_left;
                ++sample_index;
                samples[sample_index] += new_right;

                channel->sample_index += 1;
            }

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

int play_audio(Mixer * mixer, void * stream, int sample_count, float left_gain, float right_gain, int loop) {
    for (int i = 0; i < mixer->channel_count; ++i) {
        if (mixer->channels[i].samples == NULL) {
            mixer->channels[i].samples      = stream;
            mixer->channels[i].sample_count = sample_count;
            mixer->channels[i].sample_index = 0;
            mixer->channels[i].left_gain    = left_gain;
            mixer->channels[i].right_gain   = right_gain;
            mixer->channels[i].loop         = loop;
            return i;
        }
    }
    return -1;
}
