//
// sound_converter.c
//
// This is a separate program used to convert .wav audio files to the custom
// audio format. It is quick and dirty, so don't look too closely.
//

#include <stdlib.h>
#include <stdio.h>

typedef float f32;
typedef _Bool bool;
#define true 1
#define false 0

typedef struct {
    f32 * samples;
    int sample_count;
} Sound;

Sound read_pcm(char * file_name) {
    Sound result = {};

    FILE * file = fopen(file_name, "r");
    if (!file_name) return result;

    int samples_read = 0;
    int total_samples_read = 0;
    int chunk_sample_count = 512;

    f32 * samples = malloc(chunk_sample_count);
    if (!samples) return result;

    while (true) {
        samples_read = fread(samples, sizeof(f32), chunk_sample_count, file);
        total_samples_read += samples_read;
        if (samples_read > 0) {
            samples = realloc(samples, total_samples_read + chunk_sample_count);
            if (!samples) return result;
        } else {
            break;
        }
    }

    result.samples = samples;
    result.sample_count = total_samples_read;

    return result;
}

bool write_pfm(Sound sound, char * file_name) {
    FILE * file = fopen(file_name, "w");
    if (!file_name) return false;
    fprintf(file, "AUDIO_F32\nSAMPLE_COUNT %d\nENDHDR\n", sound.sample_count);
    int samples_written = fwrite(sound.samples, sizeof(f32), sound.sample_count, file);
    return samples_written == sound.sample_count;
}

int main(int argument_count, char ** arguments) {
    if (argument_count < 2) {
        fprintf(stderr, "Usage:\n\t%s in.pcm out.sound\n", arguments[0]);
        return 0;
    }

    char * in_file_name  = arguments[1];
    char * out_file_name = arguments[2];

    Sound sound = read_pcm(in_file_name);
    if (sound.samples) {
        printf("Read %d samples from '%s'.\n", sound.sample_count, in_file_name);

        if (write_pfm(sound, out_file_name)) {

        } else {

        }
    } else {
        perror("ERROR: File '%s' could not be read");
    }
}
