//
// assets.c
//
// This file contains:
//     - Image loader.
//     - Image writer.
//     - Sound loader.
//     - Sound writer.
//

//
// Image files.
//
// The Portable Arbitrary Map image file format is used. It is one of the simplest
// image formats that allows the RGBA pixel format (notably the alpha channel). It
// stores the pixels uncompressed, with a simple header.
//
// An example of the .pam header:
// P7                     <-- Magic number.
// WIDTH 256              <-- Image width (in pixels).
// HEIGHT 256             <-- Image height (in pixels).
// DEPTH 4                <-- Number of channels per pixel.
// MAXVAL 255             <-- Highest value a channel can be.
// TUPLTYPE RGB_ALPHA     <-- The pixel format. (Can also be grayscale, etc.)
// ENDHDR                 <-- Marker for the end of the header.
//

// Reverse the order of the channels of a four channel pixel.
void abgr_to_rgba(void * pixels, int pixel_count) {
    for (int pixel_index = 0; pixel_index < pixel_count; ++pixel_index) {
        u32 p = ((u32 *)pixels)[pixel_index];
        ((u32 *)pixels)[pixel_index] = rgba(get_alpha(p), get_blue(p), get_green(p), get_red(p));
    }
}

// Load an RGBA format .pam file into an Image.
// Returns a zero'd Image if unsuccessful.
Image read_image_file(int pool_index, char * file_name) {
    FILE * file = fopen(file_name, "r");
    if (!file) return (Image){};
    int width = 0, height = 0;
    fscanf(file, "P7\nWIDTH %d\nHEIGHT %d\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n", &width, &height);
    if (width && height) {
        int pixel_count = width * height;
        u32 * pixels = pool_alloc(pool_index, width * height * sizeof(u32));
        int pixels_read = fread(pixels, sizeof(u32), pixel_count, file);
        fclose(file);
        if (pixels_read == pixel_count) {
            abgr_to_rgba(pixels, pixel_count);
            return (Image){
                .pixels = pixels,
                .width = width,
                .height = height
            };
        }
    }
    return (Image){};
}

// Writes an Image to a .pam file.
// Returns true if the entire Image was successfully written.
bool write_image_file(Image image, char * file_name) {
    FILE * file = fopen(file_name, "w");
    if (!file) return false;
    fprintf(file, "P7\nWIDTH %d\nHEIGHT %d\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n", image.width, image.height);
    int byte_count = image.width * image.height * sizeof(image.pixels[0]);
    int bytes_written = fwrite(image.pixels, 1, byte_count, file);
    fclose(file);
    return byte_count == bytes_written;
}

//
// Audio files.
//
// A simple custom file format is used to store audio data. It is derived from
// the .pam format described above.
//
// An example of the file header:
// SND                    <-- Magic number.
// SAMPLE_COUNT 48000     <-- The number of samples in the file.
// ENDHDR                 <-- Marker for the end of the header.
//

// Reads a mono f32 format audio file.
// Returns a zero'd Sound if unsuccessful.
Sound read_sound_file(int pool_index, char * file_name) {
    FILE * file = fopen(file_name, "r");
    if (!file) return (Sound){};
    int sample_count = 0;
    fscanf(file, "SND\nSAMPLE_COUNT %d\nENDHDR\n", &sample_count);
    if (sample_count) {
        f32 * samples = pool_alloc(pool_index, sample_count * sizeof(f32));
        int samples_read = fread(pixels, sizeof(f32), sample_count, file);
        fclose(file);
        if (samples_read == sample_count) {
            return (Sound){
                .samples = samples,
                .sample_count = sample_count,
            };
        }
    }
    return (Sound){};
}

// Writes a sound into a file.
// Returns true if the entire Sound was successfully written.
bool write_sound_file(Sound sound, char * file_name) {
    FILE * file = fopen(file_name, "w");
    if (!file) return false;
    fprintf(file, "SND\nSAMPLE_COUNT %d\nENDHDR\n", sound.sample_count);
    int byte_count = sound.sample_count * sizeof(sound.samples[0]);
    int bytes_written = fwrite(sound.samples, 1, byte_count, file);
    fclose(file);
    return byte_count == bytes_written;
}
