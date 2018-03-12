//
// sound_converter.c
//
// This is a separate program used to convert .wav audio files to the custom
// audio format. It is quick and dirty, so don't look too closely.
//
// Usage:
//     sc input.wav output.sound
//

#include <SDL2/SDL.h>
#include <SDL2/SDL_stdinc.h>

#include "common.c"

typedef struct {
    f32 * samples;
    int sample_count;
} Sound;

bool write_sound_file(Sound sound, char * file_name) {
    FILE * file = fopen(file_name, "w");
    if (!file_name) return false;
    fprintf(file, "SND\nSAMPLE_COUNT %d\nENDHDR\n", sound.sample_count);
    int byte_count = sound.sample_count * sizeof(sound.samples[0]);
    int bytes_written = fwrite(sound.samples, 1, byte_count, file);
    fclose(file);
    return byte_count == bytes_written;
}

int main(int argument_count, char ** arguments) {
    SDL_AudioSpec spec = { .format = AUDIO_F32, .freq = 48000, .channels = 1 };
    u8 * stream;
    u32 byte_count = 0;
    printf("Loading '%s'.\n", arguments[1]);
    SDL_assert(SDL_LoadWAV(arguments[1], &spec, &stream, &byte_count));
    printf("%d bytes loaded.\n", byte_count);
    Sound sound = {
        .samples = (f32 *)stream,
        .sample_count = byte_count / sizeof(f32),
    };
    printf("Writing %d samples (%.2fs) to '%s'.\n",
        sound.sample_count, (f32)sound.sample_count / (f32)spec.freq, arguments[2]);
    SDL_assert(write_sound_file(sound, arguments[2]));
    printf("Success!\n");
}
