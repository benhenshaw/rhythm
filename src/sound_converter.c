//
// sound_converter.c
//
// This is a separate program used to convert .wav audio files to the custom
// audio format.
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

Sound read_sound(char * file_name) {
    Sound result = {};

    FILE * file = fopen(file_name, "r");
    if (!file_name) return result;

    fscanf(file, "%*[AUDIO_F32]\n");

    int sample_count = 0;
    fscanf(file, "SAMPLE_COUNT %d\n", &sample_count);

    fscanf(file, "%*[ENDHDR]\n");

    if (sample_count > 0) {
        f32 * samples = calloc(sample_count, sizeof(f32));
        int samples_read = fread(result.samples, sizeof(f32), sample_count, file);
        fclose(file);
        if (samples_read == sample_count) {
            result.samples      = samples;
            result.sample_count = sample_count;
        }
    }

    return result;
}

bool write_sound(Sound sound, char * file_name) {
    FILE * file = fopen(file_name, "w");
    if (!file_name) return false;
    fprintf(file, "AUDIO_F32\nSAMPLE_COUNT %d\nnENDHDR\n", sound.sample_count);
    int samples_written = fwrite(sound.samples, sizeof(f32), sound.sample_count, file);
    return samples_written == sound.sample_count;
}

int main(int argument_count, char ** arguments) {
    if (argument_count < 2) {
        fprintf(stderr, "Usage:\n\t%s in.wav out.af32\n", arguments[0]);
        return 0;
    }

    char * in_file_name  = arguments[1];
    char * out_file_name = arguments[2];

    Sound sound = read_sound(in_file_name);
    if (sound.samples) {
        printf("Read %d samples from '%s'.\n", sound.sample_count, in_file_name);

        if (write_sound(sound, out_file_name)) {

        } else {

        }
    } else {
        perror("ERROR: File '%s' could not be read");
    }
}
