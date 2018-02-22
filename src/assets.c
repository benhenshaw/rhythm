//
// assets.c
//
// This file contains:
//     - Image loader.
//     - Image writer.
//

// Reads a .pam image file.
// Only allows RGBA format.
Image load_pam(int pool_index, char * file_name) {
    Image result = {};

    // Attempt to open the file.
    FILE * file = fopen(file_name, "r");
    if (!file) return result;

    bool success = true;

    // Header example:
    // P7
    // WIDTH 256
    // HEIGHT 256
    // DEPTH 4
    // MAXVAL 255
    // TUPLTYPE RGB_ALPHA
    // ENDHDR

    // Read the file header.
    int type_number = 0;
    fscanf(file, "P%d\n", &type_number);
    success = type_number == 7;

    int width = 0;
    fscanf(file, "WIDTH %d\n", &width);
    success = width > 0;

    int height = 0;
    fscanf(file, "HEIGHT %d\n", &height);
    success = height > 0;

    int depth = 0;
    fscanf(file, "DEPTH %d\n", &depth);
    success = depth == 4;

    int max_val = 0;
    fscanf(file, "MAXVAL %d\n", &max_val);
    success = max_val == 255;

    char type[16];
    #define RGBA_TYPE "RGB_ALPHA"
    fscanf(file, "TUPLTYPE %16s\n", type);
    success = equal(type, RGBA_TYPE, sizeof(RGBA_TYPE));
    #undef RGBA_TYPE

    fscanf(file, "%*[ENDHDR]\n");

    if (success) {
        result.pixels = pool_alloc(pool_index, width * height * sizeof(u32));
        int pixels_read = fread(result.pixels, sizeof(u32), width * height, file);
        fclose(file);

        // Swap endianness of pixels.
        for (int pixel_index = 0; pixel_index < pixels_read; ++pixel_index) {
            u32 p = result.pixels[pixel_index];
            result.pixels[pixel_index] = rgba((p & 0x000000ff) >> 0,
                                              (p & 0x0000ff00) >> 8,
                                              (p & 0x00ff0000) >> 16,
                                              (p & 0xff000000) >> 24);
        }

        result.width = width;
        result.height = height;
    }

    return result;
}

bool write_pam(Image image, char * file_name) {
    FILE * file = fopen(file_name, "w");
    if (!file_name) return false;
    fprintf(file, "P7\nWIDTH %d\nHEIGHT %d\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n", image.width, image.height);
    int byte_count = image.width * image.height * sizeof(image.pixels[0]);
    int bytes_written = fwrite(image.pixels, 1, byte_count, file);
    return byte_count == bytes_written;
}

Sound read_sound(int pool_index, char * file_name) {
    Sound result = {};

    FILE * file = fopen(file_name, "r");
    if (!file_name) return result;

    fscanf(file, "%*[AUDIO_F32]\n");

    int sample_count = 0;
    fscanf(file, "SAMPLE_COUNT %d\n", &sample_count);

    fscanf(file, "%*[ENDHDR]\n");

    if (sample_count > 0) {
        f32 * samples = pool_alloc(pool_index, sample_count * sizeof(f32));
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
