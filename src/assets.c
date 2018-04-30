//
// assets.c
//
// This file contains:
//     - Image loader and writer.
//     - Sound loader and writer.
//     - Asset loading and storing.
//

//
// Image files.
//
// The Portable Arbitrary Map image file format is used. It is one of the simplest
// image formats that allows the RGBA pixel format. It stores the pixels
// uncompressed, with a simple header.
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
void abgr_to_rgba(void * pixels, int pixel_count)
{
    for (int pixel_index = 0; pixel_index < pixel_count; ++pixel_index)
    {
        u32 p = ((u32 *)pixels)[pixel_index];
        ((u32 *)pixels)[pixel_index] = rgba(
            get_alpha(p), get_blue(p), get_green(p), get_red(p));
    }
}

// Load an RGBA format .pam file into an Image.
// Returns a zero'd Image if unsuccessful.
Image read_image_file(int pool_index, char * file_name)
{
    FILE * file = fopen(file_name, "r");
    if (!file) return (Image){};
    int width = 0, height = 0;
    fscanf(file, "P7\n"
        "WIDTH %d\n"
        "HEIGHT %d\n"
        "DEPTH 4\n"
        "MAXVAL 255\n"
        "TUPLTYPE RGB_ALPHA\n"
        "ENDHDR\n",
        &width, &height);
    if (width && height)
    {
        int pixel_count = width * height;
        u32 * pixels = pool_alloc(pool_index, width * height * sizeof(u32));
        int pixels_read = fread(pixels, sizeof(u32), pixel_count, file);
        fclose(file);
        if (pixels_read == pixel_count)
        {
            abgr_to_rgba(pixels, pixel_count);
            return (Image){ pixels, width, height };
        }
    }
    return (Image){};
}

// Writes an Image to a .pam file.
// Returns true if the entire Image was successfully written.
bool write_image_file(Image image, char * file_name)
{
    FILE * file = fopen(file_name, "w");
    if (!file) return false;
    fprintf(file,
        "P7\nWIDTH %d\nHEIGHT %d\nDEPTH 4\n"
        "MAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n",
        image.width, image.height);
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
Sound read_sound_file(int pool_index, char * file_name)
{
    FILE * file = fopen(file_name, "r");
    if (!file) return (Sound){};
    int sample_count = 0;
    fscanf(file, "SND\nSAMPLE_COUNT %d\nENDHDR\n", &sample_count);
    if (sample_count)
    {
        f32 * samples = pool_alloc(pool_index, sample_count * sizeof(f32));
        int samples_read = fread(pixels, sizeof(f32), sample_count, file);
        fclose(file);
        if (samples_read == sample_count)
        {
            return (Sound){ samples, sample_count };
        }
    }
    return (Sound){};
}

// Writes a sound into a file.
// Returns true if the entire Sound was successfully written.
bool write_sound_file(Sound sound, char * file_name)
{
    FILE * file = fopen(file_name, "w");
    if (!file) return false;
    fprintf(file, "SND\nSAMPLE_COUNT %d\nENDHDR\n", sound.sample_count);
    int byte_count = sound.sample_count * sizeof(sound.samples[0]);
    int bytes_written = fwrite(sound.samples, 1, byte_count, file);
    fclose(file);
    return byte_count == bytes_written;
}

//
// Loading assets.
//
// All assets are loaded into a single struct, with references to them taken
// when needed. load_assets should be called at launch with a suitable path.
//

struct
{
    Animated_Image button_animation;
    Animated_Image heart_animation;
    Font main_font;
    Font scream_font;
    Image heart_icon;
    Image morse_background;
    Image relaxed_skeleton;
    Sound wood_block_sound;
    Sound yay_sound;
}
assets;

// Load all assets, given a path relative to the location of the executable
// (or app bundle).
bool load_assets(char * assets_dir)
{
    char * full_dir = pool_alloc(FRAME_POOL, 512);
    char * base_path = SDL_GetBasePath();
    snprintf(full_dir, 512, "%s%s", base_path, assets_dir ? assets_dir : "");
    SDL_free(base_path);
    chdir(full_dir);

    // Load images.
    assets.relaxed_skeleton = read_image_file(PERSIST_POOL, "relaxed_skeleton.pam");
    if (!assets.relaxed_skeleton.pixels) return false;

    assets.heart_icon = read_image_file(PERSIST_POOL, "heart_icon.pam");
    if (!assets.heart_icon.pixels) return false;

    assets.morse_background = read_image_file(PERSIST_POOL, "morse.pam");
    if (!assets.morse_background.pixels) return false;

    {
        Image main_font_image = read_image_file(PERSIST_POOL, "font.pam");
        if (!main_font_image.pixels) return false;
        assets.main_font = (Font)
        {
            .pixels = main_font_image.pixels,
            .char_width = 6,
            .char_height = 12,
        };
    }

    {
        Image scream_font_image = read_image_file(PERSIST_POOL, "scream.pam");
        if (!scream_font_image.pixels) return false;
        assets.scream_font = (Font)
        {
            .pixels = scream_font_image.pixels,
            .char_width = 9,
            .char_height = 8,
        };
    }

    {
        Image button_image = read_image_file(PERSIST_POOL, "button.pam");
        assets.button_animation = (Animated_Image)
        {
            .pixels = button_image.pixels,
            .width = button_image.width,
            .height = button_image.height / 2,
            .frame_count = 2,
            .frame_duration_ms = 10,
        };
    }

    {
        Image heart_animation_image = read_image_file(PERSIST_POOL, "heart.pam");
        if (!heart_animation_image.pixels) return false;
        assets.heart_animation = (Animated_Image)
        {
            .pixels = heart_animation_image.pixels,
            .width = 320,
            .height = 200,
            .frame_count = 7,
        };
    }

    // TODO: Use custom sound loader.
    {
        SDL_AudioSpec spec = {};
        u8 * samples = 0;
        u32 byte_count = 0;
        SDL_LoadWAV("woodblock.wav", &spec, &samples, &byte_count);
        if (!samples) return false;
        assets.wood_block_sound.samples = (f32 * )samples;
        assets.wood_block_sound.sample_count = byte_count / sizeof(f32);
    }

    {
        SDL_AudioSpec spec = {};
        u8 * samples = 0;
        u32 byte_count = 0;
        SDL_LoadWAV("yay.wav", &spec, &samples, &byte_count);
        if (!samples) return false;
        assets.yay_sound.samples = (f32 * )samples;
        assets.yay_sound.sample_count = byte_count / sizeof(f32);
    }

    return true;
}
